#!/usr/bin/env python3
"""Validate Cardputer Launcher SDK sample SD-card configuration."""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from urllib.parse import urlparse


class ValidationError(ValueError):
    """Raised when SD-card config is syntactically valid JSON but unsafe."""


SECRET_PATTERNS = [
    re.compile(r"(?i)(authorization:\s*bearer\s+)[^\s,}]+"),
    re.compile(r"(?i)\b(token|secret|password|api[_-]?key|key)=([^&\s,}]+)"),
]
APP_ID_PATTERN = re.compile(r"^[a-z0-9_]+$")
SUPPORTED_LAYOUT_VERSION = 1
WEBHOOK_APP_ID = "webhook_launcher"
WEBHOOK_PACK_DIR = Path("apps") / WEBHOOK_APP_ID
WEBHOOK_COMMANDS_PATH = WEBHOOK_PACK_DIR / "commands.json"
WEBHOOK_MANIFEST_PATH = WEBHOOK_PACK_DIR / "manifest.json"
LEGACY_WEBHOOK_PATH = Path("apps") / "webhook_launcher.json"
REQUIRED_DIRECTORIES = [
    Path("apps"),
    WEBHOOK_PACK_DIR,
    Path("logs"),
    Path("cache"),
    Path("backups"),
]
SECRET_REF_PATTERN = re.compile(r"^[A-Za-z0-9_.:-]{1,96}$")
SECRET_REF_KEY = "secretRef"


def _require_object(value, path):
    if not isinstance(value, dict):
        raise ValidationError(f"{path} must be an object")
    return value


def _require_string(value, path):
    if not isinstance(value, str) or not value.strip():
        raise ValidationError(f"{path} must be a non-empty string")
    return value


def _require_version(value, path):
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or value != SUPPORTED_LAYOUT_VERSION
    ):
        raise ValidationError(f"{path} must be {SUPPORTED_LAYOUT_VERSION}")
    return value


def validate_settings(data):
    root = _require_object(data, "settings")
    _require_version(root.get("version"), "settings.version")
    wifi = _require_object(root.get("wifi"), "settings.wifi")
    ssid = _require_string(wifi.get("ssid"), "settings.wifi.ssid")
    password = _require_string(wifi.get("password"), "settings.wifi.password")
    sync = _validate_sync_settings(root.get("sync", {}), "settings.sync")

    return {
        "version": SUPPORTED_LAYOUT_VERSION,
        "wifi": {"ssid": ssid, "password": password},
        "sync": sync,
    }


def _validate_url(value, path):
    url = _require_string(value, path)
    parsed = urlparse(url)
    if parsed.scheme != "https":
        raise ValidationError(f"{path} must use https")
    if not parsed.netloc:
        raise ValidationError(f"{path} must include a host")
    return url


def _validate_sync_settings(value, path):
    if value is None:
        value = {}
    sync = _require_object(value, path)
    enabled = sync.get("enabled", False)
    if not isinstance(enabled, bool):
        raise ValidationError(f"{path}.enabled must be true or false")

    endpoint = sync.get("endpoint", "")
    if endpoint:
        endpoint = _validate_url(endpoint, f"{path}.endpoint")
    elif enabled:
        raise ValidationError(f"{path}.endpoint is required when sync is enabled")

    return {"enabled": enabled, "endpoint": endpoint}


def validate_secret_ref_value(value, path):
    if isinstance(value, str):
        return _require_string(value, path)

    if isinstance(value, dict) and set(value.keys()) == {SECRET_REF_KEY}:
        ref = _require_string(value.get(SECRET_REF_KEY), f"{path}.{SECRET_REF_KEY}")
        if not SECRET_REF_PATTERN.fullmatch(ref):
            raise ValidationError(
                f"{path}.{SECRET_REF_KEY} must use letters, numbers, dots, "
                "underscores, colons, or hyphens"
            )
        return {SECRET_REF_KEY: ref}

    raise ValidationError(f"{path} must be a string or secretRef object")


