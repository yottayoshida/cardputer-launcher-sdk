# Issue 30 CI and Release Checks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add repeatable GitHub Actions and local scripts for host validation, firmware build, practical docs/secret checks, and release metadata validation.

**Architecture:** Keep CI jobs thin and make non-build checks executable as local Python scripts with unit coverage. GitHub Actions orchestrates those scripts plus PlatformIO, while `docs/CI.md` documents exactly how to reproduce failures locally.

**Tech Stack:** GitHub Actions, Python `unittest`, PlatformIO, shell commands available on GitHub-hosted Ubuntu runners.

## Global Constraints

- Do not require hardware for CI.
- Do not add non-standard Python package dependencies for checks.
- Keep secret scanning practical and placeholder-aware rather than claiming comprehensive DLP coverage.
- Release metadata checks validate semver tags, `CHANGELOG.md`, and `docs/RELEASE_NOTES_<tag>.md`.
- Documentation and issue/PR text must be English.

---

### Task 1: Add Tests for CI Support Scripts and Workflow Shape

**Files:**
- Create: `tests/test_release_metadata.py`
- Create: `tests/test_docs_links.py`
- Create: `tests/test_secret_scan.py`
- Create: `tests/test_ci_workflow_source.py`

**Interfaces:**
- Consumes: planned script functions from `scripts/validate_release_metadata.py`, `scripts/check_docs_links.py`, and `scripts/scan_secrets.py`.
- Produces: failing tests that define release metadata, docs link, secret scan, and workflow requirements.

- [ ] **Step 1: Write release metadata tests**

Create tests that assert `v0.1.0` passes, missing release notes fail, and invalid tag names fail.

- [ ] **Step 2: Write docs link tests**

Create tests using a temporary docs tree where a valid relative link passes and a missing relative link fails.

- [ ] **Step 3: Write secret scan tests**

Create tests that allow placeholders such as `Bearer REPLACE_WITH_TOKEN` and reject real-looking GitHub, OpenAI, AWS, and private-key patterns.

- [ ] **Step 4: Write workflow source tests**

Create tests that assert `.github/workflows/ci.yml` exists, triggers on pull requests and `v*` tags, runs host tests/config validation, runs `pio run`, uses PlatformIO cache paths, runs docs/secret checks, and validates release metadata on tag refs.

- [ ] **Step 5: Run tests and verify RED**

Run: `python3 -m unittest tests.test_release_metadata tests.test_docs_links tests.test_secret_scan tests.test_ci_workflow_source`

Expected: FAIL because the scripts and workflow do not exist yet.

### Task 2: Implement Local CI Check Scripts

**Files:**
- Create: `scripts/validate_release_metadata.py`
- Create: `scripts/check_docs_links.py`
- Create: `scripts/scan_secrets.py`

**Interfaces:**
- Produces: CLI commands used by GitHub Actions and local developers.
- Produces: importable functions used by unit tests.

- [ ] **Step 1: Implement `validate_release_metadata.py`**

Validate that a release tag matches `vMAJOR.MINOR.PATCH`, that `CHANGELOG.md` has a matching `## [MAJOR.MINOR.PATCH] - YYYY-MM-DD` section, and that `docs/RELEASE_NOTES_<tag>.md` exists with a title beginning with `# <tag>`.

- [ ] **Step 2: Implement `check_docs_links.py`**

Scan Markdown files under README, docs, and examples. Validate local relative links exist, including fragment anchors for Markdown heading targets. Skip external URLs, mailto links, and pure anchors that point inside the current file.

- [ ] **Step 3: Implement `scan_secrets.py`**

Scan tracked text files or a provided root. Reject high-confidence token/private-key patterns while allowing obvious placeholders such as `REPLACE_WITH_TOKEN`, `YOUR_*`, `example`, and `REDACTED`.

- [ ] **Step 4: Run focused tests and verify GREEN**

Run: `python3 -m unittest tests.test_release_metadata tests.test_docs_links tests.test_secret_scan`

Expected: PASS.

### Task 3: Add GitHub Actions CI Workflow

**Files:**
- Create: `.github/workflows/ci.yml`

**Interfaces:**
- Consumes: local scripts from Task 2.
- Produces: PR and tag checks for host validation, docs/secret checks, firmware build, artifacts, and release metadata.

- [ ] **Step 1: Add workflow**

Create one workflow with jobs:
- `host-tests`: run unit tests and config validation.
- `docs-and-secrets`: run docs link and secret scans.
- `firmware-build`: install PlatformIO, cache PlatformIO directories, run `pio run`, and upload firmware build outputs when present.
- `release-metadata`: run only for tag refs matching `v*` and validate release metadata.

- [ ] **Step 2: Run workflow source tests**

Run: `python3 -m unittest tests.test_ci_workflow_source`

Expected: PASS.

### Task 4: Document CI Reproduction

**Files:**
- Create: `docs/CI.md`
- Modify: `README.md`
- Modify: `CHANGELOG.md`

**Interfaces:**
- Produces: operator-facing reproduction commands and release checklist documentation.

- [ ] **Step 1: Add CI documentation**

Document every CI job, local reproduction commands, PlatformIO cache notes, release tag metadata requirements, and what artifacts are uploaded.

- [ ] **Step 2: Link CI docs from README**

Add a short pointer to `docs/CI.md` near the validation/build sections.

- [ ] **Step 3: Add changelog entry**

Add an `Unreleased` entry for CI and release validation.

### Task 5: Final Verification and Review

**Files:**
- Review all changed files.

**Interfaces:**
- Produces: PR-ready branch for issue #30.

- [ ] **Step 1: Run full local verification**

Run:

```bash
git diff --check
python3 -m unittest discover -s tests
python3 scripts/validate_configs.py
python3 scripts/check_docs_links.py
python3 scripts/scan_secrets.py
python3 scripts/validate_release_metadata.py v0.1.0
pio run
```

- [ ] **Step 2: Run adversarial review**

Run `codex review --uncommitted` and fix all Critical/Important findings.

- [ ] **Step 3: Create PR**

Push branch `issue-30-ci-release-checks` and open a PR with `Closes #30`.
