import json
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class SecretProvisioningDocsTests(unittest.TestCase):
    def test_secret_provisioning_doc_covers_design_and_threat_model(self):
        doc = (REPO_ROOT / "docs/SECRET_PROVISIONING.md").read_text()

        for text in [
            "Secret Provisioning",
            "secretRef",
            "sdcard/secrets.json",
            "ESP32-S3 storage options",
            "Lost SD card",
            "Malicious command pack",
            "Network observer",
            "does not protect a stolen SD card",
        ]:
            self.assertIn(text, doc)

    def test_sync_is_documented_as_disabled_by_default(self):
        doc = (REPO_ROOT / "docs/SECRET_PROVISIONING.md").read_text()

        for text in [
            "sync.enabled",
            "false",
            "disabled entirely",
            "HTTPS endpoint",
            "no background sync client",
        ]:
            self.assertIn(text, doc)

    def test_security_doc_links_secret_design_and_threats(self):
        security = (REPO_ROOT / "docs/SECURITY.md").read_text()

        for text in [
            "docs/SECRET_PROVISIONING.md",
            "secretRef",
            "lost SD card",
            "malicious command pack",
            "network observer",
        ]:
            self.assertIn(text, security)

    def test_secret_files_and_examples_are_safe(self):
        gitignore = (REPO_ROOT / ".gitignore").read_text()
        example = json.loads((REPO_ROOT / "sdcard/secrets.example.json").read_text())
        command_example = json.loads(
            (REPO_ROOT / "sdcard/apps/webhook_launcher_secret_ref.example.json").read_text()
        )

        self.assertIn("sdcard/secrets.json", gitignore)
        self.assertIn("webhooks.deploy.authorization", example)
        self.assertEqual(
            command_example["commands"][0]["headers"]["Authorization"]["secretRef"],
            "webhooks.deploy.authorization",
        )
        self.assertNotIn("live-token", json.dumps(example))
        self.assertNotIn("live-token", json.dumps(command_example))


if __name__ == "__main__":
    unittest.main()
