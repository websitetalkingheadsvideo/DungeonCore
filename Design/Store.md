# Store

The in-game shop where the core spends mana on things to place in rooms. Built 2026-11-13 as a first pass: the list, prices and placement work; most items do nothing yet.

Related: trap behavior in [General.md](./General.md), generator and chest shells in [Theme.md](./Theme.md), unlock order in [Tutorial.md](./Tutorial.md), parked ideas in [Future.md](./Future.md).

## How the store works (current build)

- The store is a list on the left of the dungeon view, under the mana bar.
- **One gesture (not click-then-click):** **LMB down** on a store row → keep the button held while moving over the grid (ghost follows) → **LMB up** on a valid room square to place. Do **not** use UMG drag-drop; the grid polls pressed store index + global LMB. A broken build instead behaves like **click item → click room** because `UpdateDrag` used `Pressed < 0` (store released) as “drop”, which cancels drag before you reach the grid.
- **C++ (authoritative drag):** [DungeonLegacyStoreHudWidget](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreHudWidget.h) (hold latch on `Btn_*`), [DungeonLegacyStoreDragLibrary](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreDragLibrary.h) (`EvaluateLegacyStoreDrag`). Wire steps: [wire-legacy-store-drag.md](e:\Dungeon Core\tmp\wire-legacy-store-drag.md).
- **Rooms only:** placement checks `RoomId > 0` (see `DetectRooms` on the grid)—not every dug floor tile. You need a **2×2 room** (log: `New room created - rooms: N`). Corridors and single dug cells show floor but **reject** with “Items can only go on an empty room square”. The placement ghost only appears on valid room cells (same rules as `TryPlace`). Never on the core square.
- **Not enough mana:** pressing an item the player can't afford shows a message and no drag starts.
- **Bad drop:** letting go anywhere else shows a message and spends nothing.
- Placed items show as an orange placeholder block with the item name above it.
- Store list UI: `WBP_DungeonHUD` — nine store rows (`Btn_0`–`Btn_8`): each button wraps a horizontal row (`Icon_N` 48×48 + `Cost_N` mana label). Icon/cost/row use **Self Hit Test Invisible** so presses hit the button (placement drag uses `Button|IsPressed`, not UMG drag-drop). Rows are **left-aligned / auto width** so the button background hugs icon + price. Hover tooltips stay on the button. Textures: `/Game/UI/Icons/` (from [Reference Images/icons/](e:\Dungeon Core\Design\Reference Images\icons)). Re-apply layout via [fix-store-dnd-rows.py](e:\Dungeon Core\tmp\fix-store-dnd-rows.py) + [fix-store-button-chrome.py](e:\Dungeon Core\tmp\fix-store-button-chrome.py).
- **Do not** leave `.png` files inside `DungeonCore/Content/UI/Icons/` next to the `.uasset` textures — Unreal will spam import/re-import dialogs. Canonical art stays in Reference Images; re-import via **Reimport** on the texture asset or run [wire-store-icons.py](e:\Dungeon Core\tmp\wire-store-icons.py).
- **Stop PIE** before MCP compiles `WBP_DungeonHUD`. If the store does nothing (no “Placing”, no ghost), the usual cause is **broken BindWidget** after MCP tree edits — `DungeonCore.log` shows `Accessed None` on `Btn_0`…`Btn_8`. Until those properties are non-null at runtime, **no script fix can make drag work** (the grid never sees a pressed row). **Editor fix (required):** stop PIE → open `WBP_DungeonHUD` → in the hierarchy select `Btn_0`–`Btn_8`, `Bar_Mana`, `Txt_Mana`, `Txt_Status` → enable **Is Variable** (eye icon) on each → **Compile** → save → Play. Helper scripts (best-effort): [fix-hud-widget-bindings.py](e:\Dungeon Core\tmp\fix-hud-widget-bindings.py), [reset-get-pressed-simple.py](e:\Dungeon Core\tmp\reset-get-pressed-simple.py), [fix-hud-rebuild-buttons.py](e:\Dungeon Core\tmp\fix-hud-rebuild-buttons.py) (may need finishing in-editor if icons fail).
- **Drag latch (HUD):** `GetPressedItem` must keep the pressed row index until **LMB release**, not until the button stops being hovered. Without latch, the grid sees `-1` the instant the cursor leaves the store and `UpdateDrag` aborts placement. Re-apply: [fix-get-pressed-item-latch.py](e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py).
- **Drop on LMB (grid), required for real drag:** `BP_DungeonGrid` → `UpdateDrag` → find branch **Pressed &lt; 0 AND Selected Item ≥ 0** → replace **Pressed &lt; 0** with **Left Mouse Button → Was Released** (player controller). Delete/replace, do not add a second drop path. Re-apply helpers (often fail): [fix-update-drag-release-v2.py](e:\Dungeon Core\tmp\fix-update-drag-release-v2.py), [fix-update-drag-one-gesture.py](e:\Dungeon Core\tmp\fix-update-drag-one-gesture.py).
- **While LMB held (grid):** add int `PendingPress` on the grid; in `TickStore` after `Get Pressed Item`, if index ≥ 0 set `PendingPress`; pass into `UpdateDrag`: if **LMB down** and `PendingPress ≥ 0` use `PendingPress`, else raw pressed index. [fix-tickstore-one-gesture.py](e:\Dungeon Core\tmp\fix-tickstore-one-gesture.py) (MCP pin issues—do in editor if needed).
- **Store row buttons:** set **Click Method = Down and Up** (or **Mouse Down** in UE5) on each `Btn_N` so arming happens on **press**, not after a full click cycle.

