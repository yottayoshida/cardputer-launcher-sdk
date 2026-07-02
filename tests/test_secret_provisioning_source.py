import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class SecretProvisioningSourceTests(unittest.TestCase):
    def test_secret_store_reads_untracked_sd_secret_map(self):
        header = REPO_ROOT / "src/storage/SecretStore.h"
        source = REPO_ROOT / "src/storage/SecretStore.cpp"

        self.assertTrue(header.exists())
        self.assertTrue(source.exists())

        header_text = header.read_text()
        source_text = source.read_text()
        self.assertIn("class SecretStore", header_text)
        self.assertIn("bool resolve(const String& ref, String& value)", header_text)
        self.assertIn('SD.open("/secrets.json"', source_text)
        self.assertIn("doc[ref.c_str()]", source_text)

    def test_config_loader_can_resolve_header_secret_refs(self):
        header = (REPO_ROOT / "src/storage/ConfigLoader.h").read_text()
        source = (REPO_ROOT / "src/storage/ConfigLoader.cpp").read_text()

        self.assertIn("class SecretStore;", header)
        self.assertIn("SecretStore* secrets", header)
        self.assertIn("secretRef", source)
        self.assertIn("object.size() != 1", source)
        self.assertIn("secrets->resolve", source)

    def test_webhook_launcher_passes_secret_store_from_context(self):
        context = (REPO_ROOT / "src/launcher/AppContext.h").read_text()
        app = (REPO_ROOT / "src/apps/WebhookLauncherApp.cpp").read_text()
        main = (REPO_ROOT / "src/main.cpp").read_text()

        self.assertIn('#include "storage/SecretStore.h"', context)
        self.assertIn("SecretStore& secrets", context)
        self.assertIn("ctx.config.loadWebhooks(commands_, &ctx.secrets)", app)
        self.assertIn("SecretStore secrets;", main)
        self.assertIn("secrets.begin(sdAvailable)", main)


if __name__ == "__main__":
    unittest.main()