def _validate_headers(value, path):
    if value is None:
        return {}
    headers = _require_object(value, path)
    normalized = {}
    for key, header_value in headers.items():
        header_name = _require_string(key, f"{path} key")
        normalized[header_name] = validate_secret_ref_value(
            header_value, f"{path}.{header_name}"
        )
    return normalized


def _validate_json_body(value, path):
    if value is None:
        return None
    if isinstance(value, (dict, list, str, int, float, bool)) or value is None:
        return value
    raise ValidationError(f"{path} must be a JSON value")


def validate_webhook_config(data):
    root = _require_object(data, "webhook config")
    _require_version(root.get("version"), "webhook config version")

    commands = root.get("commands")
    if not isinstance(commands, list) or not commands:
        raise ValidationError("webhook config commands must be a non-empty array")

    normalized_commands = []
    for index, command in enumerate(commands):
        path = f"commands[{index}]"
        command = _require_object(command, path)
        name = _require_string(command.get("name"), f"{path}.name")
        method = _require_string(command.get("method"), f"{path}.method").upper()
        if method not in {"GET", "POST"}:
            raise ValidationError(f"{path}.method must be GET or POST")
        url = _validate_url(command.get("url"), f"{path}.url")
        confirm = command.get("confirm", False)
        if not isinstance(confirm, bool):
            raise ValidationError(f"{path}.confirm must be true or false")
        headers = _validate_headers(command.get("headers"), f"{path}.headers")
        body = _validate_json_body(command.get("body"), f"{path}.body")
        if method == "GET" and body is not None:
            raise ValidationError(f"{path}.body is only supported for POST")

        normalized_commands.append(
            {
                "name": name,
                "method": method,
                "url": url,
                "confirm": confirm,
                "headers": headers,
                "body": body,
            }
        )

    return {"version": 1, "commands": normalized_commands}


def validate_app_manifest(data):
    root = _require_object(data, "app manifest")
    schema_version = root.get("schema_version")
    if schema_version != SUPPORTED_LAYOUT_VERSION:
        raise ValidationError(
            f"app manifest.schema_version must be {SUPPORTED_LAYOUT_VERSION}"
        )

    app_id = _require_string(root.get("id"), "app manifest.id")
    if not APP_ID_PATTERN.fullmatch(app_id):
        raise ValidationError("app manifest.id must use lowercase letters, digits, or _")

    name = _require_string(root.get("name"), "app manifest.name")
    version = _require_string(root.get("version"), "app manifest.version")
    config = _require_string(root.get("config"), "app manifest.config")
    if config.startswith("/") or "/" in config or "\\" in config:
        raise ValidationError("app manifest.config must be a file name in the app directory")

    return {
        "schema_version": SUPPORTED_LAYOUT_VERSION,
        "id": app_id,
        "name": name,
        "version": version,
        "config": config,
    }


def redact_secret_like(value):
    redacted = str(value)
    redacted = SECRET_PATTERNS[0].sub(r"\1[REDACTED]", redacted)
    redacted = SECRET_PATTERNS[1].sub(r"\1=[REDACTED]", redacted)
    return redacted


def _format_path(path):
    return path.as_posix() if isinstance(path, Path) else str(path)


def resolve_secret_refs(value, secrets):
    if isinstance(value, dict) and set(value.keys()) == {SECRET_REF_KEY}:
        ref = validate_secret_ref_value(value, "secret")[SECRET_REF_KEY]
        secret = secrets.get(ref)
        if not isinstance(secret, str) or not secret:
            raise ValidationError(f"secret reference {ref} is not available")
        return secret
    if isinstance(value, dict):
        return {key: resolve_secret_refs(item, secrets) for key, item in value.items()}
    if isinstance(value, list):
        return [resolve_secret_refs(item, secrets) for item in value]
    return value


