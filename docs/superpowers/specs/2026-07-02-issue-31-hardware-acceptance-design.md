# Issue #31 Hardware Acceptance Design

## Goal

Create a repeatable manual acceptance suite that prevents v1.0 from claiming Cardputer ADV readiness without device evidence.

## Scope

This issue adds documentation and doc-level tests. It does not claim any hardware test has passed, change pin mappings, or implement missing QR, IR, sensor, battery, or reset features.

## Approach

Add `docs/HARDWARE_ACCEPTANCE.md` as the canonical hardware test procedure. The document will cover fresh-clone setup, firmware flashing, SD-card preparation, Wi-Fi/webhook smoke testing, built-in app acceptance, hardware capability matrix, evidence format, known variants, unsupported setups, and v1.0 release gate language.

## Evidence Policy

Every release-candidate run must record:

- Device model and visible variant marking.
- Firmware commit and release candidate tag.
- PlatformIO environment and upload command.
- SD-card preparation notes.
- Wi-Fi network type and whether webhook targets were local, test, or production.
- Pass/fail/not-implemented result per test case.
- Photos, serial logs, SD-card log excerpts, or issue links where useful.

## Manual Test Matrix

The matrix covers display, keyboard, SD card, Wi-Fi, webhook execution, logs, About app, Log Viewer app, QR, IR, sensors, battery, and reset behavior. Unimplemented areas are still listed so maintainers cannot silently skip them during v1.0 certification.

## Release Gate

`docs/QUALITY.md` will require hardware evidence before v1.0. If the checklist has not been run on a real Cardputer ADV, the release must say so and cannot be marked hardware-certified.

## Acceptance Mapping

- Fresh clone checklist exists in `docs/HARDWARE_ACCEPTANCE.md`.
- Each built-in app has at least one manual acceptance case.
- `docs/QUALITY.md` requires v1.0 hardware evidence.
- `docs/ASSUMPTIONS.md` points maintainers to update hardware assumptions after evidence is collected.
