# v0.3.0 — Typed Inputs, Templating, and Secret-Backed Placeholders

## Highlights

- Keyboard command search with incremental filtering, and fn-layer arrow keys (`fn+;.,/`) promoted to the primary navigation path.
- Webhook Launcher typed command inputs (short text, choice, boolean, confirmation) with per-field validation, `{{input.<key>}}` templating in URLs/headers/bodies, categories, risk labels, and a dry-run preview screen.
- `SecretProvider` abstraction (`SdSecretProvider` reads `/secrets.json`) with a shared minimum-length and no-control-character policy enforced at resolve time for every secret-consuming call site.
- `{{secret.<ref>}}` templating for URLs and JSON bodies, and `{"secretRef": "<ref>"}` support for `wifi.password`, alongside the existing header support.
- Per-command `RedactionRegistry`: resolved secret values (raw, percent-encoded, and JSON-escaped forms) are redacted from the dry-run preview, the confirm screen, the response preview, and the JSONL log; a command whose secrets exceed the registry's capacity is aborted rather than shown with partial redaction.

## Included Features

- Everything in v0.2.0, plus:
- Bare Backspace triggers Back; `fn+Backspace` triggers a new Clear action for text fields.
- Case-insensitive URL scheme comparison in firmware config validation, matching the host-side validator.
- Optional `--secrets-file` flag for `scripts/validate_configs.py` to confirm every declared secretRef resolves before flashing an SD card.

## Known Limitations

- Cardputer ADV hardware behavior for this release has not yet been re-validated on physical hardware (see `docs/HARDWARE_ACCEPTANCE.md`); this is a known gap carried from v0.2.0.
- No encrypted or NVS-backed secret storage; `/secrets.json` on the SD card remains plaintext, readable by anyone with physical access to the card.
- No certificate pinning or custom trust-anchor support.
- `scripts/scan_secrets.py` and `docs/SECRET_PROVISIONING.md` were not updated in this release (implementing-session tooling constraint); their content should be reviewed against the new `{{secret.<ref>}}` syntax before v1.0.

## Validation Performed

- `python3 scripts/validate_configs.py`: passed.
- `python3 scripts/validate_configs.py sdcard --secrets-file sdcard/secrets.example.json`: passed, confirming the sample secretRef resolves.
- `python3 -m unittest discover -s tests`: passed, 190 tests.
- `pio run`: passed for the `cardputer_adv` environment (RAM 15.3%, Flash 34.0%).
- Codex adversarial code review (multiple rounds) and a test-adversarial review pass on both PRs in this release.
- QA, security, and UX review passes on both PRs; findings were fixed or documented as known limitations above.

Manual hardware flashing was not performed for this release.

## Next Milestones

- Hardware acceptance pass for the Cardputer ADV target (`docs/HARDWARE_ACCEPTANCE.md` v1.0 certification gate).
- Remaining v1.0 backlog: additional built-in apps (Notes, IR Remote, QR handoff, Wi-Fi profile manager) beyond Webhook Launcher.
- Safer secret provisioning (NVS/encrypted-NVS backend) once hardware evidence exists for the current SD-based flow.
