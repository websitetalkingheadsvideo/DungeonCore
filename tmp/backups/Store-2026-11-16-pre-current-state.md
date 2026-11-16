# Store

The in-game shop where the core spends mana on things to place in rooms. Built 2026-11-13 as a first pass: the list, prices and placement work; most items do nothing yet.

Related: trap behavior in [General.md](./General.md), generator and chest shells in [Theme.md](./Theme.md), unlock order in [Tutorial.md](./Tutorial.md), parked ideas in [Future.md](./Future.md).

## How the store works (current build)

- The store is a list on the left of the dungeon view, under the mana bar.
- **One gesture (not click-then-click):** drag a store row onto the grid; release on a valid room square to place. **Use UMG drag-and-drop** (`OnDragDetected` on each store row, `UDragDropOperation` payload with slot index, `OnDrop` / `OnDragOver` on a grid drop target that calls the same `TryPlace` rules as today). The old hold-and-poll path (`GetPressedItem` + `TickStore` + `IsInputKeyDown`) is **abandoned** — it fought `UIOnly` input and never reliably armed `SelectedItem`.
- **C++ helpers (optional):** [DungeonLegacyStoreHudWidget](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreHudWidget.h) and [DungeonLegacyStoreDragLibrary](e:\Dungeon Core\Source\DungeonCore\Public\UI\DungeonLegacyStoreDragLibrary.h) were for the abandoned poll path; do not treat them as the store UX direction unless reparented for drag-source wiring only.
- **Rooms only:** placement checks `RoomId > 0` (see `DetectRooms` on the grid)—not every dug floor tile. You need a **2×2 room** (log: `New room created - rooms: N`). Corridors and single dug cells show floor but **reject** with “Items can only go on an empty room square”. The placement ghost only appears on valid room cells (same rules as `TryPlace`). Never on the core square.
- **Not enough mana:** pressing an item the player can't afford shows a message and no drag starts.
- **Bad drop:** letting go anywhere else shows a message and spends nothing.
- Placed items show as an orange placeholder block with the item name above it.
- Store list UI: `WBP_DungeonHUD` — **ten** store rows when **Spore Generator** ships (`Btn_0`–`Btn_9`; today’s build still has nine). Each button wraps a horizontal row (`Icon_N` 48×48 + `Cost_N` mana label). Each row is a **drag source** (detect drag if pressed → create drag operation). Icon/cost/row use **Self Hit Test Invisible** so hits reach the button; rows are **left-aligned / auto width**. Hover tooltips stay on the button. Textures: `/Game/UI/Icons/` (from [Reference Images/icons/](e:\Dungeon Core\Design\Reference Images\icons)). Layout helpers: [fix-store-dnd-rows.py](e:\Dungeon Core\tmp\fix-store-dnd-rows.py) + [fix-store-button-chrome.py](e:\Dungeon Core\tmp\fix-store-button-chrome.py).
- **Do not** leave `.png` files inside `DungeonCore/Content/UI/Icons/` next to the `.uasset` textures — Unreal will spam import/re-import dialogs. Canonical art stays in Reference Images; re-import via **Reimport** on the texture asset or run [wire-store-icons.py](e:\Dungeon Core\tmp\wire-store-icons.py).
- **Stop PIE** before MCP compiles `WBP_DungeonHUD`. If the store does nothing (no “Placing”, no ghost), the usual cause is **broken BindWidget** after MCP tree edits — `DungeonCore.log` shows `Accessed None` on `Btn_0`…`Btn_8`. Until those properties are non-null at runtime, **no script fix can make drag work** (the grid never sees a pressed row). **Editor fix (required):** stop PIE → open `WBP_DungeonHUD` → in the hierarchy select `Btn_0`–`Btn_8`, `Bar_Mana`, `Txt_Mana`, `Txt_Status` → enable **Is Variable** (eye icon) on each → **Compile** → save → Play. Helper scripts (best-effort): [fix-hud-widget-bindings.py](e:\Dungeon Core\tmp\fix-hud-widget-bindings.py), [reset-get-pressed-simple.py](e:\Dungeon Core\tmp\reset-get-pressed-simple.py), [fix-hud-rebuild-buttons.py](e:\Dungeon Core\tmp\fix-hud-rebuild-buttons.py) (may need finishing in-editor if icons fail).
- **Drop target:** full-area overlay (or root canvas slot) over the dungeon view — `OnDragOver` updates ghost via grid `UpdateCursorCell`; `OnDrop` calls `TryPlace` when `CursorOk`. Mana check before starting drag or on drop (same as today).
- **Legacy poll wiring (delete, do not extend):** `GetPressedItem` latch, `PendingPress`, `TickStore` → `UpdateDrag` press polling. Scripts under `tmp/fix-*store*` are archaeology only.

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
| 9 | **Spore Generator** | 30 | Placeholder | Nothing, open from the start | Emits a **crowd of spores**; invaders in the **same powered room** fall **asleep** while the generator is active. **Neutral shell for now:** round fungal cap on a stalk rooted to the floor (photoreal PBR ref in [Reference Images/Dungeon Items/](e:\Dungeon Core\Design\Reference Images/Dungeon Items)). **Per-theme shells** later (see [Theme.md](./Theme.md) pattern used for mana/monster generators). | `spore_generator.png` |

