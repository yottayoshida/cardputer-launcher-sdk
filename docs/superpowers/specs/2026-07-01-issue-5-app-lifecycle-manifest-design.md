# Issue 5 App Lifecycle and Manifest Draft Design

## Goal

Design and implement the first v0.3 appkit foundation for Cardputer Launcher SDK: a documented app manifest draft, lightweight manifest metadata in firmware, and optional lifecycle hooks that do not force existing v0.1 apps into a complex state machine.

## Context

The v0.1 app boundary is intentionally small. `App` exposes `id()`, `name()`, `onStart()`, `onStop()`, `onInput()`, and `render()`. `Launcher` stores a single active app pointer and calls `onStart()` when an app opens and `onStop()` when Back returns to the launcher.

Issue #5 asks for the next foundation:

- Draft an app manifest model with id, name, version, config paths, permissions, and capabilities.
- Add lifecycle hooks for focus, blur, tick, suspend, and resume only where justified.
- Keep binary dynamic loading out of scope unless a separate design proves it is safe.
- Update `docs/ARCHITECTURE.md` and `docs/DESIGN.md`.

## Chosen Approach

Use a conservative staged appkit design.

Firmware gets a small `AppManifest` value type with string fields and bitmask fields for permissions and capabilities. `App` gains a default `manifest()` method plus no-op lifecycle hooks. Built-in apps can override `manifest()` where metadata is useful, while apps that only need `id()` and `name()` continue to compile with little extra code.

`Launcher` will call:

- `onStart()` and `onFocus()` when a user opens an app.
- `onTick()` once per loop while an app is active.
- `onBlur()` and `onStop()` when Back closes the active app.

`onSuspend()` and `onResume()` are added as no-op interface hooks but not actively used by the launcher in this issue. Background app suspension needs a state machine and failure policy, which belongs to #15.

## Alternatives Considered

### Docs-only manifest draft

This would be low risk, but it would not prove that the manifest vocabulary fits the firmware app boundary. It would also leave #14 with no source-level foothold.

### Full lifecycle state machine now

This would solve more of the v1.0 direction, but it would cross into #15 and #11. It would also make this PR larger and risk destabilizing the simple v0.1 launcher.

### Dynamic SD-card app loading

This is explicitly out of scope. Loading arbitrary binaries from SD card requires a sandbox, ABI contract, permissions, memory policy, failure recovery, signing, and a security review.

## Manifest Draft

The v0.3 draft manifest fields are:

- `id`: stable machine-readable identifier, lower-case snake case.
- `name`: short display name for launcher menus.
- `version`: app version string.
- `category`: broad app category such as `automation`, `system`, `notes`, `hardware`, or `diagnostics`.
- `configPath`: optional SD-card config path owned by the app.
- `permissions`: bitmask of requested access areas.
- `capabilities`: bitmask of app-provided behavior.

Permission names:

- `storage.read`
- `storage.write`
- `network.http`
- `input.keyboard`
- `display.draw`
- `ir.transmit`
- `sensor.read`

Capability names:

- `command.list`
- `command.execute`
- `config.reload`
- `log.view`
- `notes.edit`
- `ir.send`
- `sensor.view`

The C++ draft should use enum-like `uint32_t` flags so it stays cheap on embedded targets and easy to test with source-level checks.

## Example Manifests

Webhook Launcher:

```json
{
  "id": "webhook_launcher",
  "name": "Webhook Launcher",
  "version": "0.1.0",
  "category": "automation",
  "configPath": "/apps/webhook_launcher.json",
  "permissions": ["storage.read", "storage.write", "network.http", "input.keyboard", "display.draw"],
  "capabilities": ["command.list", "command.execute", "config.reload"]
}
```

Log Viewer:

```json
{
  "id": "log_viewer",
  "name": "Log Viewer",
  "version": "0.1.0",
  "category": "diagnostics",
  "configPath": "",
  "permissions": ["storage.read", "input.keyboard", "display.draw"],
  "capabilities": ["log.view"]
}
```

## Lifecycle Draft

The lightweight lifecycle is:

```text
launcher menu
  select app
    onStart()
    onFocus()
    loop: onInput() for events, onTick() every loop, render() when dirty
  back
    onBlur()
    onStop()
launcher menu
```

Rules:

- `onStart()` loads app state and config needed for a foreground session.
- `onFocus()` is for short foreground-entry UI updates and should not allocate large state.
- `onTick()` is optional and must be fast; long network or storage work remains explicit app behavior.
- `onBlur()` cancels transient prompts or input state.
- `onStop()` releases foreground session state.
- `onSuspend()` and `onResume()` are reserved for the later multi-state lifecycle in #15.

## Documentation Updates

`docs/ARCHITECTURE.md` will explain the firmware boundary, manifest metadata, and the lightweight lifecycle call sequence.

`docs/DESIGN.md` will explain the v1.0 manifest direction, permission vocabulary, example manifests, and the dynamic loading deferral.

`docs/ROADMAP.md` can stay unchanged unless the implementation reveals a roadmap inconsistency.

## Testing

Use source-level host tests because this repo does not yet have host-buildable C++ appkit tests. The tests should verify:

- `AppManifest` has all required fields.
- Permission and capability constants cover the required vocabulary.
- `App` includes default manifest and lifecycle hooks.
- `Launcher` calls focus, blur, and tick hooks.
- Built-in app manifests include at least Webhook Launcher and Log Viewer metadata.
- Docs contain two manifest examples, permission names, lifecycle terms, and the dynamic loading deferral.

Final verification should include:

- `git diff --check`
- `python3 -m unittest discover -s tests`
- `python3 scripts/validate_configs.py`
- `pio run`
- `codex review --uncommitted`

## Non-Goals

- No dynamic binary loading from SD card.
- No stable v1.0 manifest schema enforcement; that belongs to #14.
- No full lifecycle state machine, disabled-app policy, or failure isolation; that belongs to #15.
- No permission prompts; those belong to #28.
- No broad refactor of app storage, network, or UI layers.

## Spec Self-Review

- Placeholder scan: no placeholders remain.
- Internal consistency: the design keeps firmware changes small and matches the documented non-goals.
- Scope check: this is one coherent PR that unlocks #14 and #15 without implementing them.
- Ambiguity check: suspend/resume are explicitly interface reservations in this issue, not active launcher behavior.
