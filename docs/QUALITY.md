# Quality

## Validation Strategy

v0.1 uses three validation paths:

1. Host-side JSON config validation.
2. Python unit tests for the validator and redaction helpers.
3. PlatformIO firmware build attempt when dependencies are available.

## Commands

```bash
python3 scripts/validate_configs.py
python3 -m unittest discover -s tests
pio run
```

## Manual Hardware Test Plan

1. Boot without SD card. Expected: visible SD/config error, no crash.
2. Boot with sample SD card and placeholder Wi-Fi. Expected: Wi-Fi failure is visible and logged if possible.
3. Boot with real low-privilege Wi-Fi and test webhook. Expected: command list loads, confirmation appears, request sends, result is shown.
4. Inspect `/logs/launcher.jsonl`. Expected: no full authorization token is present.
5. Edit webhook JSON to invalid syntax. Expected: parse error is shown.

## Release Criteria

- Licenses present.
- README matches actual behavior.
- Docs cover architecture, UX, security, quality, roadmap, and release notes.
- Sample configs validate.
- Unit tests pass.
- PlatformIO build attempted and result recorded.
- Adversarial code review completed.
- Git commit and `v0.1.0` tag created.

## Validation Results

Completed on 2026-06-29:

- `python3 scripts/validate_configs.py`: passed, validated settings and 2 webhook commands.
- `python3 -m unittest discover -s tests`: passed, 6 tests.
- `pio run`: passed for `cardputer_adv` using the `esp32-s3-devkitc-1` PlatformIO board target and GitHub-pinned library dependencies.

Hardware flashing and real Cardputer ADV SD/Wi-Fi/webhook behavior still require manual device testing.
