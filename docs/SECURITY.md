# Security

## Supported Threat Model for v0.1

v0.1 helps users avoid obvious mistakes. It does not make a lost SD card safe, it does not sandbox apps, and it does not provide a secret vault.

## Secrets

Webhook URLs, headers, and JSON bodies may contain secrets. Keep checked-in examples fake. Use low-privilege webhook tokens. Rotate tokens if an SD card is lost.

The issue #10 prototype adds `secretRef` support for Webhook Launcher header values. See [docs/SECRET_PROVISIONING.md](SECRET_PROVISIONING.md) for the concrete provisioning design, supported storage options, sync control, and threat model.

## SD-Card Risk

The SD card is editable and readable by anyone with physical access. A malicious config can point commands at unexpected URLs or change request bodies.

Mitigations in v0.1:

- Users can inspect plain JSON.
- Commands can require confirmation.
- Logs redact obvious secret-like values.
- Docs discourage high-privilege tokens.

## Logging

`LogStore` redacts common sensitive words and authorization-style values before writing JSONL logs. Redaction is best effort. Avoid placing secrets in names or response bodies.

## Network

Prefer HTTPS endpoints. v0.1 does not implement a complete certificate trust store, certificate pinning, or per-command trust policies. Do not use v0.1 as the only control for sensitive production workflows.

## Issue #10 Threats

- lost SD card: `/secrets.json`, command packs, and logs are readable by anyone with the card.
- malicious command pack: a shared pack can point at unexpected URLs or request a `secretRef` intended for another workflow.
- network observer: HTTPS is required by validation, but TLS trust and future pinning still need hardware and transport hardening.

## Future Security Work

- Config schema versioning and strict validation.
- Broader secret references instead of inline secrets.
- Safer provisioning flow using NVS, encrypted NVS, QR, serial, or browser handoff after hardware evidence exists.
- Certificate pinning or managed trust anchor support.
- Per-app permission model.
- Signed command packs.
- Tamper-evident logs.
