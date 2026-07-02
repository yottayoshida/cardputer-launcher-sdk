#!/usr/bin/env python3
"""Run a practical high-confidence secret scan over repository text files."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


SKIP_DIRS = {".git", ".pio", "__pycache__", ".pytest_cache", ".mypy_cache"}
PLACEHOLDER_MARKERS = (
    "REPLACE_WITH",
    "YOUR_",
    "REDACTED",
    "PLACEHOLDER",
    "EXAMPLE",
    "example.com",
)
SECRET_PATTERNS = (
    ("private key", re.compile(r"-----BEGIN (?:[A-Z0-9]+ )*PRIVATE KEY(?: BLOCK)?-----")),
    ("GitHub token", re.compile(r"\bgh[pousr]_[A-Za-z0-9_]{36,}\b")),
    ("GitHub fine-grained token", re.compile(r"\bgithub_pat_[A-Za-z0-9_]{20,}\b")),
    ("OpenAI API key", re.compile(r"\bsk-(?:proj-)?[A-Za-z0-9_-]{20,}\b")),
    ("AWS access key", re.compile(r"\bAKIA[0-9A-Z]{16}\b")),
)


def _is_placeholder_value(value: str) -> bool:
    upper_value = value.upper()
    return any(marker.upper() in upper_value for marker in PLACEHOLDER_MARKERS)


def _looks_binary(path: Path) -> bool:
    try:
        return b"\0" in path.read_bytes()[:2048]
    except OSError:
        return True


def _tracked_files(root: Path) -> list[Path]:
    try:
        result = subprocess.run(
            ["git", "ls-files"],
            cwd=root,
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return []
    return [root / line for line in result.stdout.splitlines() if line]


def _default_paths(root: Path) -> list[Path]:
    tracked = _tracked_files(root)
    if tracked:
        return tracked

    paths: list[Path] = []
    for child in root.rglob("*"):
        if child.is_file() and not any(part in SKIP_DIRS for part in child.relative_to(root).parts):
            paths.append(child)
    return paths


def scan_paths(paths: list[Path | str]) -> list[str]:
    findings: list[str] = []
    for raw_path in paths:
        path = Path(raw_path)
        if not path.is_file() or _looks_binary(path):
            continue

        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except UnicodeDecodeError:
            continue

        for line_number, line in enumerate(lines, start=1):
            for label, pattern in SECRET_PATTERNS:
                if any(
                    not _is_placeholder_value(match.group(0))
                    for match in pattern.finditer(line)
                ):
                    findings.append(f"{path}:{line_number}: possible {label}")
    return findings


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="Files or directories to scan. Defaults to tracked repository files.",
    )
    parser.add_argument(
        "--root",
        default=Path(__file__).resolve().parents[1],
        type=Path,
        help="Repository root for default tracked-file scanning.",
    )
    args = parser.parse_args(argv)

    paths: list[Path] = []
    if args.paths:
        for path in args.paths:
            if path.is_dir():
                paths.extend(
                    child
                    for child in path.rglob("*")
                    if child.is_file()
                    and not any(part in SKIP_DIRS for part in child.relative_to(path).parts)
                )
            else:
                paths.append(path)
    else:
        paths = _default_paths(args.root)

    findings = scan_paths(paths)
    if findings:
        print("secret scan failed:", file=sys.stderr)
        for finding in findings:
            print(f"- {finding}", file=sys.stderr)
        return 1

    print("validated repository secret scan")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
