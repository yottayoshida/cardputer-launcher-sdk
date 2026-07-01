# Issue #17 SD-Card Layout and Recovery Design

## Goal

Define and enforce a v1.0 SD-card tree that users can validate, back up, migrate, and repair without guessing which files are owned by firmware, apps, or runtime logs.

## Scope

This issue adds a layout contract and recovery foundation. It does not implement the final v1.0 app manifest schema from #14, the full offline audit subsystem from #21, or secret vault behavior from #10/#20.

## Layout

The v1.0 sample SD-card tree uses these paths:

```text
/settings.json
/apps/webhook_launcher/manifest.json
/apps/webhook_launcher/commands.json
/logs/launcher.jsonl
/cache/
/backups/
```

`/apps/webhook_launcher.json` remains a legacy compatibility path for existing v0.1 cards. New sample cards use the app-pack directory form.

## File Ownership

- `settings.json` is user-owned and contains Wi-Fi settings plus a schema `version`.
- `apps/<app_id>/manifest.json` is user-owned metadata for a config-driven app pack.
- `apps/<app_id>/commands.json` is user-owned app config. For Webhook Launcher it keeps the existing `version` and `commands` shape.
- `logs/` is runtime-owned and append-only by default.
- `cache/` is runtime-owned and disposable.
- `backups/` is user/runtime-owned and stores copies made before manual edits or future migrations.

## Validation

The host validator checks the whole tree, not only individual JSON files. It reports the exact file and field for malformed app packs, detects partial-write residues such as `*.tmp`, validates schema versions, and keeps the existing HTTPS/default safety checks for webhook commands.

## Firmware Recovery

Firmware creates missing non-secret directories when SD is available:

- `/apps`
- `/apps/webhook_launcher`
- `/logs`
- `/cache`
- `/backups`

Firmware does not create `settings.json`, command packs, or secret-bearing files. Malformed JSON remains a clear load error rather than an automatic rewrite. Webhook loading prefers the v1.0 app-pack command path and falls back to the legacy v0.1 path.

## Migration Rules

- User-owned JSON files carry explicit schema versions.
- Unknown major versions fail validation with a per-file error.
- Legacy `/apps/webhook_launcher.json` remains readable for v0.1 cards during the transition.
- Future migrations must write to a temporary file, verify the replacement, then leave a dated backup under `/backups`.

## Repair Workflow

Users should validate the card on a host, copy the existing card to a backup directory, fix the reported file and field, remove stale `*.tmp` files after checking their contents, then re-run validation before booting the device.

## Acceptance Mapping

- Fresh `sdcard/` validates with `python3 scripts/validate_configs.py`.
- Malformed app pack files report paths such as `apps/webhook_launcher/manifest.json` and fields such as `app manifest.id`.
- Firmware creates missing non-secret directories through a storage layout helper.
- README and design docs describe backup, migration, and repair behavior.
