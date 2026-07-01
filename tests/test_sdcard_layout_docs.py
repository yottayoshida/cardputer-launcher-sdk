import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class SdCardLayoutDocsTests(unittest.TestCase):
    def test_readme_documents_v1_sd_card_tree_and_validation(self):
        readme = (REPO_ROOT / "README.md").read_text()

        for text in [
            "/settings.json",
            "/apps/webhook_launcher/manifest.json",
            "/apps/webhook_launcher/commands.json",
            "/logs/launcher.jsonl",
            "/cache/",
            "/backups/",
            "python3 scripts/validate_configs.py",
        ]:
            self.assertIn(text, readme)

    def test_docs_describe_legacy_compatibility_and_firmware_recovery(self):
        docs = "\n".join(
            [
                (REPO_ROOT / "docs/ARCHITECTURE.md").read_text(),
                (REPO_ROOT / "docs/DESIGN.md").read_text(),
            ]
        )

        for text in [
            "/apps/webhook_launcher.json",
            "legacy compatibility",
            "creates missing non-secret directories",
            "does not create settings.json",
            "app-pack",
        ]:
            self.assertIn(text, docs)

    def test_docs_describe_backup_migration_and_partial_write_repair(self):
        docs = "\n".join(
            [
                (REPO_ROOT / "README.md").read_text(),
                (REPO_ROOT / "docs/DESIGN.md").read_text(),
            ]
        )

        for text in [
            "Back up the SD card",
            "schema version",
            "remove stale *.tmp",
            "partial write",
            "re-run validation",
        ]:
            self.assertIn(text, docs)

    def test_changelog_mentions_sd_layout_recovery(self):
        changelog = (REPO_ROOT / "CHANGELOG.md").read_text()

        self.assertIn("SD-card app-pack layout", changelog)
        self.assertIn("whole-tree validator", changelog)

    def test_webhook_command_example_uses_app_pack_path(self):
        example = (REPO_ROOT / "examples/webhook_command/README.md").read_text()

        self.assertIn("sdcard/apps/webhook_launcher/commands.json", example)
        self.assertNotIn("sdcard/apps/webhook_launcher.json", example)


if __name__ == "__main__":
    unittest.main()
