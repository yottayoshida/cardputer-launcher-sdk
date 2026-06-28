# Cardputer Launcher SDK Design

## Product Thesis

Cardputer Launcher SDK turns a tiny keyboard-first ESP32-S3 computer into a practical workflow surface. The thesis is simple: many automations do not need a phone, laptop, or dashboard. They need a small trusted device with a physical keyboard, a screen, local logs, and a confirm button.

The project should feel like a tiny Raycast or Alfred for a physical IoT device. Users should be able to open a launcher, search or select a command, review what will happen, and trigger a workflow with confidence.

## Why Cardputer ADV Is Interesting

Cardputer ADV combines a keyboard, display, Wi-Fi, microcontroller, battery-friendly form factor, and removable storage. That makes it more expressive than a button box and more focused than a phone. It can become a field tool, desk command deck, incident-response controller, classroom device, or personal automation remote.

The SD card matters: configuration can be copied, edited, backed up, audited, and swapped without recompiling firmware.

## Target Users

- Embedded developers who want a practical Cardputer app starting point.
- Automation builders who want a physical approval or trigger device.
- Makers who want small utility apps without writing a whole UI framework.
- Future contributors who want clean boundaries for apps, UI, storage, and network helpers.
- Security-conscious users who need clear limitations before placing real secrets on removable media.

## Core Use Cases

- Trigger a webhook after a clear confirmation flow.
- Keep a local history of workflow attempts.
- Carry a small set of operational commands in a pocket device.
- Build new Cardputer apps using the app interface and UI primitives.
- Share config-driven command packs through SD-card files.

## Non-Goals

- Dynamic binary plugin loading in v0.1.
- A stable marketplace or package manager in v0.1.
- A secret vault in v0.1.
- Full offline sync in v0.1.
- Replacing a phone, laptop, or full incident console.

## v1.0 User Experience

On boot, the user sees a compact command palette:

```text
CL SDK        WiFi:ok
> Webhooks
  Notes
  IR Remote
  Sensors
  Logs
```

Typing filters apps and commands. Arrow-like key bindings move selection. Enter opens or triggers. Backspace/Escape returns. Status and errors stay short, readable, and recoverable.

Webhook commands show risk before action:

```text
Deploy Preview
POST example.com

Run command?
[Y] yes   [N] no
```

Logs are offline-first:

```text
Last Runs
200 Deploy Preview
503 Build Kickoff
ERR WiFi missing
```

## Example Flows

1. A maker copies `sdcard/` examples to a card, edits Wi-Fi and webhook URLs, flashes firmware, and triggers a test workflow.
2. A team creates command packs for safe low-privilege webhooks, then carries a Cardputer as a human-in-the-loop approval device.
3. A developer adds a compile-time app by implementing `App`, registering it in `main.cpp`, and reusing `Menu`, `Toast`, `StatusBar`, and storage helpers.
4. A future v1.0 user scans a QR code from the Cardputer to continue a longer workflow on a phone or browser.

## Example Screen Sketches

Launcher:

```text
Launcher      SD:ok
> Webhook Launcher
  Log Viewer
  Settings/About
```

Webhook list:

```text
Webhooks      3 cmd
> Deploy Preview
  Start Backup
  Ping Healthcheck
```

Failure state:

```text
Webhook
Wi-Fi config missing

Edit /settings.json
```

## App Model

An app is a small C++ object with an id, display name, lifecycle hooks, input handling, and render method. The launcher owns navigation and calls the active app. v0.1 uses compile-time registration so memory, startup behavior, and build output remain predictable.

## App Manifest Model

The v1.0 direction is a manifest that describes app id, title, version, permissions, config schema, command entries, and assets. v0.1 does not load binary apps from manifests. It uses JSON config files for built-in apps only.

## Config-Driven App Model

The Webhook Launcher proves the config-driven model. Commands are declared in `apps/webhook_launcher.json` with:

- `name`
- `method`
- `url`
- optional `headers`
- optional JSON `body`
- optional `confirm`

This is enough to make the device useful without promising a dynamic runtime.

## Built-In Apps

v0.1:

- Webhook Launcher
- Settings/About
- Log Viewer

v1.0 candidates:

- Notes
- IR Remote
- Sensor Viewer
- QR Handoff
- AI Workflow Command Deck

## Developer Experience

Developers should understand the repo without hardware. Documentation explains architecture, security, UX, and roadmap. Sample configs validate on a normal computer. Firmware code is split by responsibility and uses small interfaces rather than a monolithic sketch.

## Hardware Capability Mapping

- Keyboard: primary launcher navigation and text entry.
- Display: compact menu, status, confirmation, and logs.
- SD card: settings, app config, and local logs.
- Wi-Fi: webhook execution and optional future sync.
- IR and sensors: future built-in app families.

## Offline-First Model

The device should remain useful when offline. v0.1 shows config and logs locally, reports Wi-Fi errors clearly, and records request attempts when possible. v1.0 should add durable queues and optional sync.

## AI Workflow Integration Model

The Cardputer can become a human approval surface for AI workflows. A command might approve a queued automation, pause a deployment, request a summary, or send a structured signal to a server. The device should make high-risk commands explicit and preserve a local trail.

## Security and Safety Posture

The project favors honest limitations over false comfort. v0.1 examples use placeholders, redaction protects common log leaks, confirmation is available per command, and docs warn that SD-card config is sensitive. Future work should add permissions, config schemas, secret storage, certificate strategy, and safer provisioning flows.

## Roadmap From v0.1 to v1.0

- v0.1.0: coherent launcher shell, built-in Webhook Launcher, docs, sample configs, redacted logs.
- v0.2.0: stronger config schema, host-testable core logic, better log viewer, first hardware test guide.
- v0.3.0: richer app lifecycle, Notes and IR Remote prototypes, input improvements.
- v0.5.0: QR handoff, sensor viewer, safer secret handling, optional sync experiment.
- v1.0.0: polished appkit, stable APIs, app manifests, permission model, examples, and a security-reviewed release process.

