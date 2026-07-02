# Issue 5 App Lifecycle and Manifest Draft Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a conservative app manifest draft and lightweight lifecycle hooks that prepare the appkit for v0.3/v1.0 without dynamic loading or a full state machine.

**Architecture:** Add a small `AppManifest` value type and optional no-op lifecycle hooks to `App`. Keep `Launcher` as the only lifecycle caller and limit active behavior to focus, blur, and tick. Document the manifest and lifecycle direction in the existing architecture and design docs.

**Tech Stack:** PlatformIO firmware C++, Python `unittest` source-level tests, existing Markdown docs.

## Global Constraints

- Do not require hardware for verification.
- Do not add runtime dependencies.
- Do not add dynamic binary loading from SD card.
- Do not implement the full #15 lifecycle state machine in this PR.
- Keep issue and PR text in English.
- Use TDD: write failing Python source/docs tests before firmware or docs implementation.

---

### Task 1: Add Manifest and Lifecycle Source Tests

**Files:**
- Create: `tests/test_app_manifest_source.py`

**Interfaces:**
- Consumes: existing source files under `src/launcher` and `src/apps`.
- Produces: source-level tests that define the new `AppManifest`, permission/capability constants, default hooks, and launcher hook calls.

- [ ] **Step 1: Write failing source tests**

Create `tests/test_app_manifest_source.py` with tests for:

```python
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
```

- [ ] **Step 2: Run test to verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_app_manifest_source -v`

Expected: FAIL because `src/launcher/AppManifest.h` does not exist yet.

### Task 2: Implement Firmware Manifest and Lifecycle Hooks

**Files:**
- Create: `src/launcher/AppManifest.h`
- Modify: `src/launcher/App.h`
- Modify: `src/launcher/Launcher.h`
- Modify: `src/launcher/Launcher.cpp`
- Modify: `src/main.cpp`
- Modify: `src/apps/WebhookLauncherApp.h`
- Modify: `src/apps/WebhookLauncherApp.cpp`
- Modify: `src/apps/LogViewerApp.h`
- Modify: `src/apps/LogViewerApp.cpp`

**Interfaces:**
- Consumes: tests from Task 1.
- Produces: `AppManifest App::manifest() const`, lifecycle hooks on `App`, `Launcher::tick(AppContext&)`, and built-in manifest overrides.

- [ ] **Step 1: Add `AppManifest.h`**

Add permission and capability constants as `constexpr uint32_t` bit flags, plus:

```cpp
struct AppManifest {
  const char* id;
  const char* name;
  const char* version;
  const char* category;
  const char* configPath;
  uint32_t permissions;
  uint32_t capabilities;
};
```

- [ ] **Step 2: Add default manifest and lifecycle hooks to `App`**

Include `launcher/AppManifest.h` in `App.h`. Add `virtual AppManifest manifest() const` that returns `{id(), name(), CARDPUTER_LAUNCHER_VERSION, "system", "", 0, 0}`. Add no-op `onFocus`, `onBlur`, `onTick`, `onSuspend`, and `onResume` methods.

- [ ] **Step 3: Add `Launcher::tick()` and hook calls**

Add `void tick(AppContext& ctx)` to `Launcher`. Call `activeApp_->onTick(ctx)` from it. In `openSelected()`, call `onStart(ctx)` then `onFocus(ctx)`. In Back handling, call `onBlur(ctx)` before `onStop(ctx)`.

- [ ] **Step 4: Call `launcher.tick(ctx)` from `loop()`**

Call `launcher.tick(ctx)` after input handling and before render.

- [ ] **Step 5: Add manifest overrides for Webhook Launcher and Log Viewer**

Webhook Launcher returns automation metadata with `/apps/webhook_launcher.json`, storage read/write, network HTTP, input keyboard, display draw, command list, command execute, and config reload.

Log Viewer returns diagnostics metadata with storage read, input keyboard, display draw, and log view.

- [ ] **Step 6: Run test to verify GREEN**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_app_manifest_source -v`

Expected: PASS.

### Task 3: Add Manifest and Lifecycle Documentation Tests

**Files:**
- Create: `tests/test_app_manifest_docs.py`

**Interfaces:**
- Consumes: docs to be updated in Task 4.
- Produces: failing docs coverage for manifest fields, examples, permissions, lifecycle terms, and dynamic loading deferral.

- [ ] **Step 1: Write failing docs tests**

Create `tests/test_app_manifest_docs.py` with:

```python
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
```

- [ ] **Step 2: Run test to verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_app_manifest_docs -v`

Expected: FAIL because the required docs are not yet detailed enough.

### Task 4: Update Architecture and Design Docs

**Files:**
- Modify: `docs/ARCHITECTURE.md`
- Modify: `docs/DESIGN.md`
- Modify: `CHANGELOG.md`

**Interfaces:**
- Consumes: tests from Task 3.
- Produces: documented manifest draft, two manifest examples, permission vocabulary, lifecycle call sequence, and explicit dynamic loading deferral.

- [ ] **Step 1: Update architecture docs**

Add sections that explain `AppManifest`, the lightweight lifecycle call sequence, and why suspend/resume are reserved for #15.

- [ ] **Step 2: Update design docs**

Expand the app manifest model with field definitions, permission names, capability names, Webhook Launcher and Log Viewer examples, and the sentence: "v1.0 does not load binary apps from SD card unless a separate sandbox design exists."

- [ ] **Step 3: Update changelog**

Add an `Unreleased` entry describing the manifest draft and lifecycle hook foundation.

- [ ] **Step 4: Run docs tests to verify GREEN**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_app_manifest_docs -v`

Expected: PASS.

### Task 5: Final Verification, Review, and PR

**Files:**
- Review all changed files.

**Interfaces:**
- Produces: PR-ready branch for issue #5.

- [ ] **Step 1: Run local verification**

Run:

```bash
git diff --check
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests
PYTHONDONTWRITEBYTECODE=1 python3 scripts/validate_configs.py
pio run
```

Expected: all commands pass.

- [ ] **Step 2: Run adversarial review**

Run: `codex review --uncommitted`

Expected: no Critical, High, or unresolved Medium correctness issues. Fix all valid findings and rerun verification.

- [ ] **Step 3: Create PR**

Commit the branch, push `issue-5-app-lifecycle-manifest`, and open a PR with `Closes #5`.
