#!/usr/bin/env python3
"""Regenerate the local pack table in library.md from library-manifest.yaml."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

import yaml

TOOLS_DIR = Path(__file__).resolve().parent
REPO_ROOT = TOOLS_DIR.parent.parent
MANIFEST_PATH = TOOLS_DIR / "library-manifest.yaml"
PACK_ROOTS_PATH = TOOLS_DIR / "pack-roots.json"
LIBRARY_PATH = REPO_ROOT / "library.md"

START_MARKER = "<!-- LOCAL_PACK_TABLE_START -->"
END_MARKER = "<!-- LOCAL_PACK_TABLE_END -->"


def escape_table_cell(text: str) -> str:
    return text.replace("|", "\\|").replace("\n", " ").strip()


def build_table_rows(packs: list[dict]) -> str:
    lines = [
        "| Folder | Type | Short description |",
        "| --- | --- | --- |",
    ]
    for pack in sorted(packs, key=lambda p: p["id"].casefold()):
        folder = escape_table_cell(pack["id"])
        typ = escape_table_cell(pack.get("type", "Environment"))
        summary = escape_table_cell(pack.get("summary", ""))
        lines.append(f"| {folder} | {typ} | {summary} |")
    return "\n".join(lines) + "\n"


def main() -> int:
    check_only = "--check" in sys.argv

    if not MANIFEST_PATH.is_file():
        print(f"Missing manifest: {MANIFEST_PATH}", file=sys.stderr)
        return 1

    manifest = yaml.safe_load(MANIFEST_PATH.read_text(encoding="utf-8"))
    packs: list[dict] = manifest.get("packs") or []

    root_ids: set[str] = set()
    if PACK_ROOTS_PATH.is_file():
        roots = json.loads(PACK_ROOTS_PATH.read_text(encoding="utf-8"))
        root_ids = {p["id"] for p in roots.get("packs", [])}

    manifest_ids = {p["id"] for p in packs}
    errors: list[str] = []
    if root_ids:
        missing_manifest = sorted(root_ids - manifest_ids)
        extra_manifest = sorted(manifest_ids - root_ids)
        if missing_manifest:
            errors.append(
                "pack-roots.json ids missing from library-manifest.yaml: "
                + ", ".join(missing_manifest)
            )
        if extra_manifest:
            errors.append(
                "library-manifest.yaml ids not on disk (pack-roots): "
                + ", ".join(extra_manifest)
            )

    for err in errors:
        print(err, file=sys.stderr)

    if check_only:
        return 1 if errors else 0

    if errors:
        print("Fix manifest vs pack-roots before updating library.md (or run Sync-PackRoots.ps1).", file=sys.stderr)
        return 1

    if not LIBRARY_PATH.is_file():
        print(f"Missing {LIBRARY_PATH}", file=sys.stderr)
        return 1

    text = LIBRARY_PATH.read_text(encoding="utf-8")
    if START_MARKER not in text or END_MARKER not in text:
        print(f"library.md must contain {START_MARKER} and {END_MARKER}", file=sys.stderr)
        return 1

    table = build_table_rows(packs)
    replacement = f"{START_MARKER}\n{table}{END_MARKER}"
    pattern = re.compile(
        rf"{re.escape(START_MARKER)}.*?{re.escape(END_MARKER)}",
        re.DOTALL,
    )
    new_text = pattern.sub(replacement, text, count=1)
    LIBRARY_PATH.write_text(new_text, encoding="utf-8", newline="\n")
    print(f"Updated local pack table ({len(packs)} rows) in {LIBRARY_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
