# Issue 10 Secret Provisioning Prototype Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a documented safer secret provisioning design, a minimal header `secretRef` prototype, and disabled-by-default sync settings validation for issue #10.

**Architecture:** Keep the prototype inside existing storage/config boundaries. `SecretStore` reads untracked SD-card secrets, `ConfigLoader` resolves webhook header refs when a store is provided, and host validation mirrors the accepted JSON shape.

**Tech Stack:** PlatformIO Arduino C++, ArduinoJson, Python `unittest`, existing SD-card sample config tooling.

## Global Constraints

- Docs and issue artifacts must be in English.
- Do not commit real secrets.
- Do not claim hardware-backed or encrypted storage in this PR.
- `sdcard/secrets.json` must be ignored by git.
- `secretRef` support is prototype-only and limited to webhook header values.
- Sync must default to disabled and require explicit opt-in.
- Every behavior change must start with a failing test.

---

### Task 1: Host Validation and Redaction Prototype

**Files:**
- Modify: `scripts/validate_configs.py`
- Create: `tests/test_secret_provisioning.py`

**Interfaces:**
- Produces: `validate_secret_ref_value(value, path)`, `resolve_secret_refs(value, secrets)`, and `redact_known_secret_values(value, secrets)`.
- Produces: `validate_settings(data)["sync"]["enabled"]`, defaulting to `False`.

- [ ] **Step 1: Write the failing tests**

```python
def test_accepts_webhook_header_secret_reference(self):
    data = {
        "version": 1,
        "commands": [
            {
                "name": "Deploy",
                "method": "POST",
                "url": "https://example.com/deploy",
                "headers": {
                    "Authorization": {
                        "secretRef": "webhooks.deploy.authorization"
                    }
                },
            }
        ],
    }

    result = validate_webhook_config(data)

    self.assertEqual(
        result["commands"][0]["headers"]["Authorization"],
        {"secretRef": "webhooks.deploy.authorization"},
    )


def test_resolves_and_redacts_known_secret_values(self):
    value = {"Authorization": {"secretRef": "webhooks.deploy.authorization"}}
    secrets = {"webhooks.deploy.authorization": "Bearer live-token"}

    resolved = resolve_secret_refs(value, secrets)
    redacted = redact_known_secret_values(str(resolved), secrets)

    self.assertEqual(resolved["Authorization"], "Bearer live-token")
    self.assertNotIn("live-token", redacted)
```

- [ ] **Step 2: Run the focused tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_secret_provisioning -v`

Expected: FAIL because the new test module or imported functions do not exist yet.

- [ ] **Step 3: Implement minimal host prototype**

Add secret reference validation, recursive resolution, known-secret redaction, and optional sync validation to `scripts/validate_configs.py`.

- [ ] **Step 4: Run focused and full host tests**

Run:

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_secret_provisioning -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests
PYTHONDONTWRITEBYTECODE=1 python3 scripts/validate_configs.py
```

Expected: all pass.

### Task 2: Firmware SecretStore Prototype

**Files:**
- Create: `src/storage/SecretStore.h`
- Create: `src/storage/SecretStore.cpp`
- Modify: `src/storage/ConfigLoader.h`
- Modify: `src/storage/ConfigLoader.cpp`
- Modify: `src/launcher/AppContext.h`
- Modify: `src/apps/WebhookLauncherApp.cpp`
- Modify: `src/main.cpp`
- Create: `tests/test_secret_provisioning_source.py`

**Interfaces:**
- Produces: `SecretStore::begin(bool sdAvailable)`.
- Produces: `SecretStore::resolve(const String& ref, String& value)`.
- Extends: `ConfigLoader::loadWebhooks(std::vector<WebhookCommand>& commands, SecretStore* secrets = nullptr)`.

- [ ] **Step 1: Write source-level failing tests**

Tests must assert that `SecretStore` exists, reads `/secrets.json`, `ConfigLoader` accepts a `SecretStore*`, Webhook Launcher passes `&ctx.secrets`, and `main.cpp` initializes the store.

- [ ] **Step 2: Run focused tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_secret_provisioning_source -v`

Expected: FAIL because `SecretStore` and the new call path do not exist.

- [ ] **Step 3: Implement minimal firmware resolver**

Implement a flat `/secrets.json` map lookup. Resolve only header values shaped as `{"secretRef":"name"}`. If a command contains a secret ref without a configured `SecretStore`, return a config error.

- [ ] **Step 4: Verify source tests and firmware build**

Run:

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_secret_provisioning_source -v
pio run
```

Expected: source tests pass and firmware builds.

### Task 3: Documentation, Samples, and Git Ignore

**Files:**
- Modify: `.gitignore`
- Create: `docs/SECRET_PROVISIONING.md`
- Modify: `docs/SECURITY.md`
- Modify: `docs/ARCHITECTURE.md`
- Create: `sdcard/secrets.example.json`
- Create: `sdcard/apps/webhook_launcher_secret_ref.example.json`
- Create: `tests/test_secret_provisioning_docs.py`

**Interfaces:**
- Produces: English docs that map issue #10 acceptance criteria.
- Produces: ignored `sdcard/secrets.json` policy and checked-in examples without real secrets.

- [ ] **Step 1: Write failing documentation tests**

Tests must require docs for ESP32-S3 storage options, provisioning flow, `secretRef`, opt-in sync, lost SD card, malicious command pack, network observer, and ignored `sdcard/secrets.json`.

- [ ] **Step 2: Run focused docs tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_secret_provisioning_docs -v`

Expected: FAIL because the new docs and examples do not exist.

- [ ] **Step 3: Write docs and examples**

Document the prototype honestly: `/secrets.json` separates shareable command packs from local secrets but does not protect a stolen SD card. Document later NVS/encrypted-NVS or QR provisioning options as future work.

- [ ] **Step 4: Verify all checks**

Run:

```bash
git diff --check
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests
PYTHONDONTWRITEBYTECODE=1 python3 scripts/validate_configs.py
pio run
```

Expected: all pass.

### Task 4: Review, Commit, Push, PR

**Files:**
- Modify: harness ledger and PR body under `/Users/i.yoshida/claude_workspace/issue-harness/yottayoshida-cardputer-launcher-sdk/20260629-090607/`.

**Interfaces:**
- Produces: PR closing #10.

- [ ] **Step 1: Run plan review gates**

Record architecture, security, and UX/operator review results in the ledger.

- [ ] **Step 2: Run Codex review**

Run: `codex review --base origin/main`

Expected: no Critical or Important findings remain.

- [ ] **Step 3: Commit, push, and create PR**

Commit the implementation, push `issue-10-secret-provisioning`, and create a non-draft PR with validation and review evidence.
