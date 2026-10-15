---
name: asset-library-lookup
description: Find Unreal environment packs, meshes, materials, and /Game asset paths in the Dungeon Core library using library-manifest.yaml, pack-roots.json, library.md picks, and optional H drive inventory. Use when the user asks what assets are available, which pack fits a theme (dungeon, gothic, cave), where to find SM_/MI_ assets, or before fab-asset-migration.
---

# Asset Library Lookup (Dungeon Core)

Find packs and concrete `/Game/...` paths **before** migrating into the project. Migration is a separate skill (`fab-asset-migration`).

## Source map

| Need | Where |
| --- | --- |
| Short pack list + one-line summary | `library.md` (generated table) or `tools/asset-library/library-manifest.yaml` |
| Theme → pack ids | `library.md` → **Dungeon Core — suggested picks** |
| `contentPath`, `contentMounts`, nested `.uproject` | `tools/asset-library/pack-roots.json` (refresh: `Sync-PackRoots.ps1`) |
| Gumroad tags, long descriptions | `H:\Unreal Assets\asset-pack-inventory.md` (optional; names may not match catalog id) |
| Fab-owned items | `library.md` → Fab section (partial) |

**Catalog id** = top-level folder under `H:\Unreal Assets\`. Never assume `H:\Unreal Assets\<id>\Content` — use `contentPath` from pack-roots.

## Workflow

```
- [ ] 1. Run search_library.py (manifest + picks; add --assets for SM_/MI_)
- [ ] 2. For a candidate pack, --pack-info to print mounts and contentPath
- [ ] 3. If still thin, grep asset-pack-inventory.md on H: (--inventory on search)
- [ ] 4. Hand off seeds to fab-asset-migration / Get-MigratePackCommand.ps1
```

Always run search in the repo root (paths are relative to `tools/asset-library/`).

## Commands

List every local catalog id:

```powershell
python tools/asset-library/search_library.py --list-packs
```

Theme / keyword (dungeon, gothic, decal, Leartes):

```powershell
python tools/asset-library/search_library.py -q dungeon
python tools/asset-library/search_library.py -q gothic --inventory
```

Resolve one pack (paths + `/Game` mount prefixes):

```powershell
python tools/asset-library/search_library.py --pack-info MedievalFantasyDungeon_5.4
```

Find meshes/materials on disk (regex on **file name**, not folder):

```powershell
python tools/asset-library/search_library.py -a "SM_.*Wall" --pack MedievalFantasyDungeon_5.4 --limit 25
python tools/asset-library/search_library.py -a "^MI_" -q cave --limit 30
```

`-q` and `-a` can be combined: manifest/picks first, then asset file hits.

After new installs on `H:\`:

```powershell
& tools/asset-library/Sync-PackRoots.ps1
python tools/asset-library/update_library_table.py --check
```

## How to answer the user

1. Lead with **pack ids** that match the request and **why** (from summary or picks row).
2. For usable assets, give **`/Game/...` package paths** from `--assets` output (migration seeds).
3. Note **mount ≠ catalog id** when relevant (e.g. `MedievalFantasyDungeon_5.4` → `/Game/FantasyDungeon/...`).
4. Do not copy `.umap` or migrate whole packs unless asked — prefer a short list of SM_/MI_ instances.

## Related

- Migrate into project: `fab-asset-migration` skill + `Get-MigratePackCommand.ps1`
- Edit catalog summaries: `library-manifest.yaml` → `update_library_table.py`
