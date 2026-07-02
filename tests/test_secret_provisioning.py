import unittest

from scripts.validate_configs import (
    ValidationError,
    redact_known_secret_values,
    resolve_secret_refs,
    validate_settings,
    validate_webhook_config,
)


class SecretProvisioningTests(unittest.TestCase):
    def test_accepts_webhook_header_secret_reference(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Deploy",
                    "method": "POST",
                    "url": "https://example.com/deploy",
                    "headers": {
                        "Authorization": {
                            "secretRef": "webhooks.deploy.authorization"
                        }
                    },
                }
            ],
        }

        result = validate_webhook_config(data)

        self.assertEqual(
            result["commands"][0]["headers"]["Authorization"],
            {"secretRef": "webhooks.deploy.authorization"},
        )

    def test_rejects_invalid_secret_reference_name(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Deploy",
                    "method": "POST",
                    "url": "https://example.com/deploy",
                    "headers": {
                        "Authorization": {
                            "secretRef": "../private/token"
                        }
                    },
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_resolves_and_redacts_known_secret_values(self):
        value = {"Authorization": {"secretRef": "webhooks.deploy.authorization"}}
        secrets = {"webhooks.deploy.authorization": "Bearer live-token"}

        resolved = resolve_secret_refs(value, secrets)
        redacted = redact_known_secret_values(str(resolved), secrets)

        self.assertEqual(resolved["Authorization"], "Bearer live-token")
        self.assertNotIn("live-token", redacted)
        self.assertIn("[REDACTED]", redacted)

    def test_missing_secret_reference_fails_resolution(self):
        value = {"Authorization": {"secretRef": "webhooks.deploy.authorization"}}

        with self.assertRaises(ValidationError):
            resolve_secret_refs(value, {})

    def test_sync_settings_default_to_disabled(self):
        result = validate_settings(
            {"version": 1, "wifi": {"ssid": "ssid", "password":"password"}}
        )

        self.assertEqual(result["sync"], {"enabled": False, "endpoint": ""})

    def test_enabled_sync_requires_https_endpoint(self):
        with self.assertRaises(ValidationError):
            validate_settings(
                {
                    "wifi": {"ssid": "ssid", "password": "password"},
                    "sync": {"enabled": True, "endpoint": "http://example.com/sync"},
                }
            )


if __name__ == "__main__":
    unittest.main()
