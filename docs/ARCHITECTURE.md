# Architecture

## Overview

Cardputer Launcher SDK is a layered Arduino/PlatformIO firmware project. The key architectural choice is to keep v0.1 predictable: apps are registered at compile time, config is loaded from SD-card JSON, and the launcher owns the main loop.

## Layers

```text
main.cpp
  launcher/
    App, AppContext, AppRegistry, Launcher
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
6. Render the current screen and transient status.

## App Boundary

Apps receive an `AppContext` with display, config, logs, Wi-Fi, and HTTP helpers. Apps do not directly own the hardware main loop. This makes the app model clear even before a stable public SDK exists.

## Storage Boundary

`ConfigLoader` is the only place that knows the current SD-card config paths. `LogStore` is the only place that writes log records and applies log redaction.

## Network Boundary

`WifiManager` owns connection attempts. `HttpClient` owns HTTP request execution. The Webhook Launcher describes what to send but does not manage socket details.

## Why Static Registration

Static registration is boring in the right way. It avoids binary loading, ABI compatibility, permission prompts, heap surprises, and malicious SD-card app code. v1.0 can add manifests without pretending v0.1 has a sandbox.

