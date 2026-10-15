#!/usr/bin/env python3
"""Search Dungeon Core local asset library (manifest, picks, on-disk /Game paths)."""

from __future__ import annotations

import argparse
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
INVENTORY_PATH = Path(r"H:\Unreal Assets\asset-pack-inventory.md")

ASSET_SUFFIXES = (".uasset", ".umap")
SKIP_CONTENT_DIRS = {
    "__ExternalActors__",
    "__ExternalObjects__",
    "Collections",
    "Developers",
    "Splash",
    "StarterContent",
    "Demo",
}


def load_manifest() -> list[dict]:
    data = yaml.safe_load(MANIFEST_PATH.read_text(encoding="utf-8"))
    return list(data.get("packs") or [])


def load_pack_roots() -> dict[str, dict]:
    if not PACK_ROOTS_PATH.is_file():
        return {}
    roots = json.loads(PACK_ROOTS_PATH.read_text(encoding="utf-8"))
    return {p["id"]: p for p in roots.get("packs", [])}


def load_picks() -> list[tuple[str, str]]:
    if not LIBRARY_PATH.is_file():
        return []
    text = LIBRARY_PATH.read_text(encoding="utf-8")
    block = re.search(
        r"### Dungeon Core — suggested picks from local library\s*\n\n(\|.*?\n\|.*?\n(?:\|.*\n)+)",
        text,
        re.DOTALL,
    )
    if not block:
        return []
    rows: list[tuple[str, str]] = []
    for line in block.group(1).splitlines()[2:]:
        if not line.startswith("|"):
            continue
        cells = [c.strip() for c in line.strip("|").split("|")]
        if len(cells) >= 2:
            rows.append((cells[0], cells[1]))
    return rows


def normalize_mounts(value) -> list[str]:
    if value is None:
        return []
    if isinstance(value, list):
        return [str(v) for v in value]
    return [str(value)]


def package_path(content_root: Path, asset_file: Path) -> str:
    rel = asset_file.relative_to(content_root)
    stem = asset_file.stem
    folders = rel.parts[:-1]
    return "/Game/" + "/".join(folders + (stem,))


def search_manifest(packs: list[dict], query: str) -> list[dict]:
    q = query.casefold()
    hits = []
    for pack in packs:
        blob = " ".join(
            [
                pack.get("id", ""),
                pack.get("type", ""),
                pack.get("summary", ""),
            ]
        ).casefold()
        if q in blob or q in pack.get("id", "").casefold():
            hits.append(pack)
    return hits


def search_picks(query: str) -> list[tuple[str, str]]:
    q = query.casefold()
    return [(theme, assets) for theme, assets in load_picks() if q in theme.casefold() or q in assets.casefold()]


def search_inventory_headings(query: str, limit: int) -> list[str]:
    if not INVENTORY_PATH.is_file():
        return []
    q = query.casefold()
    hits: list[str] = []
    for line in INVENTORY_PATH.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("## "):
            title = line[3:].strip()
            if q in title.casefold():
                hits.append(title)
                if len(hits) >= limit:
                    break
    return hits


def find_assets(
    pack_roots: dict[str, dict],
    pattern: str,
    pack_id: str | None,
    limit: int,
) -> list[tuple[str, str, str]]:
    """Returns (catalog_id, /Game/path, filesystem path)."""
    regex = re.compile(pattern, re.IGNORECASE)
    results: list[tuple[str, str, str]] = []
    ids = [pack_id] if pack_id else sorted(pack_roots.keys())
    for pid in ids:
        entry = pack_roots.get(pid)
        if not entry or not entry.get("hasUproject"):
            continue
        content = entry.get("contentPath")
        if not content:
            continue
        root = Path(content)
        if not root.is_dir():
            continue
        for path in root.rglob("*"):
            if not path.is_file():
                continue
            if path.suffix not in ASSET_SUFFIXES:
                continue
            if any(part in SKIP_CONTENT_DIRS for part in path.parts):
                continue
            if path.name == "Map.umap" or path.parent.name in ("Maps", "Map"):
                continue
            if not regex.search(path.stem):
                continue
            game = package_path(root, path)
            results.append((pid, game, str(path)))
            if len(results) >= limit:
                return results
    return results