## Items

Prices are placeholders. "Needs to open" has two columns on purpose: what the build does today, and what the design docs already say. Nothing in the second column is built yet.

| # | Item | Price (mana) | What it does today | Needs to open: current build | Needs to open: design docs | Icon file |
| --- | --- | ---: | --- | --- | --- | --- |
| 0 | **Monster Generator** | 40 | Placeholder | Nothing, open from the start | Creature menu appears at tutorial beat 8 (first defender), after the first trap | `monster_generator.png` |
| 1 | **Mana Generator** | 30 | +1 mana per second each | Nothing, open from the start | Works only inside a **powered** room ([Future.md](./Future.md)). No unlock step written | `mana_generator.png` |
| 2 | **Prism Beam Trap** | 35 | Placeholder | Nothing, open from the start | Needs wall mirrors and sightlines to work. No unlock step written | `prism_beam_trap.png` |
| 3 | **Resonance Plate Trap** | 20 | Placeholder | Nothing, open from the start | First trap the player gets: trap menu appears at tutorial beat 7, once the first dug room is powered | `resonance_plate_trap.png` |
| 4 | **Gravity Well Trap** | 45 | Placeholder | Nothing, open from the start | No unlock step written | `gravity_well_trap.png` |
| 5 | **Runaway Cart Trap** | 30 | Placeholder | Nothing, open from the start | Needs a **built rail line** to run on. Rails are a parked idea ([Future.md](./Future.md)) | `runaway_cart_trap.png` |
| 6 | **Loot Bait** | 15 | Placeholder | Nothing, open from the start | Taught in free play, after the tutorial ("first loot placed as bait"). No unlock step written | `loot_bait.png` |
| 7 | **Unshackled Golem** | 50 | Placeholder | Nothing, open from the start | No unlock step written | `unshackled_golem.png` |
| 8 | **Shard Sentinel** | 25 | Placeholder | Nothing, open from the start | No unlock step written | `shard_sentinel.png` |

## Unlock rules that apply to everything

These come from the other design docs and are not built yet.

- **Progressive reveal:** during the tutorial, each menu appears as its system is taught. Skipping the tutorial opens everything at once ([Tutorial.md](./Tutorial.md)).
- **Absorb to unlock:** new monsters and traps are meant to come from what the core absorbs, not only from leveling ([Overall.md](./Overall.md)). Which store items this covers is not decided.
- **Core level:** boss chambers and powered pockets are capped by core level ([Dungeon Core Levels.md](./Dungeon%20Core%20Levels.md)). No store item is tied to a level yet.

## In the design docs but not in the store yet

- **Worker:** bought with essence at tutorial beat 3 ([Tutorial.md](./Tutorial.md)).
- **Family creatures:** spawned from the creature menu ([Monsters.md](./Monsters.md)). The store sells only the generator.
- **Door and archway:** room thresholds, one shell per theme ([Theme.md](./Theme.md)).
- **Rails:** needed by the Runaway Cart Trap ([Future.md](./Future.md)).

## Open questions

- What opens each of the items marked "No unlock step written": absorbing something, a core level, or nothing?
- Are the golem and sentinel bought once each, or do they need a generator like family creatures?
- Does the Monster Generator sell one family's creatures (the family picked on New Game) or any family?
- Final prices, and whether prices rise with each copy bought.