def redact_known_secret_values(value, secrets):
    redacted = str(value)
    secret_values = {
        secret for secret in secrets.values() if isinstance(secret, str) and secret
    }
    for secret in sorted(secret_values, key=len, reverse=True):
        redacted = redacted.replace(secret, "[REDACTED]")
    return redact_secret_like(redacted)


def _load_json(path):
    try:
        with path.open() as handle:
            return json.load(handle)
    except FileNotFoundError as exc:
        raise ValidationError(f"missing file: {_format_path(path)}") from exc
    except json.JSONDecodeError as exc:
        raise ValidationError(f"invalid JSON in {_format_path(path)}: {exc}") from exc


def _validate_json_file(root, relative_path, validator):
    path = root / relative_path
    try:
        return validator(_load_json(path))
    except ValidationError as exc:
        raise ValidationError(f"{relative_path.as_posix()}: {exc}") from exc


def _detect_partial_writes(root):
    if not root.exists():
        return
    for path in root.rglob("*"):
        if path.is_file() and (
            path.name.endswith(".tmp")
            or path.name.endswith(".partial")
            or path.name.endswith(".new")
        ):
            try:
                relative = path.relative_to(root)
            except ValueError:
                relative = path
            raise ValidationError(f"partial write residue: {relative.as_posix()}")


def _require_directories(root):
    for relative_path in REQUIRED_DIRECTORIES:
        if not (root / relative_path).is_dir():
            raise ValidationError(f"missing directory: {relative_path.as_posix()}")


def _validate_webhook_app_pack(root):
    manifest = _validate_json_file(root, WEBHOOK_MANIFEST_PATH, validate_app_manifest)
    if manifest["id"] != WEBHOOK_APP_ID:
        raise ValidationError(
            f"{WEBHOOK_MANIFEST_PATH.as_posix()}: app manifest.id must be {WEBHOOK_APP_ID}"
        )
    if manifest["config"] != WEBHOOK_COMMANDS_PATH.name:
        raise ValidationError(
            f"{WEBHOOK_MANIFEST_PATH.as_posix()}: "
            f"app manifest.config must be {WEBHOOK_COMMANDS_PATH.name}"
        )

    commands = _validate_json_file(root, WEBHOOK_COMMANDS_PATH, validate_webhook_config)
    return {"manifest": manifest, "commands": commands}


def _validate_legacy_webhook_app(root):
    commands = _validate_json_file(root, LEGACY_WEBHOOK_PATH, validate_webhook_config)
    manifest = {
        "schema_version": SUPPORTED_LAYOUT_VERSION,
        "id": WEBHOOK_APP_ID,
        "name": "Webhook Launcher",
        "version": "legacy",
        "config": LEGACY_WEBHOOK_PATH.name,
        "legacy": True,
    }
    return {"manifest": manifest, "commands": commands}


def validate_root(root):
    root = Path(root)
    _detect_partial_writes(root)
    _require_directories(root)

    settings = _validate_json_file(root, Path("settings.json"), validate_settings)
    if (root / WEBHOOK_MANIFEST_PATH).exists() or (root / WEBHOOK_COMMANDS_PATH).exists():
        webhook_app = _validate_webhook_app_pack(root)
    elif (root / LEGACY_WEBHOOK_PATH).exists():
        webhook_app = _validate_legacy_webhook_app(root)
    else:
        raise ValidationError(
            "missing app pack: apps/webhook_launcher/manifest.json and commands.json"
        )

    return {
        "layout": {"version": SUPPORTED_LAYOUT_VERSION},
        "settings": settings,
        "apps": {WEBHOOK_APP_ID: webhook_app},
        "webhooks": webhook_app["commands"],
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        default=Path(__file__).resolve().parents[1] / "sdcard",
        type=Path,
        help="SD-card root directory to validate",
    )
    args = parser.parse_args(argv)

    try:
        result = validate_root(args.root)
    except ValidationError as exc:
        print(f"config validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "validated settings and "
        f"{len(result['webhooks']['commands'])} webhook command(s)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
