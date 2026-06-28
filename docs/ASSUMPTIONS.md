# Assumptions

Date: 2026-06-28

## Tooling Observed

- `git`: available, version 2.53.0.
- `gh`: available, version 2.95.0.
- `python3`: available, version 3.14.6.
- `platformio` / `pio`: available, PlatformIO Core 6.1.19.
- `clang-format`: not found locally.
- The PlatformIO registry client returned `HTTPClientError` while resolving registry packages, so `platformio.ini` uses official GitHub URLs for M5GFX, M5Unified, IRremote, M5Cardputer, and ArduinoJson.

## Hardware and Build Assumptions

- Primary hardware is M5Stack Cardputer ADV.
- The local PlatformIO board registry lookup for `m5stack` did not complete during setup, so `platformio.ini` uses `esp32-s3-devkitc-1` plus the M5Cardputer library as the initial build target.
- The SD-card mount is attempted with the default Arduino SD initialization path in v0.1.0. Hardware-specific pin corrections may be needed after first device testing.
- The code uses static app registration for v0.1.0. Dynamic plugin loading is intentionally out of scope.

## Security Assumptions

- SD-card configuration is user-controlled and may contain secrets.
- Example URLs and credentials are placeholders only.
- v0.1.0 redacts obvious secret-like values from logs, but it does not provide encrypted storage.
- v0.1.0 does not implement a full certificate trust store or pinning strategy for HTTPS requests.

## Release Assumptions

- GitHub repository creation, issue creation, push, tag push, and release creation depend on `gh` authentication and network availability.
- The public GitHub repository is `https://github.com/yottayoshida/cardputer-launcher-sdk`.
