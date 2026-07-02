import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class AppManifestDocsTests(unittest.TestCase):
    def combined_docs(self):
        return "\n".join(
            [
                (REPO_ROOT / "docs/ARCHITECTURE.md").read_text(),
                (REPO_ROOT / "docs/DESIGN.md").read_text(),
            ]
        )

    def test_manifest_fields_and_examples_are_documented(self):
        text = self.combined_docs()
        for token in (
            "id",
            "name",
            "version",
            "configPath",
            "permissions",
            "capabilities",
            "Webhook Launcher",
            "Log Viewer",
        ):
            self.assertIn(token, text)

    def test_permission_vocabulary_is_documented(self):
        text = self.combined_docs()
        for token in (
            "storage.read",
            "storage.write",
            "network.http",
            "input.keyboard",
            "display.draw",
            "ir.transmit",
            "sensor.read",
        ):
            self.assertIn(token, text)

    def test_lifecycle_and_dynamic_loading_limits_are_documented(self):
        text = self.combined_docs()
        for token in ("onFocus", "onBlur", "onTick", "onSuspend", "onResume"):
            self.assertIn(token, text)
        self.assertIn("does not load binary apps from SD card", text)


if __name__ == "__main__":
    unittest.main()
