import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


class AppManifestSourceTests(unittest.TestCase):
    def read(self, relative_path):
        return (REPO_ROOT / relative_path).read_text()

    def test_manifest_type_defines_required_fields(self):
        text = self.read("src/launcher/AppManifest.h")
        for token in (
            "struct AppManifest",
            "const char* id",
            "const char* name",
            "const char* version",
            "const char* category",
            "const char* configPath",
            "uint32_t permissions",
            "uint32_t capabilities",
        ):
            self.assertIn(token, text)

    def test_permission_and_capability_flags_cover_draft_vocabulary(self):
        text = self.read("src/launcher/AppManifest.h")
        for token in (
            "kPermissionStorageRead",
            "kPermissionStorageWrite",
            "kPermissionNetworkHttp",
            "kPermissionInputKeyboard",
            "kPermissionDisplayDraw",
            "kPermissionIrTransmit",
            "kPermissionSensorRead",
            "kCapabilityCommandList",
            "kCapabilityCommandExecute",
            "kCapabilityConfigReload",
            "kCapabilityLogView",
            "kCapabilityNotesEdit",
            "kCapabilityIrSend",
            "kCapabilitySensorView",
        ):
            self.assertIn(token, text)

    def test_app_interface_exposes_default_manifest_and_lifecycle_hooks(self):
        text = self.read("src/launcher/App.h")
        for token in (
            "virtual AppManifest manifest() const",
            "virtual void onFocus(AppContext& ctx)",
            "virtual void onBlur(AppContext& ctx)",
            "virtual void onTick(AppContext& ctx)",
            "virtual void onSuspend(AppContext& ctx)",
            "virtual void onResume(AppContext& ctx)",
        ):
            self.assertIn(token, text)

    def test_launcher_calls_focus_blur_and_tick_hooks(self):
        header = self.read("src/launcher/Launcher.h")
        source = self.read("src/launcher/Launcher.cpp")
        self.assertIn("void tick(AppContext& ctx)", header)
        self.assertIn("activeApp_->onFocus(ctx);", source)
        self.assertIn("activeApp_->onTick(ctx);", source)
        self.assertIn("activeApp_->onBlur(ctx);", source)

    def test_tick_marks_active_app_dirty_for_render(self):
        source = self.read("src/launcher/Launcher.cpp")
        tick_start = source.index("void Launcher::tick(AppContext& ctx)")
        render_start = source.index("void Launcher::render(AppContext& ctx)")
        tick_body = source[tick_start:render_start]
        self.assertIn("activeApp_->onTick(ctx);", tick_body)
        self.assertIn("dirty_ = true;", tick_body)

    def test_builtin_apps_define_manifest_metadata(self):
        webhook = self.read("src/apps/WebhookLauncherApp.cpp")
        logs = self.read("src/apps/LogViewerApp.cpp")
        self.assertIn("AppManifest WebhookLauncherApp::manifest() const", webhook)
        self.assertIn('"/apps/webhook_launcher.json"', webhook)
        self.assertIn("kPermissionNetworkHttp", webhook)
        self.assertIn("AppManifest LogViewerApp::manifest() const", logs)
        self.assertIn("kCapabilityLogView", logs)


if __name__ == "__main__":
    unittest.main()
