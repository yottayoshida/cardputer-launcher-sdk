# Secret Provisioning

This document defines the v0.5 secret provisioning direction for Cardputer Launcher SDK and the issue #10 prototype. It improves command-pack hygiene, but it does not protect a stolen SD card.

## Prototype Boundary

The first prototype supports `secretRef` only for Webhook Launcher header values. The shareable command pack can contain a reference:

```json
{
  "Authorization": {
    "secretRef": "webhooks.deploy.authorization"
  }
}
```

The device resolves that reference from `sdcard/secrets.json`, which is copied to the SD-card root as `/secrets.json`:

```json
{
  "webhooks.deploy.authorization": "Bearer REPLACE_WITH_REAL_TOKEN"
}
```

`sdcard/secrets.json` is ignored by git. Commit `sdcard/secrets.example.json` or command-pack examples, not real local secrets.

Normal request logs include command name, method, URL, status, outcome, and response preview. They do not include request headers, so a header secret resolved through this prototype is not exposed in normal logs.

## Provisioning Flow

1. Copy `sdcard/secrets.example.json` to `sdcard/secrets.json`.
2. Replace placeholder values with low-privilege test tokens.
3. Copy `sdcard/` to the SD card.
4. Keep `sdcard/apps/webhook_launcher.json` and shared command packs free of raw secrets.
5. Run `python3 scripts/validate_configs.py` before copying command packs.
6. Rotate any token if the SD card is lost or shared with the wrong person.

This flow separates local secrets from shareable app configuration. It is not encryption, tamper resistance, or secure element storage.

## ESP32-S3 storage options

- Plain SD-card file: implemented as the issue #10 prototype. It is easy to provision and audit, but a lost SD card exposes the secret file.
- ESP32-S3 NVS: future candidate for device-local provisioning. It can keep secrets off the shareable command pack, but it still needs reset, backup, and extraction-risk analysis.
- Encrypted NVS or flash encryption: future candidate when hardware configuration and bootloader policy are verified on Cardputer ADV.
- Short-lived external provisioning: future candidate using QR, serial, or browser handoff to place temporary tokens on the device.

The project must not claim hardware-backed secret storage until the hardware acceptance suite records evidence for the exact supported target.

## Sync Control

Sync is opt-in. `settings.sync.enabled` defaults to `false`, and a user can keep sync disabled entirely by omitting the `sync` object or setting:

```json
{
  "sync": {
    "enabled": false
  }
}
```

If sync is enabled in a future client, validation requires an HTTPS endpoint. This PR defines the configuration contract and validation behavior only; there is no background sync client.

## Threat Model

### Lost SD card

Impact: anyone with the card can read `/secrets.json`, command packs, and logs.

Controls in this prototype:

- Do not store high-privilege tokens.
- Keep command packs shareable and secrets local.
- Rotate tokens after loss.
- Do not mark this as protection against physical access.

### Malicious command pack

Impact: a command pack can point at an attacker-controlled URL, request unexpected actions, or ask for a `secretRef` intended for another workflow.

Controls in this prototype:

- Keep command packs reviewable plain JSON.
- Use low-privilege, workflow-specific secret refs.
- Require confirmation for risky commands.
- Reserve app permissions and signed command packs for later v1.0 work.

### Network observer

Impact: an observer can see metadata and any plaintext traffic. TLS details still depend on the ESP32 HTTP stack and certificate strategy.

Controls in this prototype:

- Validate HTTPS endpoints for webhook and future sync configs.
- Keep tokens low privilege and rotateable.
- Document TLS and pinning as future hardening work.

## Future Work

- Extend `secretRef` to URLs, JSON bodies, settings, and sync credentials.
- Add a provider boundary for NVS or encrypted NVS after hardware evidence exists.
- Add rotation metadata and lost-device recovery guidance.
- Add per-app permission prompts before sensitive secret or network access.
