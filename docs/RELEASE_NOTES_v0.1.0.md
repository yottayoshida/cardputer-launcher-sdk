# v0.1.0 — Tiny Apps for Tiny Computers

## Highlights

- Initial OSS release of Cardputer Launcher SDK.
- Keyboard-first launcher shell for M5Stack Cardputer ADV.
- Config-driven Webhook Launcher showcase app.
- SD-card sample settings and webhook command files.
- Redacted request logging.
- Design, architecture, UX, security, quality, and roadmap documentation.

## Included Features

- Compile-time app registry.
- Basic app lifecycle.
- Menu, text input, toast/status, and confirmation primitives.
- Wi-Fi config loading from SD card.
- Static HTTP GET/POST webhook helper.
- Log Viewer and Settings/About apps.
- Host-side config validator.

## Known Limitations

- Cardputer ADV hardware behavior still needs real-device validation.
- `platformio.ini` uses an ESP32-S3 board fallback.
- No dynamic plugin loading.
- No encrypted secret storage.
- No full certificate trust or pinning strategy.
- No webhook input templating.

## Validation Performed

- `python3 scripts/validate_configs.py`: passed.
- `python3 -m unittest discover -s tests`: passed, 6 tests.
- `pio run`: passed for the `cardputer_adv` environment.
- Adversarial plan and code reviews were completed and release-blocking issues were fixed or documented.

Manual hardware flashing was not performed in this release session.

## Next Milestones

- v0.2.0: hardware validation and stronger config schema.
- v0.3.0: richer app lifecycle and first non-webhook utility apps.
- v0.5.0: QR handoff, safer secrets, and optional sync experiments.
- v1.0.0: stable appkit and security-reviewed release.
