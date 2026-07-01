# Issue #31 Hardware Acceptance Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a documented hardware acceptance suite and v1.0 evidence gate for Cardputer ADV.

**Architecture:** Keep the acceptance suite in `docs/HARDWARE_ACCEPTANCE.md`, link it from existing quality and assumptions docs, and lock the coverage with Python doc tests. The change is documentation-only and must not imply real hardware testing has already passed.

**Tech Stack:** Markdown docs, Python `unittest` doc coverage tests, existing PlatformIO command references.

## Global Constraints

- Do not claim hardware tests passed without evidence.
- Keep issue documents and user-facing docs in English.
- Each built-in app must have at least one manual acceptance test.
- v1.0 release readiness must require real device evidence.

---

### Task 1: Hardware Acceptance Doc Coverage

**Files:**
- Create: `tests/test_hardware_acceptance_docs.py`
- Create: `docs/HARDWARE_ACCEPTANCE.md`
- Modify: `docs/QUALITY.md`
- Modify: `docs/ASSUMPTIONS.md`

**Interfaces:**
- Produces: `docs/HARDWARE_ACCEPTANCE.md` as the canonical manual acceptance suite.
- Produces: doc tests that assert required sections and app/hardware coverage.

- [ ] **Step 1: Write failing docs tests**

Create tests that require fresh clone setup, flashing, SD-card preparation, Wi-Fi/webhook procedure, evidence format, known variants, unsupported setups, every built-in app, hardware capability rows, and v1.0 evidence gate wording.

- [ ] **Step 2: Verify RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_hardware_acceptance_docs -v`

Expected: FAIL because `docs/HARDWARE_ACCEPTANCE.md` and the release gate wording do not exist.

- [ ] **Step 3: Add acceptance documentation**

Add the manual checklist, test matrix, evidence template, known variants, unsupported setups, and v1.0 gate. Update `docs/QUALITY.md` and `docs/ASSUMPTIONS.md` to point at the suite and require evidence before changing hardware assumptions.

- [ ] **Step 4: Verify GREEN**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_hardware_acceptance_docs -v`

Expected: PASS.

### Task 2: Final Verification and Review

**Files:**
- Modify: PR body and implementation ledger outside tracked source.

**Interfaces:**
- Consumes: Task 1 doc test and existing project validation commands.
- Produces: PR-ready verification evidence.

- [ ] **Step 1: Run full verification**

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

Expected: no critical/high/important correctness issues. Fix any findings before PR.
