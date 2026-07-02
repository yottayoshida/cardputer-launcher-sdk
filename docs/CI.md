# Continuous Integration

This repository uses GitHub Actions to keep pull requests and release tags reproducible from a clean runner.

## Jobs

- `host-tests`: runs Python unit tests and validates the sample SD-card config.
- `docs-and-secrets`: checks local Markdown links and runs a high-confidence placeholder-aware secret scan.
- `firmware-build`: installs PlatformIO, restores PlatformIO caches, runs the firmware build, and uploads firmware outputs when they exist.
- `release-metadata`: runs only for `v*` tag refs and checks release metadata.

## Local Reproduction

Run the same commands locally before opening or updating a PR:

```bash
git diff --check
python3 -m unittest discover -s tests
python3 scripts/validate_configs.py
python3 scripts/check_docs_links.py
python3 scripts/scan_secrets.py
python3 scripts/validate_release_metadata.py v0.1.0
pio run
```

If `pio run` fails, rerun with verbose output:

```bash
pio run -v
```

The firmware job uploads `cardputer-adv-firmware` artifacts from `.pio/build/cardputer_adv/` when files are present. The GitHub Actions log remains the primary diagnostic source for compiler and dependency-resolution failures.

## Release Tags

Release tags must use `vMAJOR.MINOR.PATCH`, for example `v0.1.0`.

For each release tag:

- `CHANGELOG.md` must contain `## [MAJOR.MINOR.PATCH] - YYYY-MM-DD`.
- `docs/RELEASE_NOTES_<tag>.md` must exist.
- The release notes title must begin with the tag followed by an em dash and a short title.

Validate a release locally:

```bash
python3 scripts/validate_release_metadata.py v0.1.0
```

## Secret Scan Scope

`scripts/scan_secrets.py` looks for high-confidence private key and token patterns in tracked text files. It intentionally allows documented placeholders such as `REPLACE_WITH_TOKEN`, `YOUR_API_KEY`, and `REDACTED`.

This check is a guardrail, not a complete data-loss-prevention system. Treat SD-card configs and webhook credentials as sensitive even when this scan passes.
