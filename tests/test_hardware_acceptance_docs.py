import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class HardwareAcceptanceDocsTests(unittest.TestCase):
    def test_hardware_acceptance_doc_has_fresh_clone_setup_and_flash_steps(self):
        doc = (REPO_ROOT / "docs/HARDWARE_ACCEPTANCE.md").read_text()

        for text in [
            "Fresh Clone Setup",
            "git clone",
            "python3 scripts/validate_configs.py",
            "pio run",
            "pio run --target upload",
            "SD-Card Preparation",
            "Wi-Fi and Webhook Smoke Test",
        ]:
            self.assertIn(text, doc)

    def test_each_builtin_app_has_manual_acceptance_case(self):
        doc = (REPO_ROOT / "docs/HARDWARE_ACCEPTANCE.md").read_text()

        for app_name in [
            "Webhook Launcher",
            "Log Viewer",
            "Settings/About",
        ]:
            self.assertIn(app_name, doc)

    def test_hardware_matrix_covers_required_capabilities(self):
        doc = (REPO_ROOT / "docs/HARDWARE_ACCEPTANCE.md").read_text()

        for capability in [
            "Display",
            "Keyboard",
            "SD card",
            "Wi-Fi",
            "Logs",
            "QR",
            "IR",
            "Sensors",
            "Battery",
            "Reset",
        ]:
            self.assertIn(capability, doc)

    def test_evidence_format_and_variant_policy_are_documented(self):
        doc = (REPO_ROOT / "docs/HARDWARE_ACCEPTANCE.md").read_text()

        for text in [
            "Evidence Record",
            "Device model",
            "Firmware commit",
            "Release candidate tag",
            "Pass/Fail/Not implemented",
            "Known Hardware Variants",
            "Unsupported Setups",
        ]:
            self.assertIn(text, doc)

    def test_quality_doc_requires_v1_hardware_evidence(self):
        quality = (REPO_ROOT / "docs/QUALITY.md").read_text()

        self.assertIn("docs/HARDWARE_ACCEPTANCE.md", quality)
        self.assertIn("v1.0 hardware evidence", quality)
        self.assertIn("must not be marked hardware-certified", quality)

    def test_assumptions_doc_requires_update_after_hardware_runs(self):
        assumptions = (REPO_ROOT / "docs/ASSUMPTIONS.md").read_text()

        self.assertIn("Update after hardware acceptance", assumptions)
        self.assertIn("docs/HARDWARE_ACCEPTANCE.md", assumptions)
        self.assertIn("real Cardputer ADV evidence", assumptions)


if __name__ == "__main__":
    unittest.main()
