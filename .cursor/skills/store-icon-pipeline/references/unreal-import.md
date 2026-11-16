# Import approved icon into Unreal

**Requires** Unreal Editor open and **`project-0-Dungeon Core-unreal-mcp`** (or equivalent) connected. If MCP is down: report and stop at step 4 — do not leave PNG under `DungeonCore/Content/UI/Icons/` ([Design/Store.md](../../../../Design/Store.md)).

## Paths

| Role | Path |
| --- | --- |
| Canonical art (source of truth) | `Design/Reference Images/icons/{basename}.png` |
| UObject texture | `/Game/UI/Icons/{basename}` |
| HUD widget | `WBP_DungeonHUD` → `StoreItem_{N}` / `IconTex` for row `N` |

**Never** commit or leave loose `.png` next to `.uasset` in `Content/UI/Icons/`.

## MCP steps (single icon)

1. `create_folder` `/Game/UI/Icons` if missing.
2. `TextureTools.import_file`:
   - `folder_path`: `/Game/UI/Icons`
   - `asset_name`: `{basename}` (no extension)
   - `source_file`: absolute path to approved `Design/Reference Images/icons/{basename}.png` (forward slashes OK).
3. If re-importing: use **Reimport** / `bReplaceExisting` per existing project scripts.
4. Set `WBP_StoreItem` row `IconTex` → imported texture when row exists (see [tmp/wire-store-icons.py](../../../../tmp/wire-store-icons.py) pattern).
5. **Stop PIE** before blueprint compile/save.

## Helper scripts (batch / reference)

- [tmp/wire-store-icons.py](../../../../tmp/wire-store-icons.py) — HTTP MCP, rows 0–8
- [tmp/wire-store-icons-mcp.py](../../../../tmp/wire-store-icons-mcp.py)

Extend `STORE_ITEMS` when adding `Btn_9` / Spore or `StoreItem_10` / Map Table.

## Verify

- Grep `Saved/Logs/DungeonCore.log` for failed texture import.
- Icon visible on store row in PIE at 48×56 brush size (HUD may use 56×56).