import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class FirmwareSdLayoutSourceTests(unittest.TestCase):
    def test_layout_helper_defines_non_secret_paths(self):
        header = (REPO_ROOT / "src/storage/SdLayout.h").read_text()
        source = (REPO_ROOT / "src/storage/SdLayout.cpp").read_text()

        for path in [
            "/apps",
            "/apps/webhook_launcher",
            "/logs",
            "/cache",
            "/backups",
        ]:
            self.assertIn(path, header + source)

        self.assertIn("ensureSdLayout", header)
        self.assertIn("SD.mkdir", source)
        self.assertNotIn("settings.json", source)

    def test_config_loader_exposes_and_uses_layout_recovery(self):
        header = (REPO_ROOT / "src/storage/ConfigLoader.h").read_text()
        source = (REPO_ROOT / "src/storage/ConfigLoader.cpp").read_text()
        main = (REPO_ROOT / "src/main.cpp").read_text()

        self.assertIn("bool ensureLayout()", header)
        self.assertIn("ensureSdLayout(sdAvailable_, lastError_)", source)
        self.assertIn("config.ensureLayout()", main)

    def test_webhook_loading_prefers_app_pack_and_falls_back_to_legacy_path(self):
        source = (REPO_ROOT / "src/storage/ConfigLoader.cpp").read_text()

        pack_path_index = source.index("kSdWebhookCommandsPath")
        legacy_path_index = source.index("kSdLegacyWebhookConfigPath")

        self.assertLess(pack_path_index, legacy_path_index)
        self.assertIn("FILE_READ", source)

    def test_log_store_uses_layout_log_constants(self):
        source = (REPO_ROOT / "src/storage/LogStore.cpp").read_text()

        self.assertIn('#include "storage/SdLayout.h"', source)
        self.assertIn("kSdLogDir", source)
        self.assertIn("kSdLauncherLogPath", source)


if __name__ == "__main__":
    unittest.main()
