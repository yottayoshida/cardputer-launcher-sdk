# Issue #17 SD-Card Layout and Recovery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a validated v1.0 SD-card tree, firmware directory recovery, and documentation for backup/migration/repair.

**Architecture:** Keep host validation in `scripts/validate_configs.py` and firmware recovery in `storage/`. Add a tiny firmware layout helper so directory paths are centralized without changing app behavior. Preserve legacy `/apps/webhook_launcher.json` as a fallback while the sample card moves to `/apps/webhook_launcher/commands.json`.

**Tech Stack:** Python stdlib `unittest`, Arduino/PlatformIO C++, ArduinoJson, SD filesystem API.

## Global Constraints

- Do not add dynamic binary loading.
- Do not create secret-bearing files automatically.
- Preserve existing Webhook Launcher config shape and legacy path compatibility.
- Use TDD: write failing source/host/docs tests before implementation.
- Keep all issue documents and docs in English.

---

### Task 1: Host SD-Tree Validator

**Files:**
- Modify: `tests/test_validate_configs.py`
- Modify: `scripts/validate_configs.py`
- Modify: `sdcard/settings.json`
- Create: `sdcard/apps/webhook_launcher/manifest.json`
- Create: `sdcard/apps/webhook_launcher/commands.json`
- Create: `sdcard/logs/.gitkeep`
- Create: `sdcard/cache/.gitkeep`
- Create: `sdcard/backups/.gitkeep`

**Interfaces:**
- Produces: `validate_root(root: Path) -> dict` that validates the whole tree.
- Produces: `validate_app_manifest(data, path="app manifest") -> dict`.
- Produces: per-file `ValidationError` messages for app-pack files and partial writes.

- [ ] **Step 1: Write failing tests**

Add tests that assert the checked-in sample tree validates, malformed manifests include `apps/webhook_launcher/manifest.json` and `app manifest.id`, malformed command packs include `apps/webhook_launcher/commands.json` and `commands[0].url`, and stale `*.tmp` files fail validation.

- [ ] **Step 2: Verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_validate_configs -v`

Expected: FAIL because app-pack validation, v1.0 sample paths, and partial-write detection do not exist yet.

- [ ] **Step 3: Implement validator and sample tree**

Move the sample Webhook Launcher config to `sdcard/apps/webhook_launcher/commands.json`, add `manifest.json`, add runtime directory marker files, require supported schema versions, detect `*.tmp`, and keep legacy `sdcard/apps/webhook_launcher.json` compatibility.

- [ ] **Step 4: Verify GREEN**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_validate_configs -v`

Expected: PASS.

### Task 2: Firmware Layout Recovery

**Files:**
- Create: `src/storage/SdLayout.h`
- Create: `src/storage/SdLayout.cpp`
- Modify: `src/storage/ConfigLoader.h`
- Modify: `src/storage/ConfigLoader.cpp`
- Modify: `src/storage/LogStore.cpp`
- Modify: `src/main.cpp`
- Create: `tests/test_firmware_sd_layout_source.py`

**Interfaces:**
- Produces: `bool ensureSdLayout(bool sdAvailable, String& error)`.
- Produces: `bool ConfigLoader::ensureLayout()`.
- Consumes: existing `ConfigLoader::loadWebhooks`.

- [ ] **Step 1: Write failing source tests**

Assert source contains the v1.0 directory constants, `SD.mkdir` calls for non-secret directories, `ConfigLoader::ensureLayout()`, a main-loop setup call, and v1.0 command path fallback to the legacy path.

- [ ] **Step 2: Verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_firmware_sd_layout_source -v`

Expected: FAIL because the helper and calls do not exist.

- [ ] **Step 3: Implement firmware recovery**

Add centralized SD layout constants/helper, call it from `ConfigLoader::ensureLayout()` during setup, update `LogStore` to use the log directory constant, and make `loadWebhooks()` prefer `/apps/webhook_launcher/commands.json` before `/apps/webhook_launcher.json`.

- [ ] **Step 4: Verify GREEN**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_firmware_sd_layout_source -v`

Expected: PASS.

### Task 3: Documentation and Final Review

**Files:**
- Modify: `README.md`
- Modify: `docs/ARCHITECTURE.md`
- Modify: `docs/DESIGN.md`
- Modify: `CHANGELOG.md`
- Create: `tests/test_sdcard_layout_docs.py`

**Interfaces:**
- Consumes: paths and behavior from Tasks 1 and 2.
- Produces: user-facing backup, migration, validation, and repair docs.

- [ ] **Step 1: Write failing docs tests**

Assert docs mention app-pack paths, `cache/`, `backups/`, legacy compatibility, backup-before-migration, partial-write repair, and the host validation command.

- [ ] **Step 2: Verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_sdcard_layout_docs -v`

Expected: FAIL before docs are updated.

- [ ] **Step 3: Update docs**

Document the layout contract, validation command, backup and migration workflow, firmware-created directories, and repair guidance.

- [ ] **Step 4: Full verification**

Run:

```bash
git diff --check
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests
PYTHONDONTWRITEBYTECODE=1 python3 scripts/validate_configs.py
pio run
codex review --uncommitted
```

Expected: all checks pass; fix any adversarial review findings before PR.
