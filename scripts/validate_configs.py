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


def _require_object(value, path):
    if not isinstance(value, dict):
        raise ValidationError(f"{path} must be an object")
    return value


def _require_string(value, path):
    if not isinstance(value, str) or not value.strip():
        raise ValidationError(f"{path} must be a non-empty string")
    return value


def validate_settings(data):
    root = _require_object(data, "settings")
    wifi = _require_object(root.get("wifi"), "settings.wifi")
    ssid = _require_string(wifi.get("ssid"), "settings.wifi.ssid")
    password = _require_string(wifi.get("password"), "settings.wifi.password")

    return {"wifi": {"ssid": ssid, "password": password}}


def _validate_url(value, path):
    url = _require_string(value, path)
    parsed = urlparse(url)
    if parsed.scheme != "https":
        raise ValidationError(f"{path} must use https")
    if not parsed.netloc:
        raise ValidationError(f"{path} must include a host")
    return url


def _validate_headers(value, path):
    if value is None:
        return {}
    headers = _require_object(value, path)
    normalized = {}
    for key, header_value in headers.items():
        normalized[_require_string(key, f"{path} key")] = _require_string(
            header_value, f"{path}.{key}"
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
    version = root.get("version")
    if version != 1:
        raise ValidationError("webhook config version must be 1")

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


def redact_secret_like(value):
    redacted = str(value)
    redacted = SECRET_PATTERNS[0].sub(r"\1[REDACTED]", redacted)
    redacted = SECRET_PATTERNS[1].sub(r"\1=[REDACTED]", redacted)
    return redacted


def _load_json(path):
    try:
        with path.open() as handle:
            return json.load(handle)
    except FileNotFoundError as exc:
        raise ValidationError(f"missing file: {path}") from exc
    except json.JSONDecodeError as exc:
        raise ValidationError(f"invalid JSON in {path}: {exc}") from exc


def validate_root(root):
    root = Path(root)
    settings = validate_settings(_load_json(root / "settings.json"))
    webhooks = validate_webhook_config(_load_json(root / "apps/webhook_launcher.json"))
    return {"settings": settings, "webhooks": webhooks}


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
