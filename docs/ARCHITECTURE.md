# Architecture

## Overview

Cardputer Launcher SDK is a layered Arduino/PlatformIO firmware project. The key architectural choice is to keep v0.1 predictable: apps are registered at compile time, config is loaded from SD-card JSON, and the launcher owns the main loop.

## Layers

```text
main.cpp
  launcher/
    App, AppContext, AppManifest, AppRegistry, Launcher
  apps/
    WebhookLauncherApp, AboutApp, LogViewerApp
  ui/
    Menu, TextInput, Toast, StatusBar
  input/
    Keyboard
  storage/
    ConfigLoader, LogStore
  network/
    WifiManager, HttpClient
```

## Runtime Flow

1. Initialize serial, display, keyboard, SD card, config loader, log store, Wi-Fi manager, and HTTP client.
2. Register built-in apps.
3. Render the launcher menu.
4. Read keyboard events in the Arduino loop.
5. Route events to the launcher or active app.
6. Tick the active app when one is foregrounded.
7. Render the current screen and transient status.

## App Boundary

Apps receive an `AppContext` with display, config, logs, Wi-Fi, and HTTP helpers. Apps do not directly own the hardware main loop. This makes the app model clear even before a stable public SDK exists.

Apps also expose an `AppManifest` metadata value. The current manifest is a firmware-side draft, not a package installer. It records:

- `id`: stable machine-readable app identifier.
- `name`: short display name.
- `version`: app version.
- `category`: broad app category.
- `configPath`: optional SD-card config path.
- `permissions`: bitmask of requested access areas.
- `capabilities`: bitmask of app-provided behavior.

The launcher still registers C++ app objects at compile time. Manifest metadata helps document and inspect app intent without loading arbitrary code from removable storage.

## App Lifecycle

The v0.3 lifecycle foundation remains foreground-only:

```text
launcher menu
  select app
    onStart()
    onFocus()
    loop: onInput() for events, onTick() once per loop, render()
  back
    onBlur()
    onStop()
launcher menu
```

`onTick()` marks the foreground app dirty so polling-based UI updates can redraw. `onSuspend()` and `onResume()` exist as reserved no-op hooks for the later v1.0 state machine. The current launcher does not background apps, so it does not call suspend or resume yet.

## Storage Boundary

`ConfigLoader` is the only place that knows the current SD-card config paths. `LogStore` is the only place that writes log records and applies log redaction.

## Network Boundary

`WifiManager` owns connection attempts. `HttpClient` owns HTTP request execution. The Webhook Launcher describes what to send but does not manage socket details.

## Why Static Registration

Static registration is boring in the right way. It avoids binary loading, ABI compatibility, permission prompts, heap surprises, and malicious SD-card app code. v1.0 can add manifests without pretending v0.1 has a sandbox. The firmware does not load binary apps from SD card unless a separate sandbox design exists.
