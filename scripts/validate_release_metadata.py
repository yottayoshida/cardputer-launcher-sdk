#!/usr/bin/env python3
"""Validate release tag metadata before publishing."""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path


TAG_RE = re.compile(r"^v(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$")
CHANGELOG_DATE_RE = r"\d{4}-\d{2}-\d{2}"
EM_DASH = "\u2014"


class ReleaseMetadataError(ValueError):
    """Raised when release metadata is missing or inconsistent."""


def _version_from_tag(tag: str) -> str:
    match = TAG_RE.match(tag)
    if not match:
        raise ReleaseMetadataError("release tag must match vMAJOR.MINOR.PATCH")
    return ".".join(match.groups())


def validate_release_metadata(root: Path | str, tag: str) -> None:
    root = Path(root)
    version = _version_from_tag(tag)

    changelog = root / "CHANGELOG.md"
    if not changelog.exists():
        raise ReleaseMetadataError("CHANGELOG.md is missing")

    changelog_text = changelog.read_text(encoding="utf-8")
    changelog_re = re.compile(
        rf"^## \[{re.escape(version)}\] - {CHANGELOG_DATE_RE}\s*$", re.MULTILINE
    )
    if not changelog_re.search(changelog_text):
        raise ReleaseMetadataError(
            f"CHANGELOG.md must contain a dated section for [{version}]"
        )

    notes = root / "docs" / f"RELEASE_NOTES_{tag}.md"
    if not notes.exists():
        raise ReleaseMetadataError(f"{notes.relative_to(root)} is missing")

    note_lines = notes.read_text(encoding="utf-8").splitlines()
    first_line = note_lines[0].strip() if note_lines else ""
    expected_prefix = f"# {tag} {EM_DASH} "
    if not first_line.startswith(expected_prefix):
        raise ReleaseMetadataError(
            f"{notes.relative_to(root)} title must begin with '# {tag} <em dash> '"
        )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "tag",
        nargs="?",
        default=os.environ.get("GITHUB_REF_NAME"),
        help="Release tag to validate, for example v0.1.0.",
    )
    parser.add_argument(
        "--root",
        default=Path(__file__).resolve().parents[1],
        type=Path,
        help="Repository root. Defaults to this script's repository.",
    )
    args = parser.parse_args(argv)

    if not args.tag:
        print("release metadata validation failed: missing release tag", file=sys.stderr)
        return 1

    try:
        validate_release_metadata(args.root, args.tag)
    except ReleaseMetadataError as exc:
        print(f"release metadata validation failed: {exc}", file=sys.stderr)
        return 1

    print(f"validated release metadata for {args.tag}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
