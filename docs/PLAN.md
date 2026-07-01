# v0.1.0 Plan

## 1. Architecture Analysis

### Runtime Architecture

The runtime is a single Arduino firmware with a launcher loop. Hardware setup happens in `main.cpp`, then input, launcher update, and render are called repeatedly. Apps do not own the main loop.

### App Lifecycle

Apps implement a tiny interface: `id`, `name`, `onStart`, `onStop`, `onInput`, and `render`. v0.1 keeps lifecycle synchronous to avoid hidden scheduling and memory complexity.

### App Registry

The registry stores compile-time app pointers. This avoids dynamic allocation surprises and makes v0.1 easy to reason about. Registration happens during setup.

### Device Abstraction

The first abstraction is intentionally thin: display output is represented by `DisplayPort`, keyboard events by `InputEvent`, storage by `ConfigLoader` and `LogStore`, and network by `WifiManager` and `HttpClient`.

### UI Layer

The UI layer includes `Menu`, `TextInput`, `Toast`, and `StatusBar`. The goal is consistent behavior, not a graphical widget toolkit.

### Input Layer

Keyboard input is normalized to `up`, `down`, `select`, `back`, and character events. v0.1 maps common keys such as `w`, `s`, `j`, `k`, Enter, and delete-style back.

### Storage/Config Layer

SD-card JSON files are read with ArduinoJson. Invalid or missing files return explicit errors. Examples are validated by a host-side Python script.

### Network Layer

Wi-Fi connection is explicit and timeout-bound. HTTP requests support static GET and POST commands. Responses return a status code and short preview.

### Example App Layer

Webhook Launcher is the showcase app. It loads commands from SD-card JSON, shows a menu, confirms risky commands, connects Wi-Fi if needed, sends the request, displays the result, and logs the attempt.

### Why Not Dynamic Plugin Loading in v0.1

Dynamic plugin loading would require a packaging format, ABI boundary, permissions, memory strategy, and failure recovery model. That is not needed to prove the product.

### Why Config-Driven Behavior Is Enough for v0.1

Webhook definitions demonstrate the core value: a user can change behavior without recompiling. It is small enough to build, test, and document honestly.

## 2. UX Analysis

### Keyboard-First Navigation

Navigation must work with a thumb-friendly subset: up/down/select/back plus typed characters later. v0.1 prioritizes predictable selection over search.

### Small-Screen Constraints

Screens use short labels, one action per line, and status messages that fit. The UI avoids dense paragraphs.

### Text Input Constraints

Text input exists as a primitive but webhook templated prompts are not part of v0.1. This prevents a half-finished form system.

### Error States

Every external dependency has a visible error state: no SD card, config parse failure, Wi-Fi missing, Wi-Fi failure, HTTP failure, and successful request.

### Status Bar

The status bar shows app context and lightweight state such as SD availability or command count.

### Confirmation Flows

Commands marked `confirm: true` require explicit confirmation before network activity.

### Logs

Logs are append-only JSONL records on SD card. v0.1 displays a small summary and points users to the file for detail.

### QR Handoff Future Path

QR handoff belongs after the device can generate safe continuation URLs and distinguish local display from external continuation.

## 3. Security Analysis

### Webhook Secrets

Secrets may appear in URLs, headers, or bodies. Examples use placeholders. Logs redact obvious secret-like keys and authorization values.

### SD-Card Config Risks

Anyone with the SD card can read and modify config. Users must treat the card as sensitive.

### Network Error Handling

Network calls return explicit error strings. The firmware does not retry indefinitely.

### HTTP vs HTTPS

Examples prefer HTTPS. v0.1 does not implement a complete CA/pinning strategy, so users should keep webhook privileges low.

### Redaction

Redaction targets common strings: authorization, token, secret, key, password, and bearer-style values. It is defense in depth, not a guarantee.

### Logging Risks

Logs can leak metadata. Request bodies and headers should be kept minimal.

### Safe Defaults

No real endpoints, passwords, or tokens are checked in. Confirmation defaults can be set per command.

### v0.1 Limitations

No encrypted secret storage, no permission model, no dynamic app sandbox, no certificate pinning.

### Future Secret Storage Strategy

Future versions should support first-run provisioning, encrypted storage where hardware permits, external secret references, and per-command permission prompts.

## 4. Quality Analysis

### Test Strategy

Host-side tests validate sample JSON and redaction logic. PlatformIO build is attempted when dependencies are available.

### Host-Buildable Core Logic

v0.1 keeps config schema validation in Python for host testing. A later release should extract C++ core parsing into host-buildable units.

### Formatting

No local `clang-format` was found. Code style is kept simple and consistent manually.

### Static Review

Adversarial review checks memory, parsing, logging, security claims, docs, and release readiness.

### Example Config Validation

`scripts/validate_configs.py` validates `sdcard/settings.json` and the Webhook Launcher app pack at `sdcard/apps/webhook_launcher/manifest.json` plus `sdcard/apps/webhook_launcher/commands.json`.

### Manual Hardware Test Plan

1. Flash firmware.
2. Boot without SD card and confirm a clear error.
3. Boot with sample SD card and placeholder Wi-Fi, confirm Wi-Fi failure is clear.
4. Use real low-privilege Wi-Fi and webhook, run a confirmed command.
5. Inspect `/logs/launcher.jsonl` for redaction and status.

### Release Checklist

- Docs complete.
- Licenses present.
- Sample configs validate.
- Build attempted.
- Adversarial plan and code reviews complete.
- Commit and tag created.
- Push and release attempted or blocker reported.

## 5. v0.1 Scope

Included:

- Launcher shell.
- App registry.
- Basic app lifecycle.
- Keyboard navigation abstraction.
- Tiny UI components: menu, text input, toast/status message, and simple confirm flow.
- SD-card sample config.
- Webhook Launcher app.
- Wi-Fi config loading.
- HTTP GET/POST helper.
- Request/response logging.
- README getting started.
- Design docs.
- Security notes.
- Changelog.

Excluded:

- Dynamic app loading.
- Rich command search.
- Webhook input templating.
- Secret vault.
- QR handoff.
- IR remote and sensor viewer implementations.

## 6. v1.0 Issue Backlog

Milestones:

- v0.1.0: initial release.
- v0.2.0: config schema and hardware validation.
- v0.3.0: richer app lifecycle and first additional app prototypes.
- v0.5.0: handoff, safer secrets, and sync experiments.
- v1.0.0: stable appkit and polished release.

The actionable backlog is tracked in GitHub issues under `yottayoshida/cardputer-launcher-sdk`.
