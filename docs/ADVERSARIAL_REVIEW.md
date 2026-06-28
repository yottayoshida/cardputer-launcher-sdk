# Adversarial Review

## Plan Review Loop

### Perspective: Embedded Engineer

- Risk: The plan could assume a perfect Cardputer board ID and fail at build time.
- Severity: High.
- Fix: Use a documented ESP32-S3 board fallback, keep hardware assumptions explicit, and attempt PlatformIO build before release.
- Applied: `docs/ASSUMPTIONS.md`, `platformio.ini`, and README explain the board assumption.

### Perspective: OSS Maintainer

- Risk: The project could look like a grand framework while shipping a fragile sketch.
- Severity: High.
- Fix: Separate v1.0 vision from v0.1 scope, document non-goals, and keep abstractions small.
- Applied: `docs/DESIGN.md` and `docs/PLAN.md` split vision, scope, and exclusions.

### Perspective: Security Reviewer

- Risk: Webhook secrets can leak through examples, logs, URLs, headers, or SD-card loss.
- Severity: High.
- Fix: Use placeholders, redaction, clear warnings, confirmation flags, and a future secret-storage issue.
- Applied: `docs/SECURITY.md`, sample configs, and logging scope.

### Perspective: UX Reviewer

- Risk: Tiny-screen UX could become unreadable or confusing when Wi-Fi or config fails.
- Severity: Medium.
- Fix: Define explicit failure states and short status messages.
- Applied: `docs/UX.md` and app requirements.

### Perspective: User With No Hardware

- Risk: A user cannot learn anything if the repo only works on the device.
- Severity: Medium.
- Fix: Include docs, sample configs, and host-side validation tests.
- Applied: README and `tests/test_validate_configs.py`.

### Perspective: User Configuring Webhooks

- Risk: Config shape might be unclear, causing runtime parse errors.
- Severity: Medium.
- Fix: Provide sample JSON and a validator that checks required fields, methods, URL scheme, headers, and body shape.
- Applied: `sdcard/` examples and planned validator.

### Perspective: Future Contributor

- Risk: App, UI, network, and storage responsibilities could be tangled.
- Severity: Medium.
- Fix: Split small files by responsibility and document boundaries.
- Applied: `docs/ARCHITECTURE.md` and planned file layout.

### Perspective: Release Manager

- Risk: The release could claim unverified build, push, or GitHub release status.
- Severity: High.
- Fix: Record validation results, attempt build, and report exact blockers.
- Applied: `docs/QUALITY.md` and release report requirements.

## Plan Self-Approval

- v0.1 scope is buildable: yes, because it uses static app registration and config-driven webhooks only.
- v1.0 vision remains exciting but separated from v0.1: yes.
- Implementation choices are justified: yes.
- Security limitations are explicit: yes.
- Docs and examples are sufficient: yes, pending implementation alignment.
- Release criteria are testable: yes.

## Code Review

### Perspective: Embedded Correctness

- Risk: PlatformIO registry package resolution could block first build.
- Severity: Medium.
- Fix: Pin library dependencies to official GitHub repositories.
- Applied: `platformio.ini` uses GitHub URLs and `pio run` passes.

### Perspective: Memory Safety

- Risk: Dynamic app loading or unbounded config parsing would make memory behavior hard to reason about.
- Severity: High.
- Fix: Use static app registration and keep command config small. Avoid dynamic plugins in v0.1.
- Applied: `AppRegistry` has a fixed app count and Webhook Launcher is compile-time registered.

### Perspective: Error Handling

- Risk: Missing SD card, malformed config, Wi-Fi failure, and HTTP failure could silently fail.
- Severity: High.
- Fix: Store explicit errors in config, Wi-Fi, and HTTP helpers and render short messages.
- Applied: `ConfigLoader`, `WifiManager`, `HttpClient`, and `WebhookLauncherApp` report visible status strings.

### Perspective: Config Parsing Robustness

- Risk: The firmware parser is less strict than the host validator.
- Severity: Medium.
- Fix: Keep host validator strict for examples and document v0.2 schema-hardening work in GitHub issues.
- Applied: v0.1 sample configs validate, and runtime rejects missing names, URLs, and unsupported methods.

### Perspective: Network Safety

- Risk: The HTTP helper can send `http://` URLs even though examples require HTTPS.
- Severity: Medium.
- Fix: Keep helper generic for local development but make docs and validator prefer HTTPS. Track stricter policy as v0.2 work.
- Applied: README and SECURITY warn about HTTPS limitations.

### Perspective: Secret Leakage

- Risk: Headers, URLs, and response previews can contain secrets.
- Severity: High.
- Fix: Do not log headers or request bodies, redact secret-like URLs/previews, and keep examples fake.
- Applied: `LogStore` redaction and sample placeholders.

### Perspective: UX Failure States

- Risk: A user could be stuck after config failure.
- Severity: Medium.
- Fix: Webhook Launcher shows the error and allows Enter to reload after fixing SD config.
- Applied: `WebhookLauncherApp::render` and `onInput`.

### Perspective: Documentation Accuracy

- Risk: Docs could claim hardware validation or registry availability that was not proven.
- Severity: High.
- Fix: Record exact validation commands and limitations.
- Applied: `docs/QUALITY.md`, `docs/ASSUMPTIONS.md`, and release notes.

### Perspective: Release Readiness

- Risk: Release could ship without a build or tests.
- Severity: High.
- Fix: Run host tests, config validation, and PlatformIO build before commit/tag/release.
- Applied: All three commands passed on 2026-06-29.

## Code Self-Approval

- The repo has a coherent structure: yes.
- v0.1 features are implemented or explicitly scoped out: yes.
- Sample config exists and validates: yes.
- README instructions match current behavior: yes.
- No real secrets are checked in: yes; placeholders and test-only fake strings are present.
- Build was attempted and passed: yes.
- Validation result is documented: yes.
- Changelog and release notes exist: yes.
- Licenses exist: yes.
