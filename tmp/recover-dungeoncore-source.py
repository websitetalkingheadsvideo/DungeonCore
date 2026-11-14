"""Recover latest DungeonCore\\Source writes from agent transcripts."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(r"E:\Dungeon Core")
TRANSCRIPTS = Path(r"C:\Users\paris\.cursor\projects\e-Dungeon-Core\agent-transcripts")
OUT_MANIFEST = ROOT / "tmp" / "recovered-source-manifest.txt"


def normalize_path(path: str) -> Path | None:
    path = path.replace("/", "\\")
    lower = path.lower()
    if "dungeoncore\\source" not in lower:
        return None
    if not lower.startswith(str(ROOT).lower()):
        if lower.startswith("dungeoncore\\"):
            return ROOT / path
        if lower.startswith("e:\\dungeon core\\"):
            return Path(path)
        return None
    return Path(path)


def iter_writes() -> dict[str, tuple[Path, str, str]]:
    latest: dict[str, tuple[Path, str, str]] = {}
    for jsonl in TRANSCRIPTS.rglob("*.jsonl"):
        for line in open(jsonl, encoding="utf-8", errors="ignore"):
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                continue
            content = obj.get("message", {}).get("content", [])
            if not isinstance(content, list):
                continue
            for part in content:
                if not isinstance(part, dict) or part.get("type") != "tool_use":
                    continue
                inp = part.get("input")
                if not isinstance(inp, dict):
                    continue
                path = inp.get("path")
                contents = inp.get("contents")
                if not isinstance(path, str) or not isinstance(contents, str):
                    continue
                resolved = normalize_path(path)
                if resolved is None:
                    continue
                if not resolved.suffix.lower() in {".cpp", ".h", ".cs", ".json", ".ini"}:
                    continue
                key = str(resolved).lower()
                latest[key] = (resolved, contents, jsonl.name)
    return latest


def main() -> None:
    files = iter_writes()
    manifest_lines = [f"{len(files)} files"]
    for key in sorted(files):
        path, contents, src = files[key]
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents, encoding="utf-8")
        manifest_lines.append(f"{len(contents):6d}  {path}  ({src})")
    OUT_MANIFEST.write_text("\n".join(manifest_lines), encoding="utf-8")
    print("\n".join(manifest_lines))


if __name__ == "__main__":
    main()
