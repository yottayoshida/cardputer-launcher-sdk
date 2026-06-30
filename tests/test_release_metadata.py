import tempfile
import unittest
from pathlib import Path

from scripts.validate_release_metadata import (
    ReleaseMetadataError,
    validate_release_metadata,
)


REPO_ROOT = Path(__file__).resolve().parents[1]


class ReleaseMetadataTests(unittest.TestCase):
    def test_current_release_metadata_is_valid(self):
        validate_release_metadata(REPO_ROOT, "v0.1.0")

    def test_rejects_invalid_release_tag_name(self):
        with self.assertRaisesRegex(ReleaseMetadataError, "tag must match"):
            validate_release_metadata(REPO_ROOT, "0.1.0")

    def test_requires_matching_release_notes_file(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "docs").mkdir()
            (root / "CHANGELOG.md").write_text(
                "# Changelog\n\n## [0.2.0] - 2026-06-30\n\n### Added\n\n- CI.\n"
            )

            with self.assertRaisesRegex(
                ReleaseMetadataError, "docs/RELEASE_NOTES_v0.2.0.md"
            ):
                validate_release_metadata(root, "v0.2.0")

    def test_requires_matching_changelog_entry(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "docs").mkdir()
            (root / "CHANGELOG.md").write_text("# Changelog\n")
            (root / "docs/RELEASE_NOTES_v0.2.0.md").write_text(
                "# v0.2.0 \u2014 CI Checks\n"
            )

            with self.assertRaisesRegex(ReleaseMetadataError, "CHANGELOG"):
                validate_release_metadata(root, "v0.2.0")

    def test_requires_non_empty_release_notes_title(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "docs").mkdir()
            (root / "CHANGELOG.md").write_text(
                "# Changelog\n\n## [0.2.0] - 2026-06-30\n"
            )
            (root / "docs/RELEASE_NOTES_v0.2.0.md").write_text("")

            with self.assertRaisesRegex(ReleaseMetadataError, "title must begin"):
                validate_release_metadata(root, "v0.2.0")


if __name__ == "__main__":
    unittest.main()
