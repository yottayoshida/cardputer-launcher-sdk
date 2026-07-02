# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog, and this project uses semantic versioning once a stable public API exists.

## [Unreleased]

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
