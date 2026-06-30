import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
WORKFLOW = REPO_ROOT / ".github/workflows/ci.yml"


class CiWorkflowSourceTests(unittest.TestCase):
    def test_ci_workflow_exists(self):
        self.assertTrue(WORKFLOW.exists())

    def test_workflow_runs_on_pull_requests_main_pushes_and_release_tags(self):
        text = WORKFLOW.read_text()

        self.assertIn("pull_request:", text)
        self.assertIn("branches: [main]", text)
        self.assertIn("tags:", text)
        self.assertIn("'v*'", text)

    def test_workflow_runs_host_validation_commands(self):
        text = WORKFLOW.read_text()

        self.assertIn("python3 -m unittest discover -s tests", text)
        self.assertIn("python3 scripts/validate_configs.py", text)

    def test_workflow_runs_firmware_build_with_platformio_cache(self):
        text = WORKFLOW.read_text()

        self.assertIn("pio run", text)
        self.assertIn("~/.platformio/.cache", text)
        self.assertIn(".pio/build", text)
        self.assertIn("actions/upload-artifact", text)

    def test_workflow_does_not_cache_firmware_build_outputs(self):
        text = WORKFLOW.read_text()
        cache_block = text.split("uses: actions/cache@v4", 1)[1].split(
            "- name: Install PlatformIO", 1
        )[0]

        self.assertNotIn(".pio/build", cache_block)

    def test_workflow_runs_docs_secret_and_release_checks(self):
        text = WORKFLOW.read_text()

        self.assertIn("python3 scripts/check_docs_links.py", text)
        self.assertIn("python3 scripts/scan_secrets.py", text)
        self.assertIn("python3 scripts/validate_release_metadata.py", text)
        self.assertIn("refs/tags/v", text)


if __name__ == "__main__":
    unittest.main()
