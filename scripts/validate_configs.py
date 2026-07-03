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


def _is_valid_secret_ref_name(ref):
    return bool(SECRET_REF_PATTERN.fullmatch(ref))

MAX_REQUEST_BODY_BYTES = 2048
LOCAL_HTTP_HOSTS = {"localhost", "127.0.0.1", "::1"}
INPUT_KEY_PATTERN = re.compile(r"^[a-z][a-z0-9_]*$")
INPUT_KINDS = {"text", "choice", "boolean", "confirmation"}
RISK_LEVELS = {"low", "medium", "high"}


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
    # Accepts a literal string (unchanged from earlier versions) or a
    # {"secretRef": "<ref>"} object, mirroring header values.
    password = validate_secret_ref_value(wifi.get("password"), "settings.wifi.password")
    if isinstance(password, str) and not password.strip():
        raise ValidationError("settings.wifi.password must be a non-empty string")
    sync = _validate_sync_settings(root.get("sync", {}), "settings.sync")

    return {
        "version": SUPPORTED_LAYOUT_VERSION,
        "wifi": {"ssid": ssid, "password": password},
        "sync": sync,
    }


# "input" is always implemented. "secret" is accepted only where
# allow_secret is True (url and body; header text keeps the older
# {"secretRef": ...} object syntax instead, see validate_secret_ref_value),
# and only validates ref syntax here -- actual resolution happens firmware
# side (or via --secrets-file below) so a config can be validated without a
# real secrets file. Any other namespace is reserved so a config that loads
# today keeps the same meaning once a new namespace is implemented (see
# SECURITY.md).
def _validate_placeholder_token(token, input_keys, path, allow_secret=False):
    dot = token.find(".")
    if dot < 0:
        raise ValidationError(f"{path} placeholder must be namespace.key")
    namespace = token[:dot]
    key = token[dot + 1 :]
    if namespace == "input":
        if key not in input_keys:
            raise ValidationError(f"{path} references undefined input '{key}'")
        return key
    if namespace == "secret" and allow_secret:
        if not _is_valid_secret_ref_name(key):
            raise ValidationError(
                f"{path} secret ref must use letters, numbers, dots, underscores, "
                "colons, or hyphens"
            )
        return key
    raise ValidationError(f"{path} placeholder namespace '{namespace}' is reserved or unknown")


def _validate_embedded_placeholders(text, input_keys, path, allow_secret=False):
    search_from = 0
    while True:
        open_idx = text.find("{{", search_from)
        if open_idx < 0:
            return
        close_idx = text.find("}}", open_idx + 2)
        if close_idx < 0:
            raise ValidationError(f"{path} has an unterminated placeholder")
        _validate_placeholder_token(
            text[open_idx + 2 : close_idx], input_keys, path, allow_secret
        )
        search_from = close_idx + 2


# Placeholders may appear anywhere at or after `authority_end` (the URL's
# path/query/fragment), never in the scheme/userinfo/host/port, so a typed
# value or a resolved secret can never redirect a request to a different
# host.
def _validate_url_placeholders(url, authority_end, input_keys, path):
    first_open = url.find("{{")
    if 0 <= first_open < authority_end:
        raise ValidationError(f"{path} placeholder not allowed before the host")
    _validate_embedded_placeholders(url, input_keys, path, allow_secret=True)


# Body placeholders must occupy an entire JSON string value ("{{input.x}}" or
# "{{secret.x}}") so substitution is a single token replacement, not string
# concatenation inside arbitrary JSON text.
def _validate_body_placeholders(value, input_keys, path):
    if isinstance(value, str):
        if "{{" not in value and "}}" not in value:
            return
        inner = value[2:-2] if value.startswith("{{") and value.endswith("}}") else None
        if inner is None or "{{" in inner or "}}" in inner:
            raise ValidationError(f"{path} placeholder must be the entire JSON string value")
        _validate_placeholder_token(inner, input_keys, path, allow_secret=True)
    elif isinstance(value, dict):
        for key, item in value.items():
            if "{{" in key or "}}" in key:
                raise ValidationError(f"{path} placeholder must be a value, not an object key")
            _validate_body_placeholders(item, input_keys, f"{path}.{key}")
    elif isinstance(value, list):
        for index, item in enumerate(value):
            _validate_body_placeholders(item, input_keys, f"{path}[{index}]")


def _validate_url(value, path, allow_local_http=False, input_keys=None):
    url = _require_string(value, path)
    parsed = urlparse(url)
    if parsed.scheme == "http":
        if not allow_local_http:
            raise ValidationError(f"{path} must use https")
        if parsed.hostname not in LOCAL_HTTP_HOSTS:
            raise ValidationError(f"{path} local HTTP must use a loopback host")
    elif parsed.scheme != "https":
        raise ValidationError(f"{path} must use https")
    if not parsed.netloc:
        raise ValidationError(f"{path} must include a host")

    if input_keys is not None:
        authority_end = len(parsed.scheme) + 3 + len(parsed.netloc)
        _validate_url_placeholders(url, authority_end, input_keys, path)

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
        if not _is_valid_secret_ref_name(ref):
            raise ValidationError(
                f"{path}.{SECRET_REF_KEY} must use letters, numbers, dots, "
                "underscores, colons, or hyphens"
            )
        return {SECRET_REF_KEY: ref}

    raise ValidationError(f"{path} must be a string or secretRef object")


