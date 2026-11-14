# Wire hold-drag-release store (C++ + BP)

**Why this exists:** MCP cannot reliably rewrite `BP_DungeonGrid` graphs; the blueprint used `Pressed < 0` as “drop,” which is click semantics, not drag.

## C++ (done)

- [DungeonLegacyStoreHudWidget.h](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreHudWidget.h) — parent for `WBP_DungeonHUD`; latches slot while LMB held.
- [DungeonLegacyStoreDragLibrary.h](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreDragLibrary.h) — `EvaluateLegacyStoreDrag` for grid tick.

**Build** the project (Live Coding or full compile).

## Editor (5 min)

1. **WBP_DungeonHUD** → Class Settings → parent **DungeonLegacyStoreHudWidget** → compile. Remove BP `GetPressedItem` / `GetHoveredItem` if they block override, or call **GetPressedStoreSlot** / **GetHoveredStoreSlot** from grid instead.

2. **BP_DungeonGrid** → add int **PendingPress** (default -1).

3. **TickStore:** after HUD ref, call **Get Pressed Store Slot** (or old GetPressedItem if still on BP parent). Then **Evaluate Legacy Store Drag** (PendingPress, SelectedItem). Branch on result:
   - `PressedIndex >= 0` && SelectedItem < 0 → existing arm/mana check → SetSelectedItem
   - `bShowGhost` → ghost branch (UpdateCursorCell + move ghost)
   - `bTryPlaceOnLmbRelease` → TryPlace, clear SelectedItem
   - `bAllowDigClick` → OnLeftClick

   **Delete** the old `UpdateDrag` branch that uses **Pressed < 0** for drop.

4. Stop PIE → Play. **Hold** store row → drag → **release** on room.

With MCP up: run [fix-update-drag-one-gesture.py](e:\Dungeon Core\tmp\fix-update-drag-one-gesture.py) only after manual pin fix, or wire the library nodes above instead of DSL.