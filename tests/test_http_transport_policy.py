import unittest

from scripts.validate_configs import ValidationError, validate_webhook_config


class HttpTransportPolicyTests(unittest.TestCase):
    def test_accepts_local_http_only_when_flag_is_enabled(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Local Dev",
                    "method": "GET",
                    "url": "http://127.0.0.1:8080/webhook",
                    "allowLocalHttp": True,
                }
            ],
        }

        result = validate_webhook_config(data)

        self.assertTrue(result["commands"][0]["allowLocalHttp"])

    def test_rejects_non_local_http_even_when_flag_is_enabled(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Unsafe",
                    "method": "GET",
                    "url": "http://example.com/webhook",
                    "allowLocalHttp": True,
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_rejects_local_http_without_flag(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Local Dev",
                    "method": "GET",
                    "url": "http://localhost:8080/webhook",
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_rejects_non_bool_local_http_flag(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Local Dev",
                    "method": "GET",
                    "url": "http://127.0.0.1:8080/webhook",
                    "allowLocalHttp": "yes",
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)

    def test_rejects_oversized_post_body(self):
        data = {
            "version": 1,
            "commands": [
                {
                    "name": "Too Large",
                    "method": "POST",
                    "url": "https://example.com/webhook",
                    "body": {"payload": "x" * 2048},
                }
            ],
        }

        with self.assertRaises(ValidationError):
            validate_webhook_config(data)


if __name__ == "__main__":
    unittest.main()