def _validate_headers(value, path, input_keys):
    if value is None:
        return {}
    headers = _require_object(value, path)
    normalized = {}
    for key, header_value in headers.items():
        header_name = _require_string(key, f"{path} key")
        normalized_value = validate_secret_ref_value(header_value, f"{path}.{header_name}")
        # Secret-backed values are opaque; only literal header text may
        # reference typed inputs.
        if isinstance(normalized_value, str):
            _validate_embedded_placeholders(
                normalized_value, input_keys, f"{path}.{header_name}"
            )
        normalized[header_name] = normalized_value
    return normalized


def _validate_json_body(value, path, input_keys):
    if value is None:
        return None
    if isinstance(value, (dict, list, str, int, float, bool)) or value is None:
        _validate_body_placeholders(value, input_keys, path)
        body_bytes = len(json.dumps(value, separators=(",", ":")).encode("utf-8"))
        if body_bytes > MAX_REQUEST_BODY_BYTES:
            raise ValidationError(
                f"{path} must be at most {MAX_REQUEST_BODY_BYTES} bytes"
            )
        return value
    raise ValidationError(f"{path} must be a JSON value")


def _validate_input_field(value, path):
    field = _require_object(value, path)
    key = _require_string(field.get("key"), f"{path}.key")
    if not INPUT_KEY_PATTERN.fullmatch(key):
        raise ValidationError(
            f"{path}.key must start with a-z and contain only a-z, 0-9, or _"
        )

    kind = _require_string(field.get("kind"), f"{path}.kind")
    if kind not in INPUT_KINDS:
        raise ValidationError(f"{path}.kind must be text, choice, boolean, or confirmation")

    label = _require_string(field.get("label"), f"{path}.label")

    required = field.get("required", True)
    if not isinstance(required, bool):
        raise ValidationError(f"{path}.required must be true or false")

    choices = field.get("choices")
    if kind == "choice":
        if not isinstance(choices, list) or not (2 <= len(choices) <= 8):
            raise ValidationError(f"{path}.choices must have 2 to 8 entries")
        choices = [
            _require_string(choice, f"{path}.choices[{i}]")
            for i, choice in enumerate(choices)
        ]
    elif choices is not None:
        raise ValidationError(f"{path}.choices is only valid for kind choice")
    else:
        choices = []

    max_length = field.get("maxLength")
    if kind == "text":
        if max_length is None:
            max_length = 96
        elif (
            not isinstance(max_length, int)
            or isinstance(max_length, bool)
            or not (1 <= max_length <= 128)
        ):
            raise ValidationError(f"{path}.maxLength must be between 1 and 128")
    elif max_length is not None:
        raise ValidationError(f"{path}.maxLength is only valid for kind text")
    else:
        max_length = 96

    # Validated after choices/maxLength are known so a bad default (too
    # long, not a declared choice, not a real boolean) fails loudly at load
    # time instead of being silently clamped or ignored when shown.
    default = field.get("default", "")
    if default and not isinstance(default, str):
        raise ValidationError(f"{path}.default must be a string")
    if default:
        if kind == "text" and len(default) > max_length:
            raise ValidationError(f"{path}.default exceeds maxLength")
        elif kind == "choice" and default not in choices:
            raise ValidationError(f"{path}.default must be one of choices")
        elif kind == "boolean" and default not in ("true", "false"):
            raise ValidationError(f"{path}.default must be true or false")
        elif kind == "confirmation":
            raise ValidationError(f"{path}.default is not valid for kind confirmation")

    return {
        "key": key,
        "kind": kind,
        "label": label,
        "required": required,
        "default": default or "",
        "choices": choices,
        "maxLength": max_length,
    }


