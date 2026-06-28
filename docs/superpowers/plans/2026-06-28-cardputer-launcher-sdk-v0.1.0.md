# Cardputer Launcher SDK v0.1.0 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a small, documented, testable v0.1.0 firmware skeleton and release package for Cardputer Launcher SDK.

**Architecture:** A PlatformIO Arduino firmware owns the hardware loop and routes keyboard events through a launcher to compile-time apps. Config and logs live on SD-card JSON/JSONL, while host-side Python validation proves the sample config shape without hardware.

**Tech Stack:** PlatformIO, Arduino ESP32, M5Cardputer, ArduinoJson, Python 3 unittest.

## Global Constraints

- Repository path: `/Users/i.yoshida/claude_workspace/cardputer-launcher-sdk`.
- Version: `v0.1.0`.
- License: Apache-2.0 OR MIT.
- Primary target: M5Stack Cardputer ADV.
- v0.1 scope excludes dynamic plugin loading, encrypted secrets, and v1.0 app marketplace behavior.

---

### Task 1: Repository and Docs

**Files:**
- Create: README, changelog, licenses, docs, sample configs, tests.

**Interfaces:**
- Produces: documented v0.1 scope, sample config schema, validation expectations.

- [x] Create repository and initialize git.
- [x] Record tooling and hardware assumptions.
- [x] Write design, plan, architecture, UX, security, quality, roadmap, and release notes.
- [x] Write failing config validator tests before implementing the validator.

### Task 2: Host Config Validation

**Files:**
- Create: `scripts/validate_configs.py`
- Test: `tests/test_validate_configs.py`

**Interfaces:**
- Produces: `validate_settings`, `validate_webhook_config`, and `redact_secret_like`.

- [ ] Run tests and confirm failure from missing validator.
- [ ] Implement validator.
- [ ] Run tests and confirm pass.

### Task 3: Firmware Skeleton

**Files:**
- Create: `include/CardputerLauncher.h`, `src/main.cpp`, `src/launcher/*`, `src/ui/*`, `src/input/*`, `src/storage/*`, `src/network/*`, `src/apps/*`.

**Interfaces:**
- Produces: compile-time app registry, display context, keyboard event model, Webhook Launcher app.

- [ ] Add app and UI interfaces.
- [ ] Add config loader and log store.
- [ ] Add Wi-Fi and HTTP helpers.
- [ ] Add Webhook Launcher, About, and Log Viewer apps.
- [ ] Add main loop and app registration.

### Task 4: Review and Release

**Files:**
- Modify: `docs/ADVERSARIAL_REVIEW.md`, `docs/QUALITY.md`, `docs/RELEASE_NOTES_v0.1.0.md`.

**Interfaces:**
- Produces: validation evidence and release status.

- [ ] Run adversarial code review.
- [ ] Fix high and medium findings.
- [ ] Run validation/build commands.
- [ ] Commit, tag, push, and create release if GitHub credentials allow.

