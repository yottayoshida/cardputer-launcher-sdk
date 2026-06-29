# Issue 2 Strict Config Schema Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make host and firmware config validation stricter and more consistent for `settings.json` and `apps/webhook_launcher.json`.

**Architecture:** Keep the host validator as the executable schema oracle and bring firmware parsing closer to it with explicit field/index errors. Avoid a new schema dependency; this embedded project already uses small hand-written validation paths.

**Tech Stack:** Python `unittest`, ArduinoJson, PlatformIO Arduino/C++ for M5Cardputer.

## Global Constraints

- Keep sample configs safe: placeholder Wi-Fi and HTTPS webhook examples only.
- Do not add dynamic app loading or runtime plugin execution.
- Firmware parser must keep using ArduinoJson and current SD-card paths.
- Every production behavior change starts with a failing test where host-side testing is available.

---

### Task 1: Add Schema Version and Host Validation Coverage

**Files:**
- Modify: `tests/test_validate_configs.py`
- Modify: `scripts/validate_configs.py`
- Modify: `sdcard/settings.json`

**Interfaces:**
- Consumes: `validate_settings(data)` from `scripts/validate_configs.py`.
- Produces: normalized settings with `version` and `wifi` keys.

- [ ] **Step 1: Write failing settings schema-version tests**

Add tests asserting `settings.version == 1` is required and normalized:

```python
def test_settings_require_schema_version(self):
    with self.assertRaisesRegex(ValidationError, "settings.version must be 1"):
        validate_settings({"wifi": {"ssid": "ssid", "password": "password"}})

def test_rejects_unsupported_settings_version(self):
    with self.assertRaisesRegex(ValidationError, "settings.version must be 1"):
        validate_settings(
            {"version": 2, "wifi": {"ssid": "ssid", "password": "password"}}
        )
```

- [ ] **Step 2: Run tests and verify RED**

Run: `python3 -m unittest tests.test_validate_configs.ConfigValidationTests.test_settings_require_schema_version tests.test_validate_configs.ConfigValidationTests.test_rejects_unsupported_settings_version`

Expected: FAIL because `validate_settings` currently accepts settings with no version.

- [ ] **Step 3: Implement settings version validation**

In `validate_settings`, require `root.get("version") == 1` and include `version` in the normalized return.

- [ ] **Step 4: Update sample config**

Add `"version": 1` to `sdcard/settings.json`.

- [ ] **Step 5: Run tests and verify GREEN**

Run: `python3 -m unittest tests.test_validate_configs.ConfigValidationTests.test_settings_require_schema_version tests.test_validate_configs.ConfigValidationTests.test_rejects_unsupported_settings_version tests.test_validate_configs.ConfigValidationTests.test_sample_settings_are_valid`

Expected: PASS.

### Task 2: Pin Required Invalid Webhook Cases in Host Tests

**Files:**
- Modify: `tests/test_validate_configs.py`

**Interfaces:**
- Consumes: `validate_webhook_config(data)`.
- Produces: regression coverage for issue acceptance cases.

- [ ] **Step 1: Add explicit invalid-case tests**

Add tests for missing URL, invalid header values, GET bodies, non-array `commands`, and non-object command entries. Use `assertRaisesRegex` with field/index paths such as `commands[0].url` and `commands[0].headers.Authorization`.

- [ ] **Step 2: Run focused tests**

Run: `python3 -m unittest tests.test_validate_configs.ConfigValidationTests`

Expected: PASS if existing host behavior already covers the case; otherwise update only `scripts/validate_configs.py` until the field-specific behavior passes.

### Task 3: Bring Firmware Validation Closer to Host Schema

**Files:**
- Modify: `src/storage/ConfigLoader.cpp`

**Interfaces:**
- Consumes: ArduinoJson `JsonDocument` from `/settings.json` and `/apps/webhook_launcher.json`.
- Produces: `lastError_` messages that identify invalid fields or command indexes.

- [ ] **Step 1: Update firmware settings parser**

Require `settings.version == 1`, `settings.wifi.ssid`, and `settings.wifi.password`; return errors like `settings.version must be 1` and `settings.wifi.ssid missing`.

- [ ] **Step 2: Update firmware webhook parser**

Require `version == 1`, non-empty `commands`, object command entries, `GET`/`POST`, HTTPS URL with host, object/string headers, no GET body, and field/index errors such as `commands[0].url must use https`.

- [ ] **Step 3: Build firmware**

Run: `pio run`

Expected: PASS.

### Task 4: Document the Config Schema

**Files:**
- Modify: `README.md`

**Interfaces:**
- Produces: concise schema reference for `settings.json` and `apps/webhook_launcher.json`.

- [ ] **Step 1: Add schema reference**

Document required fields, optional fields, HTTPS requirement, GET body rule, and the local command to validate configs.

- [ ] **Step 2: Validate docs and configs**

Run:

```bash
python3 scripts/validate_configs.py
python3 -c 'import pathlib,re,sys; rx=re.compile("[\\u3040-\\u309f\\u30a0-\\u30ff\\u4e00-\\u9fff]"); bad=[str(p) for root in ["README.md","CHANGELOG.md","docs","examples"] for p in ([pathlib.Path(root)] if pathlib.Path(root).is_file() else pathlib.Path(root).rglob("*")) if p.is_file() and rx.search(p.read_text(errors="ignore"))]; print("\\n".join(bad)); sys.exit(1 if bad else 0)'
```

Expected: config validation PASS; Japanese text search has no matches.

### Task 5: Final Verification and Review Loop

**Files:**
- Review all changed files.

**Interfaces:**
- Produces: PR-ready branch for issue #2.

- [ ] **Step 1: Run full validation**

Run:

```bash
git diff --check
python3 -m unittest discover -s tests
python3 scripts/validate_configs.py
pio run
```

- [ ] **Step 2: Run review gates**

Perform architecture, security, and UX/operator self-review against issue #2 acceptance criteria.

- [ ] **Step 3: Run adversarial diff review**

Run `codex review --base origin/main` if available; otherwise perform the code-reviewer template manually against `origin/main..HEAD`.

- [ ] **Step 4: Fix Critical and Important findings**

Apply fixes, rerun focused tests, and repeat review until no Critical/Important findings remain.

- [ ] **Step 5: Create PR**

Push branch `issue-2-strict-config-schema` and open a PR whose body includes validation evidence and review results, with `Closes #2`.
