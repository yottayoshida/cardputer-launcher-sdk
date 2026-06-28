# Cardputer Launcher SDK Design Spec

This spec records the autonomous design approval for the v0.1.0 release goal. The user explicitly requested autonomous execution without stopping for confirmation unless blocked, so this spec is approved by the brief and the adversarial self-review in `docs/ADVERSARIAL_REVIEW.md`.

## Goal

Build and release v0.1.0 of Cardputer Launcher SDK: a small PlatformIO-based keyboard-first app framework and launcher for M5Stack Cardputer ADV.

## Design Decision

Use static built-in apps, SD-card JSON configuration, a Webhook Launcher showcase app, and explicit docs for the richer v1.0 vision. Avoid dynamic loading and unstable framework promises in v0.1.

## Acceptance

The design is accepted when docs, sample configs, firmware skeleton, validation, adversarial review, commit, tag, and best-effort GitHub release path are complete.

