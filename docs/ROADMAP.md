# Roadmap

## v0.1.0

- Initial launcher shell.
- Built-in Webhook Launcher.
- SD-card JSON examples.
- Wi-Fi and HTTP helper boundaries.
- Redacted local logs.
- Design, planning, security, and release docs.

## v0.2.0

- Confirm Cardputer ADV board and SD pin mapping on hardware.
- Add schema versioning and stricter config errors.
- Improve Log Viewer navigation.
- Extract host-testable core config logic.
- Add contributor guide and hardware test checklist.

Tracked issues:

- [#1 Validate Cardputer ADV hardware target and SD-card pin mapping](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/1)
- [#2 Add strict config schema validation on device and host](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/2)
- [#3 Extract host-testable core logic for config, redaction, and command execution planning](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/3)
- [#4 Improve Log Viewer navigation, retention, and redaction coverage](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/4)

## v0.3.0

- Improve app lifecycle events.
- Add Notes prototype.
- Add IR Remote prototype.
- Add richer keyboard text input.
- Introduce app manifest draft.

Tracked issues:

- [#5 Design and implement a richer app lifecycle with manifest draft](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/5)
- [#6 Add keyboard command search and stronger text input primitives](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/6)
- [#7 Prototype built-in Notes app with offline-first SD storage](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/7)
- [#8 Prototype IR Remote app and hardware capability abstraction](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/8)

## v0.5.0

- QR handoff prototype.
- Sensor Viewer.
- Safer secret provisioning experiment.
- Optional Wi-Fi sync experiment.
- Command categories and search.

Tracked issues:

- [#9 Prototype QR handoff for continuing workflows on phone or browser](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/9)
- [#10 Design safer secret provisioning and optional Wi-Fi sync experiment](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/10)

## v1.0.0

- Stable appkit API.
- Manifest-based app definitions.
- Permission model.
- Polished built-in apps.
- Verified security posture.
- Hardware-tested setup guide.
- Developer-friendly examples and templates.
- Command palette quality that feels like a tiny keyboard-first workflow launcher.
- Safe webhook inputs, templates, transport policy, and secret references.
- Offline-first logs, diagnostics, and audit/export behavior.
- QR handoff, AI workflow command deck, and human-in-the-loop approval model.
- CI, hardware acceptance, threat-model review, and release discipline.

Tracked issues:

- [#11 Define stable v1.0 SDK API, permission model, and app registry contract](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/11)
- [#12 Create v1.0 release hardening checklist, examples, and contributor documentation](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/12)
- [#13 Polish v1.0 launcher command palette, ranking, and navigation](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/13)
- [#14 Define final v1.0 app manifest schema and compatibility rules](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/14)
- [#15 Implement v1.0 app lifecycle state machine and failure policy](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/15)
- [#16 Build device abstraction layer for display, keyboard, SD, Wi-Fi, IR, sensors, and power](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/16)
- [#17 Finalize SD-card app pack layout, migrations, and recovery behavior](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/17)
- [#18 Upgrade Webhook Launcher to v1.0 with inputs, templating, categories, and dry-run preview](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/18)
- [#19 Harden HTTP transport with TLS policy, request limits, retries, and response handling](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/19)
- [#20 Implement v1.0 secret reference model and redaction enforcement](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/20)
- [#21 Create offline-first log, audit, queue, and export subsystem](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/21)
- [#22 Add Wi-Fi profile manager and optional sync control plane](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/22)
- [#23 Ship Sensor Viewer built-in app for Cardputer ADV capabilities](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/23)
- [#24 Finalize Notes app for v1.0 with search, storage format, and recovery](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/24)
- [#25 Finalize IR Remote app with command packs, learning notes, and safety bounds](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/25)
- [#26 Deliver QR handoff v1.0 with signed or expiring continuation payloads](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/26)
- [#27 Build AI workflow command deck and approval app model](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/27)
- [#28 Define and enforce v1.0 permission prompts for risky app capabilities](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/28)
- [#29 Create v1.0 contributor SDK examples and app templates](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/29)
- [#30 Set up CI for config validation, host tests, firmware build, and release checks](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/30)
- [#31 Create hardware acceptance test suite and v1.0 certification checklist](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/31)
- [#32 Perform v1.0 security review and publish threat model](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/32)
- [#33 Build v1.0 documentation site structure and API reference](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/33)
- [#34 Add on-device diagnostics and recovery screen](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/34)
- [#35 Define release train, version policy, and changelog automation for v1.0](https://github.com/yottayoshida/cardputer-launcher-sdk/issues/35)
