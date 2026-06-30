import tempfile
import unittest
from pathlib import Path

from scripts.check_docs_links import check_markdown_links


class DocsLinkTests(unittest.TestCase):
    def test_valid_relative_markdown_links_pass(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "docs").mkdir()
            (root / "README.md").write_text(
                "# Project\n\nSee [guide](docs/guide.md) and [deep](docs/guide.md#deep-link).\n"
            )
            (root / "docs/guide.md").write_text("# Guide\n\n## Deep Link\n")

            self.assertEqual(check_markdown_links(root), [])

    def test_missing_relative_markdown_link_is_reported(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "README.md").write_text("# Project\n\nSee [missing](docs/missing.md).\n")

            findings = check_markdown_links(root)

            self.assertEqual(len(findings), 1)
            self.assertIn("README.md", findings[0])
            self.assertIn("docs/missing.md", findings[0])

    def test_missing_markdown_anchor_is_reported(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "docs").mkdir()
            (root / "README.md").write_text("# Project\n\nSee [deep](docs/guide.md#missing).\n")
            (root / "docs/guide.md").write_text("# Guide\n\n## Present\n")

            findings = check_markdown_links(root)

            self.assertEqual(len(findings), 1)
            self.assertIn("#missing", findings[0])


if __name__ == "__main__":
    unittest.main()
