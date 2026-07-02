import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class HttpTransportDocsTests(unittest.TestCase):
    def test_security_doc_describes_transport_policy(self):
        security = (REPO_ROOT / "docs/SECURITY.md").read_text()

        for text in [
            "HTTPS is the default",
            "allowLocalHttp",
            "localhost",
            "127.0.0.1",
            "[::1]",
            "connect timeout",
            "read timeout",
            "GET transport failures are retried once",
            "POST requests are not retried",
            "2048 bytes",
            "160 bytes",
            "certificate pinning",
        ]:
            self.assertIn(text, security)

    def test_readme_mentions_local_http_exception_and_limits(self):
        readme = (REPO_ROOT / "README.md").read_text()

        for text in [
            "allowLocalHttp",
            "local development",
            "HTTPS",
            "request bodies are limited to 2048 bytes",
            "response previews are limited to 160 bytes",
        ]:
            self.assertIn(text, readme)

    def test_changelog_mentions_http_transport_hardening(self):
        changelog = (REPO_ROOT / "CHANGELOG.md").read_text()

        self.assertIn("HTTP transport hardening", changelog)
        self.assertIn("timeouts", changelog)
        self.assertIn("bounded GET retry", changelog)


if __name__ == "__main__":
    unittest.main()
