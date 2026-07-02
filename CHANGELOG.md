# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog, and this project uses semantic versioning once a stable public API exists.

## [Unreleased]

### Added

- GitHub Actions CI for host tests, config validation, firmware build, docs link checks, secret scanning, and release metadata validation.
- Local CI helper scripts and documentation for reproducing failures before opening a PR.

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
