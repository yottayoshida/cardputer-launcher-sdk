# Issue 10 Secret Provisioning Design

## Goal

Design safer v0.5 secret provisioning and add a minimal prototype that proves a webhook header can use a secret reference without putting the raw secret in the command pack.

## Scope

- Add a documented `secretRef` prototype for webhook header values.
- Resolve prototype refs from an untracked `/secrets.json` SD-card file.
- Keep checked-in command packs and examples free of real secrets.
- Add host-side validation and redaction helpers for the prototype syntax.
- Add an opt-in sync settings contract that defaults to disabled.
- Document the threat model for lost SD cards, malicious command packs, and network observers.

## Non-Goals

- No encrypted hardware-backed vault claim in this PR.
- No QR or browser provisioning flow implementation.
- No background sync executor.
- No URL/body/settings secret references beyond the documented prototype boundary.
- No claim that `/secrets.json` protects against a lost SD card.

## Design

`sdcard/apps/webhook_launcher.json` remains the shareable app command pack. A command header value may be either a string or an object with exactly one `secretRef` field:

```json
{
  "Authorization": {
    "secretRef": "webhooks.deploy.authorization"
  }
}
```

The prototype firmware resolver reads `/secrets.json` as a flat string map:

```json
{
  "webhooks.deploy.authorization": "Bearer REPLACE_WITH_REAL_TOKEN"
}
```

`sdcard/secrets.json` is ignored by git. `sdcard/secrets.example.json` documents the shape without real values.

Normal request logs continue to record command name, method, URL, status, outcome, and response preview. They do not record headers, so a header secret resolved through this prototype is not exposed in normal logs.

`settings.sync.enabled` is optional and defaults to `false`. If sync is enabled, validation requires an HTTPS endpoint. This PR documents the control plane contract but does not add a sync network client.

## Acceptance Mapping

- `docs/SECURITY.md` and `docs/SECRET_PROVISIONING.md` contain the concrete design.
- `SecretStore` plus `ConfigLoader::loadWebhooks(..., SecretStore*)` resolves a header secret reference.
- Host tests cover valid and invalid secret references, known-secret redaction, and disabled-by-default sync.
- Threat model docs cover lost SD card, malicious command pack, and network observer scenarios.