def cmd_pack(pack_id: str, packs: list[dict], pack_roots: dict[str, dict]) -> int:
    manifest = next((p for p in packs if p["id"] == pack_id), None)
    if manifest is None:
        fuzzy = [p for p in packs if pack_id.casefold() in p["id"].casefold()]
        if len(fuzzy) == 1:
            pack_id = fuzzy[0]["id"]
            manifest = fuzzy[0]
        else:
            print(f"No manifest entry for '{pack_id}'", file=sys.stderr)
            return 1
    root = pack_roots.get(pack_id)
    print(f"## {pack_id}")
    print(f"- type: {manifest.get('type', '?')}")
    print(f"- summary: {manifest.get('summary', '')}")
    if root:
        print(f"- projectRel: {root.get('projectRel')}")
        print(f"- uproject: {root.get('uproject')}")
        print(f"- contentPath: {root.get('contentPath')}")
        mounts = normalize_mounts(root.get("contentMounts"))
        if mounts:
            print(f"- contentMounts: {', '.join(mounts)}")
            for m in mounts:
                if m not in SKIP_CONTENT_DIRS and m not in ("Maps", "Map", "Material"):
                    print(f"  - example prefix: /Game/{m}/")
    else:
        print("- (no pack-roots entry — run Sync-PackRoots.ps1)")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Search local Unreal asset library.")
    parser.add_argument("-q", "--query", help="Search manifest summaries and catalog ids.")
    parser.add_argument(
        "-a",
        "--assets",
        metavar="PATTERN",
        help="Regex on asset file names (e.g. SM_, MI_Stone, Wall).",
    )
    parser.add_argument("--pack", help="Limit --assets to one catalog id.")
    parser.add_argument("--pack-info", metavar="ID", help="Show manifest + pack-roots for one id.")
    parser.add_argument("--list-packs", action="store_true", help="List catalog ids from manifest.")
    parser.add_argument("--limit", type=int, default=40, help="Max asset hits (default 40).")
    parser.add_argument(
        "--inventory",
        action="store_true",
        help="Also search H:\\Unreal Assets\\asset-pack-inventory.md section titles.",
    )
    args = parser.parse_args()

    if not MANIFEST_PATH.is_file():
        print(f"Missing {MANIFEST_PATH}", file=sys.stderr)
        return 1

    packs = load_manifest()
    pack_roots = load_pack_roots()

    if args.list_packs:
        for p in sorted(packs, key=lambda x: x["id"].casefold()):
            print(f"{p['id']}\t{p.get('type', '')}")
        return 0

    if args.pack_info:
        return cmd_pack(args.pack_info, packs, pack_roots)

    if not args.query and not args.assets:
        parser.print_help()
        return 1

    if args.query and not args.assets:
        print("### Manifest matches")
        hits = search_manifest(packs, args.query)
        if not hits:
            print("(none)")
        for pack in hits:
            print(f"- **{pack['id']}** ({pack.get('type', '?')}): {pack.get('summary', '')}")

        print("\n### Suggested picks (library.md)")
        pick_hits = search_picks(args.query)
        if not pick_hits:
            print("(none)")
        for theme, assets in pick_hits:
            print(f"- **{theme}**: {assets}")

        if args.inventory:
            print("\n### Inventory headings (deep detail on H:)")
            inv = search_inventory_headings(args.query, limit=15)
            if not inv:
                print("(none)")
            for title in inv:
                print(f"- {title}")

    if args.query and args.assets:
        print("### Manifest matches (query)")
        hits = search_manifest(packs, args.query)
        for pack in hits[:10]:
            print(f"- **{pack['id']}**")

    if args.assets:
        print("\n### On-disk assets (/Game paths)")
        if not pack_roots:
            print("pack-roots.json missing — run Sync-PackRoots.ps1", file=sys.stderr)
            return 1
        found = find_assets(pack_roots, args.assets, args.pack, args.limit)
        if not found:
            print("(none)")
        for pid, game, fs in found:
            print(f"- `{game}` — pack `{pid}`")
            print(f"  `{fs}`")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