def _validate_inputs(value, path):
    if value is None:
        return []
    if not isinstance(value, list):
        raise ValidationError(f"{path} must be an array")
    fields = [_validate_input_field(item, f"{path}[{i}]") for i, item in enumerate(value)]
    keys = [field["key"] for field in fields]
    duplicates = {key for key in keys if keys.count(key) > 1}
    if duplicates:
        raise ValidationError(f"{path} has duplicate key '{sorted(duplicates)[0]}'")
    return fields


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
        allow_local_http = command.get("allowLocalHttp", False)
        if not isinstance(allow_local_http, bool):
            raise ValidationError(f"{path}.allowLocalHttp must be true or false")

        confirm = command.get("confirm", False)
        if not isinstance(confirm, bool):
            raise ValidationError(f"{path}.confirm must be true or false")

        if "category" in command:
            category = _require_string(command.get("category"), f"{path}.category")
        else:
            category = ""

        if "description" in command:
            description = _require_string(command.get("description"), f"{path}.description")
        else:
            description = ""

        risk = command.get("risk", "low")
        if risk not in RISK_LEVELS:
            raise ValidationError(f"{path}.risk must be low, medium, or high")

        require_preview = command.get("requirePreview", False)
        if not isinstance(require_preview, bool):
            raise ValidationError(f"{path}.requirePreview must be true or false")

        if risk == "high" and not (confirm and require_preview):
            raise ValidationError(
                f"{path} risk:high requires confirm:true and requirePreview:true"
            )

        inputs = _validate_inputs(command.get("inputs"), f"{path}.inputs")
        input_keys = {field["key"] for field in inputs}

        url = _validate_url(
            command.get("url"),
            f"{path}.url",
            allow_local_http=allow_local_http,
            input_keys=input_keys,
        )
        headers = _validate_headers(command.get("headers"), f"{path}.headers", input_keys)
        body = _validate_json_body(command.get("body"), f"{path}.body", input_keys)
        if method == "GET" and body is not None:
            raise ValidationError(f"{path}.body is only supported for POST")

        normalized_commands.append(
            {
                "name": name,
                "method": method,
                "url": url,
                "allowLocalHttp": allow_local_http,
                "confirm": confirm,
                "category": category,
                "description": description,
                "risk": risk,
                "requirePreview": require_preview,
                "inputs": inputs,
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


# Mirrors SecretProvider::kMinSecretLength / hasControlCharacter in
# SdSecretProvider.cpp: a resolved secret too short to redact reliably, or
# containing a control character, is rejected everywhere a secret is
# resolved, not just in the firmware.
MIN_SECRET_LENGTH = 6


def _check_resolved_secret_policy(ref, secret):
    # Firmware measures length in bytes (Arduino String::length()), not
    # codepoints, so a multi-byte UTF-8 secret must be measured the same way
    # here or a short non-ASCII secret could pass host validation and then
    # be rejected by firmware (or vice versa).
    if len(secret.encode("utf-8")) < MIN_SECRET_LENGTH:
        raise ValidationError(
            f"secret ref '{ref}' is too short (must be at least {MIN_SECRET_LENGTH} bytes)"
        )
    if any(ord(char) < 0x20 for char in secret):
        raise ValidationError(f"secret ref '{ref}' contains a control character")


def _resolve_and_check_secret(ref, secrets):
    """Looks up `ref` in `secrets` and applies the same policy firmware
    enforces at resolve time (available, minimum byte length, no control
    characters). Shared by both secretRef syntaxes -- the older
    {"secretRef": ...} object form and the {{secret.<ref>}} template form."""
    secret = secrets.get(ref)
    if not isinstance(secret, str) or not secret:
        raise ValidationError(f"secret reference {ref} is not available")
    _check_resolved_secret_policy(ref, secret)
    return secret


def resolve_secret_refs(value, secrets):
    if isinstance(value, dict) and set(value.keys()) == {SECRET_REF_KEY}:
        ref = validate_secret_ref_value(value, "secret")[SECRET_REF_KEY]
        return _resolve_and_check_secret(ref, secrets)
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


_TEMPLATE_SECRET_REF_PATTERN = re.compile(r"\{\{secret\.([^}]+)\}\}")


def _extract_template_secret_refs(value):
    """Collects every {{secret.<ref>}} name found anywhere inside `value`
    (a JSON-shaped structure). Complements resolve_secret_refs, which only
    walks the older {"secretRef": ...} object form."""
    refs = []
    if isinstance(value, str):
        refs.extend(_TEMPLATE_SECRET_REF_PATTERN.findall(value))
    elif isinstance(value, dict):
        for item in value.values():
            refs.extend(_extract_template_secret_refs(item))
    elif isinstance(value, list):
        for item in value:
            refs.extend(_extract_template_secret_refs(item))
    return refs


def _check_secrets_resolvable(result, secrets):
    """Optional cross-check when --secrets-file is given: every secretRef a
    config declares, in either syntax, must actually resolve. Catches "you
    referenced a ref you forgot to add to secrets.json" before flashing."""
    resolve_secret_refs(result["settings"]["wifi"], secrets)
    for command in result["webhooks"]["commands"]:
        resolve_secret_refs(command["headers"], secrets)
        template_refs = _extract_template_secret_refs(
            command["url"]
        ) + _extract_template_secret_refs(command["body"])
        for ref in template_refs:
            _resolve_and_check_secret(ref, secrets)


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


def validate_root(root, secrets_file=None):
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

    result = {
        "layout": {"version": SUPPORTED_LAYOUT_VERSION},
        "settings": settings,
        "apps": {WEBHOOK_APP_ID: webhook_app},
        "webhooks": webhook_app["commands"],
    }

    if secrets_file is not None:
        secrets = _load_json(Path(secrets_file))
        if not isinstance(secrets, dict):
            raise ValidationError(f"{_format_path(secrets_file)}: must be a JSON object")
        _check_secrets_resolvable(result, secrets)

    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        default=Path(__file__).resolve().parents[1] / "sdcard",
        type=Path,
        help="SD-card root directory to validate",
    )
    parser.add_argument(
        "--secrets-file",
        type=Path,
        default=None,
        help="optional secrets.json to confirm every declared secretRef actually resolves",
    )
    args = parser.parse_args(argv)

    try:
        result = validate_root(args.root, secrets_file=args.secrets_file)
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
