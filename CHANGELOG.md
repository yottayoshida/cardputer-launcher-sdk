# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog, and this project uses semantic versioning once a stable public API exists.

## [Unreleased]

### Added

- Keyboard command search with incremental filtering for the launcher app list and Webhook Launcher commands (Tab to search, Cancel to clear, filter persists on Tab-exit).
- fn-layer arrow keys (`fn+;.,/`) as the primary navigation path, with legacy `w/s/y/n/j/k` shortcuts kept as Navigation-mode-only aliases so they no longer collide with typed text entry.
- `TextInput` cursor movement, field-level max length, default-value seeding, and horizontal scroll window for small-screen display.
- Webhook Launcher typed command inputs (short text, choice, boolean, confirmation) with per-field required/default/maxLength/choices validation.
- Bounded `{{input.<key>}}` templating for Webhook Launcher URLs, headers, and JSON body values, with percent-encoding for URL substitution, JSON-escaped or type-converted substitution for body values, and a reserved `secret` namespace for future secret-backed placeholders.
- Webhook Launcher categories, descriptions, risk labels, and a dry-run preview screen; `risk: high` commands require both `confirm` and `requirePreview`.
- Case-insensitive URL scheme comparison in firmware config validation, matching the host-side validator's RFC 3986 behavior.

### Changed

- Bare Backspace now triggers Back (previously required `fn+Backspace`); `fn+Backspace` now triggers a new Clear action that empties the active text field.

## [0.2.0] - 2026-07-02

### Added

- GitHub Actions CI for host tests, config validation, firmware build, docs link checks, secret scanning, and release metadata validation.
- Local CI helper scripts and documentation for reproducing failures before opening a PR.
- Hardware acceptance checklist and v1.0 evidence gate for Cardputer ADV release certification.
- App manifest metadata draft with permission and capability vocabulary for built-in apps.
- Lightweight lifecycle hooks for focus, blur, tick, suspend, and resume foundations.
- SD-card app-pack layout for Webhook Launcher with manifest and commands files.
- whole-tree validator coverage for app packs, schema versions, runtime directories, and partial-write residue.
- Firmware recovery helper for creating missing non-secret SD-card directories.
- Safer secret provisioning design, header `secretRef` prototype, and disabled-by-default sync settings validation.
- HTTP transport hardening with HTTPS policy, local-development HTTP exceptions, timeouts, bounded GET retry, request body limits, response preview limits, and error classification.

### Changed

- Require `settings.json` to include top-level `"version": 1` for host and firmware validation.

### Fixed

- Tighten firmware Webhook Launcher config validation for HTTPS URLs, command arrays, header values, GET bodies, and field-specific errors.

## [0.1.0] - 2026-06-29

### Added

- Initial PlatformIO firmware skeleton for Cardputer Launcher SDK.
- Compile-time app registry and basic app lifecycle.
- Keyboard-first launcher menu.
- Tiny UI primitives: menu, status bar, toast, text input, and simple confirmation flow.
- SD-card JSON settings and Webhook Launcher command examples.
- Wi-Fi and HTTP helper boundaries.
- Redacted SD-card JSONL request logging.
- About and Log Viewer built-in apps.
- Host-side sample config validator and unit tests.
- Design, architecture, UX, security, quality, roadmap, adversarial review, and release note documentation.
- Dual license files for Apache-2.0 OR MIT.