## Proposed items

Ideas only, added 2026-11-16. None are built, none have art or icons, and prices are guesses set against the current list. Each one is here because it fills a gap the ten items above leave open.

### Power and economy

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Ley Conduit** | 25 | A crystal pylon that powers the room it stands in, so the room counts as **powered** without touching the core's own reach. | Mana Generators and Spore Generators are meant to work only in powered rooms. Without this, every working room has to sit next to the core. |
| **Mana Reservoir** | 35 | Raises the mana cap by 25 each. Invaders who break one spill its share of stored mana. | The cap is 100 and the dearest item is 50. A deeper dungeon needs somewhere to bank mana between waves, and it gives invaders a second thing worth attacking. |
| **Siphon Font** | 40 | Invaders who fall in this room give extra essence. | Rewards the player for choosing *where* the fight happens, which is the core loop of the genre ([Overall.md](./Overall.md)). |

### Traps and control

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Mirror Pillar** | 15 | A free-standing mirror that turns a Prism Beam 90 degrees. The way it faces sets the turn, so the player aims it with edit-mode rotation. | The Prism Beam "needs wall mirrors and sightlines" and nothing in the store supplies them. This is the missing half of that trap. |
| **Drag Lattice** | 20 | A floor lattice of slow light. Invaders crossing it move at half speed. Does no damage. | Every other trap hurts. A cheap slow makes the beam, the plate and the cart land more often, and gives new players an obvious first combo. |
| **Displacement Glyph** | 45 | Bought as a linked pair. An invader who steps on one is moved to the other. | Lets the player send a party back to the entrance, split it, or drop it into the trap room. One-way, so the order placed matters. |
| **Crystal Barricade** | 10 | A waist-high wall of crystal that blocks one square until invaders break it. Defenders can pass. | The cheapest way to buy time or bend a path. Nothing in the store currently changes where invaders can walk. |
| **Mirage Veil** | 30 | Makes one room edge look like solid rock to invaders. A scout or a party that has seen through one before ignores it. | Hides the mana room or the short way to the core. Pairs with Loot Bait: one item pulls invaders in, this one steers them away. |

### Defender support and warning

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Mending Node** | 30 | Slowly restores defenders and constructs standing in the room. | Golems and sentinels cost 50 and 25. Without a way to recover them, every fight is a straight loss of mana. |
| **Alarm Chime** | 15 | Rings when invaders enter its room. Defenders within a few squares come to it. | Defenders currently have no reason to move toward a fight they cannot see. Also the natural place for the "under attack" audio cue. |
| **Scrying Lens** | 20 | Clears fog of war in a wide ring around it and shows invaders inside that ring. | Fog only lifts two squares from dug floor. This is the only way to watch an approach the player has not dug up to. |
| **Decoy Core** | 60 | A dim copy of the core. Invaders treat it as the real one until they reach it. Bought once per run. | The core is the only thing that ends the run. A decoy is the classic late answer to a party that knows the way in. |

### Open questions for these

- **Colors:** the Ley Conduit, Scrying Lens and Decoy Core all glow. They must stay off the reserved colors in [General.md](./General.md) (teal, emerald, white-hot, red, violet), and the Decoy Core must not be mistaken for the real core's `CoreColor`.
- **Mirage Veil and Crystal Barricade** sit on a room edge or block a square. Today every item occupies one room square and never blocks movement, so both need a new placement rule.
- **Displacement Glyph** is the only item sold as a pair. Decide whether the second glyph is a second drag or placed automatically.
- **Alarm Chime vs Spore Generator:** spores put invaders to sleep. Decide whether a chime in the same room wakes them.
- **Decoy Core** overlaps with Loot Bait as a lure. Keep both only if the decoy is clearly the late, expensive version.

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
- **Spore Generator:** stack multiple gens in one room; sleep vs traps that need awake targets; whether spores require a **powered** room like mana generators ([Future.md](./Future.md)).
