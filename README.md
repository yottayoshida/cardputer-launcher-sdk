# Cardputer Launcher SDK

A keyboard-first app framework and launcher for building tiny apps on tiny computers, starting with M5Stack Cardputer ADV.

Cardputer Launcher SDK is a small PlatformIO firmware project plus documentation, SD-card examples, and validation tooling. The v0.1.0 showcase app is a config-driven Webhook Launcher: put JSON command definitions on the SD card, choose a command from the Cardputer keyboard, confirm risky actions, send HTTP GET or POST requests, and keep a local request log.

## Project Status

Version: `v0.1.0`

Status: early OSS release. The v0.1.0 code is intentionally small and conservative. It is meant to be readable, flashable, and useful as a starting point, not a complete dynamic app marketplace.

## What It Does

- Boots into a tiny launcher menu.
- Registers built-in apps at compile time.
- Provides a keyboard-first menu abstraction.
- Loads Wi-Fi settings from SD card JSON.
- Loads Webhook Launcher commands from SD card JSON.
- Sends static HTTP GET or POST webhook requests.
- Shows status messages for missing config, connection failures, request failures, and successful requests.
- Writes redacted request logs to SD card when storage is available.
- Includes host-side sample config validation.

## Why It Exists

The Cardputer is a charming form factor: a pocket keyboard, screen, Wi-Fi radio, SD card, and programmable microcontroller. It is ideal for quick command workflows, physical approvals, small dashboards, and local utilities. This project explores a tiny Raycast/Alfred-like appkit for that hardware, where the keyboard is the primary interface and configuration can live on removable storage.

## What Is Not Included Yet

- Dynamic plugin loading.
- A stable public C++ SDK API.
- Verified TLS certificate handling.
- Secret storage beyond redaction and placeholder examples.
- Full text-input templating for webhooks.
- Hardware-verified Cardputer ADV pin mapping.
- OTA updates, sync, or app marketplace behavior.

## Hardware Target

Primary target: M5Stack Cardputer ADV.

The initial `platformio.ini` uses `esp32-s3-devkitc-1` plus the M5Cardputer library because local PlatformIO board metadata lookup did not complete during repository setup. Library dependencies are pinned to upstream GitHub repositories because the local PlatformIO registry client returned `HTTPClientError` while resolving packages. See [docs/ASSUMPTIONS.md](docs/ASSUMPTIONS.md).

## Quick Start

1. Install PlatformIO Core.
2. Copy the `sdcard/` directory contents to the root of a FAT-formatted SD card.
3. Edit `settings.json` with your Wi-Fi SSID and password.
4. Edit `apps/webhook_launcher.json` with your own HTTPS webhook URLs.
5. Build the firmware:

```bash
pio run
```

6. Upload to hardware:

```bash
pio run --target upload
```

## SD-Card Layout

```text
/settings.json
/apps/webhook_launcher.json
/logs/launcher.jsonl
```

Example `settings.json`:

```json
{
  "wifi": {
    "ssid": "YOUR_WIFI_SSID",
    "password": "YOUR_WIFI_PASSWORD"
  }
}
```

Example webhook command:

```json
{
  "name": "Deploy Preview",
  "method": "POST",
  "url": "https://example.com/webhook/deploy-preview",
  "confirm": true,
  "headers": {
    "Content-Type": "application/json",
    "Authorization": "Bearer REPLACE_WITH_TOKEN"
  },
  "body": {
    "source": "cardputer",
    "action": "deploy-preview"
  }
}
```

Do not commit real secrets. The checked-in examples use placeholders only.

## Architecture Summary

The firmware has five small layers:

- `launcher/`: app interface, registry, lifecycle, and launcher loop.
- `ui/`: tiny menu, status bar, toast, and text input primitives.
- `input/`: Cardputer keyboard events normalized to launcher actions.
- `storage/`: SD-card JSON config loading and redacted JSONL logging.
- `network/`: Wi-Fi connection and HTTP request helpers.
- `apps/`: built-in v0.1 apps, including Webhook Launcher, About, and Log Viewer.

Read [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the design boundaries.

## Security Notes

Treat SD-card configuration as sensitive. Webhook URLs, headers, and request bodies may contain credentials. v0.1.0 redacts obvious secret-like strings in logs, but it does not provide encrypted storage or verified TLS certificate pinning. Prefer dedicated low-privilege webhook URLs and rotate secrets if an SD card is lost.

Read [docs/SECURITY.md](docs/SECURITY.md) before using real workflows.

## Validation

Validate sample configs:

```bash
python3 scripts/validate_configs.py
python3 -m unittest discover -s tests
```

Build firmware when PlatformIO dependencies are available:

```bash
pio run
```

## Roadmap

The v1.0 direction is a mini app launcher for Cardputer ADV with app lifecycle, registry, keyboard-first UI, config-driven apps, notes, IR remote, sensor viewer, QR handoff, offline-first logs, optional Wi-Fi sync, AI workflow command decks, and a stronger device abstraction layer.

See [docs/ROADMAP.md](docs/ROADMAP.md) and the [GitHub issues](https://github.com/yottayoshida/cardputer-launcher-sdk/issues) for the backlog.

## License

Licensed under either Apache-2.0 or MIT, at your option. See [LICENSE-APACHE](LICENSE-APACHE) and [LICENSE-MIT](LICENSE-MIT).
