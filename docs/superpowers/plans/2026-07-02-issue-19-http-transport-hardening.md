# Issue 19 HTTP Transport Hardening Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add explicit HTTP transport policy, timeouts, bounded retry, body/preview limits, and docs for issue #19.

**Architecture:** Keep policy validation mirrored between host config validation, firmware config loading, and `HttpClient` send-time enforcement. Add only small fields to `WebhookCommand`, `HttpRequest`, and `HttpResponse`, avoiding new dependencies or a large networking abstraction.

**Tech Stack:** PlatformIO Arduino C++, Arduino `HTTPClient`, Python `unittest`, existing SD-card JSON validator.

## Global Constraints

- Docs and issue artifacts must be in English.
- HTTPS is the default and production-safe path.
- `allowLocalHttp` is a local-development-only exception.
- HTTP exceptions must be limited to localhost, 127.0.0.1, or [::1].
- POST requests must not be retried automatically.
- Network calls must have finite connect and read timeouts.
- Request body and response preview sizes must be bounded.
- Every behavior change must start with a failing test.

---

### Task 1: Host Config Transport Policy

**Files:**
- Modify: `scripts/validate_configs.py`
- Modify: `tests/test_validate_configs.py`
- Create: `tests/test_http_transport_policy.py`

**Interfaces:**
- Produces: `allowLocalHttp` normalized as a command boolean.
- Produces: URL validation that accepts `http://127.0.0.1` only when `allowLocalHttp` is true.
- Produces: request body validation that rejects serialized bodies over 2048 bytes.

- [ ] **Step 1: Write failing tests**

Add tests for allowed local HTTP, rejected non-local HTTP with `allowLocalHttp`, rejected non-bool `allowLocalHttp`, and oversized POST body.

- [ ] **Step 2: Run focused tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_http_transport_policy -v`

Expected: FAIL because `allowLocalHttp` and body byte limits do not exist.

- [ ] **Step 3: Implement minimal host validation**

Extend `_validate_url` and `validate_webhook_config` to handle `allowLocalHttp`, loopback-only HTTP, and a 2048-byte serialized body limit.

- [ ] **Step 4: Verify host tests**

Run:

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_http_transport_policy -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests
PYTHONDONTWRITEBYTECODE=1 python3 scripts/validate_configs.py
```

Expected: all pass.

### Task 2: Firmware Config and HTTP Client Policy

**Files:**
- Modify: `src/storage/ConfigLoader.h`
- Modify: `src/storage/ConfigLoader.cpp`
- Modify: `src/network/HttpClient.h`
- Modify: `src/network/HttpClient.cpp`
- Modify: `src/apps/WebhookLauncherApp.cpp`
- Create: `tests/test_http_transport_source.py`

**Interfaces:**
- Produces: `WebhookCommand::allowLocalHttp`.
- Produces: `HttpRequest::allowLocalHttp`.
- Produces: `enum class HttpErrorKind`.
- Produces: constants for connect timeout, read timeout, request body max, response preview max, and GET retry count.

- [ ] **Step 1: Write source-level failing tests**

Tests must require `allowLocalHttp` wiring, `setConnectTimeout`, `setTimeout`, `kMaxRequestBodyBytes`, `kMaxResponsePreviewBytes`, GET-only retry logic, and `HttpErrorKind`.

- [ ] **Step 2: Run focused tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_http_transport_source -v`

Expected: FAIL because source hooks do not exist.

- [ ] **Step 3: Implement minimal firmware hardening**

Add policy helpers, limits, timeout calls, bounded GET retry, and error classification in `HttpClient`. Parse and pass `allowLocalHttp` from command config to request execution.

- [ ] **Step 4: Verify source tests and firmware build**

Run:

```bash
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_http_transport_source -v
pio run
```

Expected: tests pass and firmware builds.

### Task 3: Documentation and Examples

**Files:**
- Modify: `docs/SECURITY.md`
- Modify: `README.md`
- Modify: `CHANGELOG.md`
- Create: `tests/test_http_transport_docs.py`

**Interfaces:**
- Produces: English docs for HTTPS default, local HTTP exception, timeouts, retry policy, body/preview limits, TLS limitations, and future pinning options.

- [ ] **Step 1: Write failing docs tests**

Tests must require docs for `allowLocalHttp`, localhost-only HTTP, timeouts, GET-only retry, body/preview limits, TLS limitations, and certificate pinning future work.

- [ ] **Step 2: Run focused docs tests and confirm RED**

Run: `PYTHONDONTWRITEBYTECODE=1 python3 -m unittest tests.test_http_transport_docs -v`

Expected: FAIL because docs do not yet describe the policy.

- [ ] **Step 3: Update docs**

Add concise transport policy sections without claiming full TLS verification or hardware Wi-Fi validation.

- [ ] **Step 4: Verify all local checks**

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
- Produces: PR closing #19.

- [ ] **Step 1: Record plan review gates**

Record architecture, security, and UX/operator findings in the ledger.

- [ ] **Step 2: Run Codex review**

Run: `codex review --base origin/main`

Expected: no Critical or Important findings remain.

- [ ] **Step 3: Commit, push, and create PR**

Commit the implementation, push `issue-19-http-transport`, and create a non-draft PR with validation and review evidence.
