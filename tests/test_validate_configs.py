import json
import tempfile
import unittest
from pathlib import Path

from scripts.validate_configs import (
    ValidationError,
    redact_secret_like,
    validate_root,
    validate_settings,
    validate_webhook_config,
)


REPO_ROOT = Path(__file__).resolve().parents[1]


class ConfigValidationTests(unittest.TestCase):
    def _write_valid_sd_tree(self, root):
        (root / "apps/webhook_launcher").mkdir(parents=True)
        (root / "logs").mkdir()
        (root / "cache").mkdir()
        (root / "backups").mkdir()
        (root / "settings.json").write_text(
            json.dumps(
                {
                    "version": 1,
                    "wifi": {"ssid": "ssid", "password": "password"},
                }
            )
        )
        (root / "apps/webhook_launcher/manifest.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "id": "webhook_launcher",
                    "name": "Webhook Launcher",
                    "version": "1.0.0",
                    "config": "commands.json",
                }
            )
        )
        (root / "apps/webhook_launcher/commands.json").write_text(
            json.dumps(
                {
                    "version": 1,
                    "commands": [
                        {
                            "name": "Ping",
                            "method": "GET",
                            "url": "https://example.com/ping",
                        }
                    ],
                }
            )
        )

    def test_sample_settings_are_valid(self):
        data = json.loads((REPO_ROOT / "sdcard/settings.json").read_text())

        result = validate_settings(data)

        self.assertEqual(result["version"], 1)
        self.assertEqual(result["wifi"]["ssid"], "YOUR_WIFI_SSID")

    def test_settings_require_schema_version(self):
        data = {"wifi": {"ssid": "ssid", "password": "password"}}

        with self.assertRaisesRegex(ValidationError, "settings.version must be 1"):
            validate_settings(data)

    def test_rejects_unsupported_settings_version(self):
        data = {"version": 2, "wifi": {"ssid": "ssid", "password": "password"}}

        with self.assertRaisesRegex(ValidationError, "settings.version must be 1"):
            validate_settings(data)

    def test_rejects_boolean_settings_version(self):
        data = {"version": True, "wifi": {"ssid": "ssid", "password": "password"}}

        with self.assertRaisesRegex(ValidationError, "settings.version must be 1"):
            validate_settings(data)

    def test_sample_sd_tree_validates_with_layout_metadata(self):
        result = validate_root(REPO_ROOT / "sdcard")

        self.assertEqual(result["layout"]["version"], 1)
        self.assertEqual(result["apps"]["webhook_launcher"]["manifest"]["id"], "webhook_launcher")
        self.assertEqual(len(result["apps"]["webhook_launcher"]["commands"]["commands"]), 2)

    def test_sample_webhook_config_is_valid(self):
        data = json.loads(
            (REPO_ROOT / "sdcard/apps/webhook_launcher/commands.json").read_text()
        )

        result = validate_webhook_config(data)

        self.assertEqual(len(result["commands"]), 2)
        self.assertEqual(result["commands"][0]["method"], "POST")

    def test_rejects_unsupported_method(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Bad",
                    "method": "DELETE",
                    "url": "https://example.com/webhook",
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_rejects_boolean_webhook_version(self):
        data = {
            "version": True,
            "commands": [
                {
                    "name": "Bad Version",
                    "method": "POST",
                    "url": "https://example.com/webhook",
                }
            ],
        }

        with self.assertRaisesRegex(ValidationError, "webhook config version must be 1"):
            validate_webhook_config(data)

    def test_rejects_missing_webhook_url_with_command_index(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Missing URL",
                    "method": "POST",
                }
            ],
        }

        with self.assertRaisesRegex(ValidationError, r"commands\[0\]\.url"):
            validate_webhook_config(data)

    def test_rejects_non_array_webhook_commands(self):
        data = {"version": 1, "commands": {"name": "Bad"}}

        with self.assertRaisesRegex(
            ValidationError, "webhook config commands must be a non-empty array"
        ):
            validate_webhook_config(data)

    def test_rejects_non_object_webhook_command_with_index(self):
        data = {"version": 1, "commands": ["not an object"]}

        with self.assertRaisesRegex(ValidationError, r"commands\[0\] must be an object"):
            validate_webhook_config(data)

    def test_rejects_plain_http_by_default(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Unsafe",
                    "method": "GET",
                    "url": "http://example.com/webhook",
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_rejects_invalid_webhook_headers_with_field_path(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Bad Header",
                    "method": "POST",
                    "url": "https://example.com/webhook",
                    "headers": {"Authorization": 123},
                }
            ],
        }

        with self.assertRaisesRegex(
            ValidationError, r"commands\[0\]\.headers\.Authorization"
        ):
            validate_webhook_config(data)

    def test_rejects_get_webhook_body_with_field_path(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "GET Body",
                    "method": "GET",
                    "url": "https://example.com/webhook",
                    "body": {"unsafe": True},
                }
            ],
        }

        with self.assertRaisesRegex(ValidationError, r"commands\[0\]\.body"):
            validate_webhook_config(data)

    def test_redacts_secret_like_values(self):
        redacted = redact_secret_like(
            "Authorization: Bearer abc123 token=super-secret password=hunter2"
        )

        self.assertNotIn("abc123", redacted)
        self.assertNotIn("super-secret", redacted)
        self.assertNotIn("hunter2", redacted)

    def test_cli_validates_custom_root(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "apps").mkdir()
            (root / "settings.json").write_text(
                json.dumps(
                    {"version": 1, "wifi": {"ssid": "ssid", "password": "password"}}
                )
            )
            (root / "apps/webhook_launcher.json").write_text(
                json.dumps(
                    {
                        "version": 1,
                        "commands": [
                            {
                                "name": "Ping",
                                "method": "GET",
                                "url": "https://example.com/ping",
                            }
                        ],
                    }
                )
            )

            settings = json.loads((root / "settings.json").read_text())
            hooks = json.loads((root / "apps/webhook_launcher.json").read_text())

            self.assertEqual(validate_settings(settings)["wifi"]["ssid"], "ssid")
            self.assertEqual(validate_webhook_config(hooks)["commands"][0]["name"], "Ping")

    def test_malformed_manifest_reports_file_and_field(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._write_valid_sd_tree(root)
            (root / "apps/webhook_launcher/manifest.json").write_text(
                json.dumps(
                    {
                        "schema_version": 1,
                        "name": "Webhook Launcher",
                        "version": "1.0.0",
                        "config": "commands.json",
                    }
                )
            )

            with self.assertRaises(ValidationError) as raised:
                validate_root(root)

        message = str(raised.exception)
        self.assertIn("apps/webhook_launcher/manifest.json", message)
        self.assertIn("app manifest.id", message)

    def test_malformed_command_pack_reports_file_and_field(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._write_valid_sd_tree(root)
            (root / "apps/webhook_launcher/commands.json").write_text(
                json.dumps(
                    {
                        "version": 1,
                        "commands": [
                            {
                                "name": "Unsafe",
                                "method": "GET",
                                "url": "http://example.com/ping",
                            }
                        ],
                    }
                )
            )

            with self.assertRaises(ValidationError) as raised:
                validate_root(root)

        message = str(raised.exception)
        self.assertIn("apps/webhook_launcher/commands.json", message)
        self.assertIn("commands[0].url", message)

    def test_partial_write_residue_fails_validation(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            self._write_valid_sd_tree(root)
            (root / "apps/webhook_launcher/commands.json.tmp").write_text("{}")

            with self.assertRaises(ValidationError) as raised:
                validate_root(root)

        message = str(raised.exception)
        self.assertIn("partial write residue", message)
        self.assertIn("apps/webhook_launcher/commands.json.tmp", message)


if __name__ == "__main__":
    unittest.main()
