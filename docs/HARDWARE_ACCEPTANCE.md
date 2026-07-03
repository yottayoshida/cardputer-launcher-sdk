# Hardware Acceptance

This checklist is the repeatable manual acceptance suite for Cardputer Launcher SDK release candidates. It records device evidence; it does not claim hardware support until a maintainer has run it on a real device.

## Fresh Clone Setup

1. Start from a clean machine or clean workspace.
2. Clone the public repository:

```bash
git clone https://github.com/yottayoshida/cardputer-launcher-sdk.git
cd cardputer-launcher-sdk
```

3. Record the firmware commit:

```bash
git rev-parse HEAD
```

4. Run host validation:

```bash
python3 scripts/validate_configs.py
python3 -m unittest discover -s tests
pio run
```

## Firmware Flashing

1. Connect the Cardputer ADV over USB.
2. Build and upload the firmware:

```bash
pio run --target upload
```

3. Open the serial monitor and capture the boot log:

```bash
pio device monitor
```

Record any board-selection warnings, upload failures, boot loops, or reset messages in the Evidence Record.

## SD-Card Preparation

1. Format an SD card using a filesystem supported by the current Arduino SD stack.
2. Copy the repository `sdcard/` contents to the root of the card.
3. Edit `settings.json` with a test Wi-Fi network.
4. Edit `apps/webhook_launcher.json` with a low-privilege HTTPS webhook target.
5. Run `python3 scripts/validate_configs.py <mounted-sd-root>` before inserting the card.
6. Keep a copy of the exact SD-card tree used for the release candidate.

## Wi-Fi and Webhook Smoke Test

Use a test network and a test webhook endpoint. Do not use production webhooks for acceptance unless the release notes explain why.

1. Boot with the prepared SD card inserted.
2. Confirm the launcher renders and reports SD availability.
3. Open Webhook Launcher.
4. Confirm the command list loads.
5. Select a command with `confirm: true`.
6. Press `n` and confirm the request is canceled.
7. Select the command again, press `y`, and confirm the request result is visible.
8. Inspect `/logs/launcher.jsonl` and confirm obvious authorization tokens are not present.

## Keyboard, Search, and Typed Input Acceptance

1. Press the bare Backspace key (no `fn`) while an app is open. Confirm it returns to the launcher (previously required `fn+Backspace`).
2. In a list, confirm `fn+;` `fn+.` `fn+,` `fn+/` move Up/Down/Left/Right, and that legacy `w`/`s`/`k`/`j` still work.
3. Press Tab in a list to open search. Type a query containing `w`, `s`, `y`, or `n` and confirm those letters filter the list as literal text instead of navigating.
4. With a filter applied, confirm results narrow correctly, an unmatched query shows an empty list without crashing, and `fn+Backspace` clears the query back to the full list.
5. Press Tab to exit search and confirm the filtered list stays applied; press `n` (or `fn+`` ` ``) to exit search from a different attempt and confirm the filter clears instead.
6. Open the "Deploy to Environment" sample command (or an equivalent command with `inputs`). Step through each typed input: cycle the choice field with `fn+,`/`fn+/`, type a value containing `w`/`s`/`y`/`n` into the text field and confirm it is accepted as literal text, and toggle the boolean field with `fn+,`/`fn+/` (confirm `y`/`n` no longer set the boolean value and instead abort the command, consistent with every other field kind).
7. Confirm the dry-run preview screen shows the resolved URL and headers, that any `secretRef`-backed header is masked, and that canceling from preview sends no request.
8. Confirm the request actually delivered to the test endpoint matches what the preview displayed.
9. Add a command with an uppercase-scheme URL (e.g. `HTTPS://example.com/...`) to a test SD card. Confirm it loads (host validator and firmware both accept mixed-case schemes) and confirm the actual HTTP request succeeds against a test endpoint, not just that it passes validation — the underlying ESP32 `HTTPClient` library's own scheme handling has not been hardware-verified.

## Built-In App Acceptance

| App | Manual acceptance case | Expected result |
| --- | --- | --- |
| Webhook Launcher | Load a validated command pack, cancel once, then confirm once against a low-privilege HTTPS webhook. | Command list loads, cancel sends no request, confirm sends one request, result appears, log entry is redacted. |
| Log Viewer | After at least one webhook attempt, open Log Viewer and inspect recent entries. | Recent log lines render without crashing and secret-like values are redacted. |
| Settings/About | Open Settings/About from the launcher. | Version, repository, and support text render clearly without requiring network or SD writes. |

## Hardware Capability Matrix

| Capability | Acceptance action | Evidence |
| --- | --- | --- |
| Display | Boot to launcher, open each built-in app, and photograph the screen. | Photo per screen or release-test video. |
| Keyboard | Navigate up/down, select, back, confirm with `y`, and cancel with `n`. | Test notes listing keys that passed or failed. |
| SD card | Boot without SD, boot with sample SD, and boot with malformed webhook JSON. | Observed error text and SD-card validation output. |
| Wi-Fi | Connect to a test Wi-Fi network from Webhook Launcher. | Serial log or screen result showing connection success or failure. |
| Logs | Create at least one webhook attempt and read `/logs/launcher.jsonl`. | Redacted log excerpt. |
| QR | Record Not implemented unless a release branch includes QR behavior. | Pass/Fail/Not implemented with issue or PR link. |
| IR | Record Not implemented unless a release branch includes IR behavior. | Pass/Fail/Not implemented with issue or PR link. |
| Sensors | Record Not implemented unless a release branch includes sensor behavior. | Pass/Fail/Not implemented with issue or PR link. |
| Battery | Boot on battery power and record whether runtime and charge state are visible. | Pass/Fail/Not implemented plus photo or note. |
| Reset | Press reset or power-cycle after a failed config load. | Device returns to launcher or records the failure mode. |

## Evidence Record

Create one evidence record per release candidate.

```text
Release candidate tag:
Firmware commit:
Device model:
Visible hardware variant marking:
PlatformIO environment:
Upload command:
SD-card source:
Wi-Fi network type:
Webhook endpoint type:
Tester:
Date:

Results:
- Display: Pass/Fail/Not implemented - evidence:
- Keyboard: Pass/Fail/Not implemented - evidence:
- SD card: Pass/Fail/Not implemented - evidence:
- Wi-Fi: Pass/Fail/Not implemented - evidence:
- Webhook Launcher: Pass/Fail/Not implemented - evidence:
- Log Viewer: Pass/Fail/Not implemented - evidence:
- Settings/About: Pass/Fail/Not implemented - evidence:
- Logs: Pass/Fail/Not implemented - evidence:
- QR: Pass/Fail/Not implemented - evidence:
- IR: Pass/Fail/Not implemented - evidence:
- Sensors: Pass/Fail/Not implemented - evidence:
- Battery: Pass/Fail/Not implemented - evidence:
- Reset: Pass/Fail/Not implemented - evidence:

Blocking issues:
Follow-up issues:
Release decision:
```

## Known Hardware Variants

- Primary target: M5Stack Cardputer ADV.
- Current build fallback: `esp32-s3-devkitc-1` with the M5Cardputer library.
- Other Cardputer variants, clones, or revised boards are unsupported until a maintainer records evidence with this checklist.

## Unsupported Setups

- Production webhook credentials during acceptance tests.
- SD cards containing unreviewed real secrets.
- Hardware variants without visible device identification.
- Releases marked hardware-certified without a completed Evidence Record.

## v1.0 Certification Gate

Before v1.0, maintainers must attach or link at least one complete Evidence Record for the supported Cardputer ADV target. If this checklist has not been run, the release must not be marked hardware-certified.
