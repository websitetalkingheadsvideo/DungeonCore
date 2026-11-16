# Store

The in-game shop where the core spends mana on things to place in rooms. First built 2026-11-13. This page was brought up to date with the build on 2026-11-16: **eleven** store items (Map Table added as #10), seven of the original ten with their own models, and an edit mode for turning placed items. Most items still do nothing once placed.

Related: trap behavior in [General.md](./General.md), generator and chest shells in [Theme.md](./Theme.md), unlock order in [Tutorial.md](./Tutorial.md), parked ideas in [Future.md](./Future.md).

## How the store works (current build)

### Buying and placing

- The store is a list of **eleven** rows on the left of the dungeon view, under the mana bar (add `StoreItem_10` / `ItemIndex` 10 when the HUD is extended). Each row shows an icon, the item name and its price.
- **One gesture:** drag a store row onto the grid and release on a valid room square to place it.
- **Rooms only:** an item goes on an empty square of a room, never on a corridor, a single dug square, or the core square. A room is at least 2x2. A bad drop shows "Items can only go on an empty room square" and spends nothing.
- **Not enough mana:** pressing a row the player can't afford shows the NOT ENOUGH MANA alert and no drag starts.
- While dragging, a ghost block follows the cursor over the grid. The ghost is still a plain block for every item.
- A placed item shows its model, or an orange placeholder block if it has no model yet, with the item name floating above it.
- Placed items cannot be sold, moved or removed yet.

### Edit mode (turning items)

- An **Edit** button sits at the top right of the dungeon view. It is dark blue and reads "Edit" when off, gold and reads "Editing" when on.
- While edit mode is on, a hint under the button reads "Click an item, then left click it or use the left / right arrow keys to turn it".
- Click a placed item to select it. A ring appears on its square.
- Turn the selected item in 90 degree steps: left click it again or press the right arrow for clockwise, the left arrow for the other way.
- Click the button again to leave. The ring goes away.
- Works on all **eleven** items. While edit mode is on, clicks do not dig and do not open the monster menu.

### Glow and theme

- Models with a grey emission map glow in the theme's color, the same color as the room highlight: Gothic gold `(1.0, 0.78, 0.45)`, Goblin Tunnels amber `(1.0, 0.55, 0.2)`, Fungal Warren green `(0.35, 1.0, 0.45)`, Sunken Ruins teal `(0.2, 0.9, 0.8)`.
- Each of those materials has two parameters: `EmissiveTint` (set by the theme) and `EmissiveStrength` (1.5).
- The Gravity Well Trap keeps its own white and purple glow and ignores the theme. Loot Bait and the Spore Generator have no emission map, so they do not glow.

### How it is wired

- **HUD:** `WBP_DungeonHUD` (a plain User Widget). The rows are `WBP_StoreItem` widgets, `StoreItem_0` to `StoreItem_10`, each with an `ItemIndex`, a `Label` and an `IconTex`. A full-screen `WBP_DropZone` behind them takes the drop. `Btn_Edit`, `Txt_Edit` and `Txt_EditHint` are the edit-mode widgets.
- **Grid:** `BP_DungeonGrid` holds the item list in `ItemNames` and `ItemCosts`. `StoreDragStart` checks mana, `StoreDragOver` moves the ghost, `StoreDragDrop` calls `TryPlace`. `TryPlace` checks the rules, spends the mana and adds the model.
- **Models:** each item with a model has its own instanced mesh component on the grid (`MonsterGens`, `ManaGens`, `BeamTraps`, `PlateTraps`, `GravTraps`, `LootBaits`, `SporeGens`). Items without one share `Items`, the placeholder block.
- **Rotation:** `ItemInst` and `ItemYaw` remember each square's model and facing. `EditTick` reads the button, the click and the arrow keys; `RotateItemTo` turns the model; `ToggleEdit` switches the mode and the button's look.
- **Theme color:** `ApplyTheme` calls `TintItems`, which sets `EmissiveTint` on every themed model.
- **Icons:** textures in `/Game/UI/Icons/`, made from [Reference Images/icons/](e:\Dungeon Core\Design\Reference Images\icons). Do not leave `.png` files inside `DungeonCore/Content/UI/Icons/` next to the `.uasset` textures; Unreal will keep asking to re-import them.
- **Stop Play** before changing either blueprint through MCP.
- **Model folders:** `/Game/Props/StoreItems/`, one folder per item. Each import also brought a backdrop mesh and material (`*_Backdrop`, `Backdrop_001`) that the game does not use.
- **Adding a model for an item:** turn on "Used with Instanced Static Meshes" on its material (without it the model renders plain grey), add an instanced mesh component to the grid, and add the item to `TryPlace` and `RotateItemTo`. If it has a grey emission map, also add the tint to its material and the component to `TintItems`.
- **Grid square size:** the build uses 200 UU squares. [General.md](./General.md) says 300 UU. Models are placed at their imported size, which fits a 200 UU square.
- The C++ classes `DungeonLegacyStoreHudWidget`, `DungeonLegacyStoreDragLibrary`, `DungeonStoreDragButton`, `DungeonStoreGridDropZone` and `DungeonStoreItemDragDropOperation` are not used by the current HUD. The old `tmp/fix-*store*` scripts are history only.

## Items

Prices are placeholders. "Needs to open" has two columns on purpose: what the build does today, and what the design docs already say. Nothing in the second column is built yet. Every item is open from the start in the current build.

**Done column:** use `[ ]` until the item is **fully shipped** (store row, place rules, icon, model when applicable, and gameplay match the row). Change to `[x]` when you mark it complete.

| # | Done | Item | Price (mana) | What it does today | Model in the build | Needs to open: design docs | Icon file |
| --- | --- | --- | ---: | --- | --- | --- | --- |
| 0 | [ ] | **Monster Generator** | 40 | Click it to open the monster menu and buy creatures | `Monster_Generator_v2`, theme glow | Creature menu appears at tutorial beat 8 (first defender), after the first trap | `monster_generator.png` |
| 1 | [ ] | **Mana Generator** | 30 | +1 mana per second each | `mana_generator/Mana_Generator_mana_generator`, theme glow | Works only inside a **powered** room ([Future.md](./Future.md)). No unlock step written | `mana_generator.png` |
| 2 | [ ] | **Prism Beam Trap** | 35 | Nothing yet | `beam_trap/Prism_Beam_Trap_PrismBeamTrap`, theme glow | Needs wall mirrors and sightlines to work (see Mirror Pillar below). No unlock step written | `prism_beam_trap.png` |
| 3 | [ ] | **Resonance Plate Trap** | 20 | Nothing yet | `PlateTrap/Plate_Trap_Plate_Trap`, theme glow | First trap the player gets: trap menu appears at tutorial beat 7, once the first dug room is powered | `resonance_plate_trap.png` |
| 4 | [ ] | **Gravity Well Trap** | 45 | Nothing yet | `gravity_well_trap/Gravity_Well_Trap_tmpq61u_eep_obj`, own white and purple glow | No unlock step written | `gravity_well_trap.png` |
| 5 | [ ] | **Runaway Cart Trap** | 30 | Nothing yet | None, placeholder block | Needs a **built rail line** to run on. Rails are a parked idea ([Future.md](./Future.md)) | `runaway_cart_trap.png` |
| 6 | [ ] | **Loot Bait** | 15 | Nothing yet | `LootBait/Loot_Bait_Loot_Bait`, no glow | Taught in free play, after the tutorial ("first loot placed as bait"). No unlock step written | `loot_bait.png` |
| 7 | [ ] | **Unshackled Golem** | 50 | Nothing yet | None, placeholder block | No unlock step written | `unshackled_golem.png` |
| 8 | [ ] | **Shard Sentinel** | 25 | Nothing yet | None, placeholder block | No unlock step written | `shard_sentinel.png` |
| 9 | [ ] | **Spore Generator** | 30 | Nothing yet | `spore_generator/Spore_Generator_Spore_Generator`, no glow | Emits a **crowd of spores**; invaders in the **same powered room** fall **asleep** while the generator is active. **Neutral shell for now:** round fungal cap on a stalk rooted to the floor. **Per-theme shells** later (see [Theme.md](./Theme.md) pattern used for mana/monster generators). | `spore_generator.png` (not made yet, so the row has no icon) |
| 10 | [ ] | **Map Table** | 25 | **+1 fog-of-war reveal** (stacking; see below) | TBD — add mesh + `TryPlace` when wired | **Base reveal 2**, **max 4**, **max two tables** per dungeon. **5 ft × 5 ft** table with rolled maps and compass tools; one room square placement. | `map_table.png` |

**Fog-of-war reveal (Map Table):** beyond dug floor, fog stays clear for a number of **grid squares**. **Base: 2.** Each Map Table **+1**, capped at **4** (two tables).

### Still to do on the current eleven

- [ ] Models for the Runaway Cart Trap, Unshackled Golem and Shard Sentinel.
- [ ] Icon for the Spore Generator.
- [ ] Map Table: model, icon, fog reveal logic, HUD row 10.
- [ ] A matching ghost while dragging, instead of the plain block.
- [ ] Behavior for every item except the two generators and Map Table reveal (when implemented).

## Proposed items

Approved 2026-11-16 as the next items for the store. None are built, none have art or icons, and prices are guesses set against the current list. Each one is here because it fills a gap the ten items above leave open.

### Power and economy

| Done | Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | --- | ---: | --- | --- |
| [ ] | **Ley Conduit** | 25 | A crystal pylon that powers the room it stands in, so the room counts as **powered** without touching the core's own reach. | Mana Generators and Spore Generators are meant to work only in powered rooms. Without this, every working room has to sit next to the core. |
| [ ] | **Mana Reservoir** | 35 | Raises the mana cap by 25 each. Invaders who break one spill its share of stored mana. | The cap is 100 and the dearest item is 50. A deeper dungeon needs somewhere to bank mana between waves, and it gives invaders a second thing worth attacking. |
| [ ] | **Siphon Font** | 40 | Invaders who fall in this room give extra essence. | Rewards the player for choosing *where* the fight happens, which is the core loop of the genre ([Overall.md](./Overall.md)). |

### Traps and control

| Done | Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | --- | ---: | --- | --- |
| [ ] | **Mirror Pillar** | 15 | A free-standing mirror that turns a Prism Beam 90 degrees. The way it faces sets the turn, so the player aims it with edit-mode rotation. | The Prism Beam "needs wall mirrors and sightlines" and nothing in the store supplies them. This is the missing half of that trap. |
| [ ] | **Drag Lattice** | 20 | A floor lattice of slow light. Invaders crossing it move at half speed. Does no damage. | Every other trap hurts. A cheap slow makes the beam, the plate and the cart land more often, and gives new players an obvious first combo. |
| [ ] | **Displacement Glyph** | 45 | Bought as a linked pair. An invader who steps on one is moved to the other. | Lets the player send a party back to the entrance, split it, or drop it into the trap room. One-way, so the order placed matters. |
| [ ] | **Crystal Barricade** | 10 | A waist-high wall of crystal that blocks one square until invaders break it. Defenders can pass. | The cheapest way to buy time or bend a path. Nothing in the store currently changes where invaders can walk. |
| [ ] | **Mirage Veil** | 30 | Makes one room edge look like solid rock to invaders. A scout or a party that has seen through one before ignores it. | Hides the mana room or the short way to the core. Pairs with Loot Bait: one item pulls invaders in, this one steers them away. |

### Defender support and warning

(Map Table is **store item #10** above; fog stacking rules are documented there.)

| Done | Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | --- | ---: | --- | --- |
| [ ] | **Mending Node** | 30 | Slowly restores defenders and constructs standing in the room. | Golems and sentinels cost 50 and 25. Without a way to recover them, every fight is a straight loss of mana. |
| [ ] | **Alarm Chime** | 15 | Rings when invaders enter its room. Defenders within a few squares come to it. | Defenders currently have no reason to move toward a fight they cannot see. Also the natural place for the "under attack" audio cue. |
| [ ] | **Scrying Lens** | 20 | Clears fog of war in a wide ring around it and shows invaders inside that ring. | Fog only lifts two squares from dug floor without Map Tables. This is the only way to **watch invaders** in fog the player has not dug up to. |
| [ ] | **Decoy Core** | 60 | A dim copy of the core. Invaders treat it as the real one until they reach it. Bought once per run. | The core is the only thing that ends the run. A decoy is the classic late answer to a party that knows the way in. |

### Mimic

Added 2026-11-16 at Andy's request. Based on the Dungeons & Dragons mimic: a shapechanger that sits perfectly still looking like an ordinary object, glues itself to whoever touches it, then bites. It is slow, tough for its size, and immune to acid.

| Done | Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | --- | ---: | --- | --- |
| [ ] | **Mimic** | 35 | A creature that looks exactly like **Loot Bait** to invaders. The first invader to open it is stuck fast and bitten until it breaks free or dies. | Loot Bait only pulls invaders in. The Mimic punishes them for coming, and makes every real chest in the dungeon a gamble. |

How it would play:

- **Disguise:** invaders cannot tell a Mimic from Loot Bait until one touches it. It draws them exactly as Loot Bait does.
- **Stick:** the invader who opens it is held on that square and cannot walk away. The rest of the party has to choose between helping and moving on.
- **Bite:** it bites the held invader each second, with a little acid on top that keeps hurting for a moment after.
- **Revealed:** once it has bitten, the disguise is gone. It stays where it is, fights anyone in reach, and creeps toward the nearest invader at a crawl.
- **Reset:** if it survives the wave, it settles back into its chest shape for the next one.
- **Trust:** a party that has been bitten grows wary and is slower to open the next chest, real or not. That is the price of using one.
- **For the player:** the Mimic always shows a tell the invaders cannot see, a thin seam of teeth along the lid, so the player never confuses it with real bait.
- **Facing:** the lid opens toward the way it faces, so edit-mode rotation decides which side the invader has to stand on.

Good pairings: among two or three real Loot Baits so the party cannot guess; on a Drag Lattice so help arrives late; in a Spore Generator room, where a sleeping invader cannot struggle free.

Look: the Loot Bait shell for the current theme ([Theme.md](./Theme.md)) with the tooth seam and a slow breathing motion when no invader is watching. Its true form shows only when it dies.

Open questions:

- Is it bought once and kept, like a defender, or used up when it dies?
- Does it get tougher for each invader it finishes?
- Can a scout spot it before touching it?
- Gothic and Fungal Warren have no chest shell yet ([Theme.md](./Theme.md)), so the Mimic has nothing to copy there.

### Open questions for these

- **Colors:** the Ley Conduit, Scrying Lens and Decoy Core all glow. They must stay off the reserved colors in [General.md](./General.md) (teal, emerald, white-hot, red, violet), and the Decoy Core must not be mistaken for the real core's `CoreColor`.
- **Mirage Veil and Crystal Barricade** sit on a room edge or block a square. Today every item occupies one room square and never blocks movement, so both need a new placement rule.
- **Displacement Glyph** is the only item sold as a pair. Decide whether the second glyph is a second drag or placed automatically.
- **Alarm Chime vs Spore Generator:** spores put invaders to sleep. Decide whether a chime in the same room wakes them.
- **Map Table vs Scrying Lens:** table raises **global** reveal depth from the dug edge; lens is a **local** clear ring plus invader intel. Decide whether both stack additively with no cap beyond two tables.
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
