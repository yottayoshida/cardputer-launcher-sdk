#!/usr/bin/env python3
"""Check local Markdown links in project documentation."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from urllib.parse import unquote


LINK_RE = re.compile(r"(?<!!)\[[^\]]+\]\(([^)\s]+)(?:\s+\"[^\"]+\")?\)")
HEADING_RE = re.compile(r"^(#{1,6})\s+(.+?)\s*$")
DEFAULT_INCLUDE = ("README.md", "docs", "examples")
EXTERNAL_PREFIXES = ("http://", "https://", "mailto:", "tel:")
SKIP_DIRS = {".git", ".pio", "__pycache__", ".pytest_cache"}


def _markdown_files(root: Path, include: tuple[str, ...] = DEFAULT_INCLUDE) -> list[Path]:
    files: list[Path] = []
    for entry in include:
        path = root / entry
        if path.is_file() and path.suffix.lower() == ".md":
            files.append(path)
        elif path.is_dir():
            files.extend(
                child
                for child in path.rglob("*.md")
                if not any(part in SKIP_DIRS for part in child.relative_to(root).parts)
            )
    return sorted(files)


def _anchor_for(heading: str) -> str:
    text = re.sub(r"`([^`]*)`", r"\1", heading.strip().lower())
    text = re.sub(r"<[^>]+>", "", text)
    text = re.sub(r"[^\w\s-]", "", text)
    text = re.sub(r"\s+", "-", text.strip())
    return text


def _anchors(path: Path) -> set[str]:
    seen: dict[str, int] = {}
    anchors: set[str] = set()
    for line in path.read_text(encoding="utf-8").splitlines():
        match = HEADING_RE.match(line)
        if not match:
            continue
        base = _anchor_for(match.group(2))
        count = seen.get(base, 0)
        seen[base] = count + 1
        anchors.add(base if count == 0 else f"{base}-{count}")
    return anchors


def check_markdown_links(root: Path | str) -> list[str]:
    root = Path(root)
    findings: list[str] = []
    anchor_cache: dict[Path, set[str]] = {}

    for markdown in _markdown_files(root):
        text = markdown.read_text(encoding="utf-8")
        for line_number, line in enumerate(text.splitlines(), start=1):
            for match in LINK_RE.finditer(line):
                target = unquote(match.group(1).strip())
                if not target or target.startswith(EXTERNAL_PREFIXES):
                    continue
                if target.startswith("#"):
                    continue

                target_path, _, fragment = target.partition("#")
                resolved = (markdown.parent / target_path).resolve()
                try:
                    relative_target = resolved.relative_to(root.resolve())
                except ValueError:
                    findings.append(
                        f"{markdown.relative_to(root)}:{line_number}: link leaves repository: {target}"
                    )
                    continue

                if not resolved.exists():
                    findings.append(
                        f"{markdown.relative_to(root)}:{line_number}: missing link target: {target}"
                    )
                    continue

                if fragment and resolved.suffix.lower() == ".md":
                    anchors = anchor_cache.setdefault(resolved, _anchors(resolved))
                    if fragment not in anchors:
                        findings.append(
                            f"{markdown.relative_to(root)}:{line_number}: missing anchor #{fragment} in {relative_target}"
                        )

    return findings


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        default=Path(__file__).resolve().parents[1],
        type=Path,
        help="Repository root. Defaults to this script's repository.",
    )
    args = parser.parse_args(argv)

    findings = check_markdown_links(args.root)
    if findings:
        print("documentation link check failed:", file=sys.stderr)
        for finding in findings:
            print(f"- {finding}", file=sys.stderr)
        return 1

    print("validated local Markdown links")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
