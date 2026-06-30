import tempfile
import unittest
from pathlib import Path

from scripts.scan_secrets import scan_paths


class SecretScanTests(unittest.TestCase):
    def test_allows_documented_placeholders(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            sample = root / "sample.md"
            sample.write_text(
                "Authorization: Bearer REPLACE_WITH_TOKEN\n"
                "api_key=YOUR_API_KEY\n"
                "token=[REDACTED]\n"
            )

            self.assertEqual(scan_paths([sample]), [])

    def test_rejects_high_confidence_tokens(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            sample = root / "sample.txt"
            sample.write_text(
                "github=" + "ghp_" + "1234567890abcdefghijklmnopqrstuvwxyzABCD\n"
                "openai=" + "sk-" + "proj-" + "1234567890abcdefghijklmnopqrstuvwxyzABCDE\n"
                "aws=" + "AKIA" + "1234567890ABCDEF\n"
            )

            findings = scan_paths([sample])

            self.assertEqual(len(findings), 3)
            self.assertTrue(any("GitHub token" in finding for finding in findings))
            self.assertTrue(any("OpenAI API key" in finding for finding in findings))
            self.assertTrue(any("AWS access key" in finding for finding in findings))

    def test_rejects_real_token_on_line_with_placeholder_word(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            sample = root / "sample.txt"
            sample.write_text(
                "example_token = "
                + "ghp_"
                + "1234567890abcdefghijklmnopqrstuvwxyzABCD\n"
            )

            findings = scan_paths([sample])

            self.assertEqual(len(findings), 1)
            self.assertIn("GitHub token", findings[0])

    def test_rejects_private_key_blocks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            sample = root / "key.pem"
            sample.write_text(
                "-----BEGIN " + "PRIVATE KEY-----\nsecret\n-----END PRIVATE KEY-----\n"
            )

            findings = scan_paths([sample])

            self.assertEqual(len(findings), 1)
            self.assertIn("private key", findings[0])

    def test_rejects_encrypted_and_pgp_private_key_blocks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            encrypted = root / "encrypted.pem"
            pgp = root / "pgp.asc"
            encrypted.write_text(
                "-----BEGIN ENCRYPTED "
                + "PRIVATE KEY-----\nsecret\n-----END ENCRYPTED PRIVATE KEY-----\n"
            )
            pgp.write_text(
                "-----BEGIN PGP "
                + "PRIVATE KEY BLOCK-----\nsecret\n-----END PGP PRIVATE KEY BLOCK-----\n"
            )

            findings = scan_paths([encrypted, pgp])

            self.assertEqual(len(findings), 2)
            self.assertTrue(all("private key" in finding for finding in findings))


if __name__ == "__main__":
    unittest.main()
