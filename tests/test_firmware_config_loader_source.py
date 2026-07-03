import re
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SOURCE = REPO_ROOT / "src/storage/ConfigLoader.cpp"


def _helper_body(name):
    text = SOURCE.read_text()
    match = re.search(rf"bool {name}\([^)]*\) \{{(.*?)\n\}}", text, re.S)
    if not match:
        raise AssertionError(f"{name} helper not found")
    return match.group(1)


class FirmwareConfigLoaderSourceTests(unittest.TestCase):
    def test_required_string_check_does_not_mutate_output_with_trim(self):
        helper = _helper_body("readRequiredString")

        self.assertNotIn("out.trim()", helper)
        self.assertIn("probe.trim()", helper)

    def test_https_url_validation_checks_query_and_fragment_boundaries(self):
        helper = _helper_body("validateHttpsUrl")

        self.assertIn("indexOf('?')", helper)
        self.assertIn("indexOf('#')", helper)

    def test_confirm_validation_distinguishes_missing_from_explicit_null(self):
        text = SOURCE.read_text()

        # confirm is parsed via the shared parseOptionalBool() helper, which
        # itself checks presence with hasMember(item, key) -- not
        # containsKey(), which does not distinguish missing from explicit
        # null.
        self.assertIn('parseOptionalBool(item, "confirm"', text)
        self.assertIn("hasMember(item, key)", _helper_body("parseOptionalBool"))
        self.assertNotIn(".containsKey(", text)


if __name__ == "__main__":
    unittest.main()
