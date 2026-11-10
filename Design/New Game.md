# New Game Setup Screen

**Status:** implemented (2026-10-29).

Reached from [Start Screen.md](./Start%20Screen.md) → **New**. The player picks a theme (biome) and creature family on a card over the real core chamber. Confirming the card triggers the New Game chain reaction (core flare, wall shatter, camera through the breach into the first-room dig UI), then the [Tutorial](./Tutorial.md) begins.

## Camera and layout

Concept mockup: [Reference Images/New Game - v4 smaller core.png](./Reference%20Images/New%20Game%20-%20v4%20smaller%20core.png) (Firefly; level-1 core size, Will, and selected monster family shown in the room). Earlier: [New_Game_Reference.png](./Reference%20Images/New_Game_Reference.png) (oversized core; superseded).

- Hero shot: real 3×3×1 core chamber (`ADungeonShowcaseActor`), core in the center cube at **level-1 size (~150 UU)**—see [General.md](./General.md) "Grid and scale"
- Camera: pitch −28°, yaw 0, arm 1300 UU, target offset (0, −340, +60) from chamber centre—core ~63% across frame
- Camera-side (min-X) wall open as cutaway
- Card: left-anchored, 56 px inset, ≤ ~30% of 1920 frame. **Three stacked panels, all options visible at once** (no click-through): **Theme** (4 rows), **Monsters** (6 rows), **Random Events** (3 rows). Each row = icon + label; selected row highlighted. Ref: v4 mockup above.
- **Start Game** button: bottom-right, same stone-plate style as the panel headers. Confirms the card and starts the New Game chain reaction.
- Scrim: 55% alpha, left 30% of frame only

## Interaction defaults

- Defaults: theme **Gothic**, family **Orcs**
- Theme tile click: re-skins floor/wall materials, four corner accent meshes, fill-light colour in place
- Family tile click: swaps creature on block (1, 2)

## Random events

Card option: **Random Events — Low / Medium / High**. Default **Medium**. Saved with the game.

- Sets how often unplanned events happen during play—good and bad (e.g. a tribe of workers, a free mana stone, a cave-in). Low = fewer ups and downs.
- Scales frequency only; the event pool is the same at every setting.
- No random events fire until the [Tutorial](./Tutorial.md) ends (or is skipped).
- Full design and event list: [Random Events.md](./Random%20Events.md).

## Theme kits

| Theme | Floor | Wall | Accent | Fill light |
| --- | --- | --- | --- | --- |
| **Gothic** | GothicCathedral `MI_Floor_Tile_B` | `MI_Stone_Tile_A` | `SM_Pillar_A` | Oil-lamp gold (1.0, 0.78, 0.45) |
| **Goblin Tunnels** | FantasyDungeon `MI_Dirt_Floor_v01` | GreenCave `MI_LargeWall_b` | `SM_SupportBeam` | Torch amber (1.0, 0.55, 0.2) |
| **Fungal Warren** | GreenCave `MI_FlatStone_a` | `MI_LargeWall_b` | Crystal_Cave `SM_CrystalCluster_D_01` @ 160 UU | Spore green (0.35, 1.0, 0.45) |
| **Sunken Ruins** | `MI_FlatStone_a` | `MI_Stone_Tile_A` | CavernMahal `SM_ColosseumPillar_01` (circular) | Teal (0.2, 0.9, 0.8) |

Accents default **250 UU** unless noted.

## Will (first contact)

The player meets [Will](./Will.md) here, before the first real choice.

- **Placement:** small pale-violet prism wisp with slow facet shimmer, orbiting near the core, clear of the card.
- **Intro:** one line when the screen opens (draft): *"Core. I am Will—an echo in your lines. Choose your ground; ask me if it puzzles you."* Dismissable; never blocks the card.
- **Hover:** shimmer brightens.
- **Click:** codex slate slides in from the right (same motion as Load / Options), scene stays live behind it.
- **FAQ (this screen only):**
  - What does a theme change?
  - Does the creature family change gameplay?
  - Can I change these later?
  - What is the core color for? (cosmetic only for now)
  - What is "Guided awakening"?
  - What do Random Events do?
- On confirm, Will follows the camera through the breach into the [Tutorial](./Tutorial.md).

## Wall fixtures

One per standing wall; camera-side wall is cutaway.

| Theme | Mesh | Scale | Mount | Cast shadow |
| --- | --- | --- | --- | --- |
| Gothic | `SM_Oil_Lamp` | 50 UU tall | 155 | 45 |
| Goblin | `SM_Torch` | 70 UU | 150 | 50 |
| Fungal | `SM_LargeMushrooms_a` | 110 UU, floor-seated | 0 | 35 |
| Sunken | `SM_Lamp_A` (Ashes of Bazaar) | 55 UU | 40 | 40 |

Each fixture has a point light using `AccentLightColor`.

**Rejected:** Lost_Cave `SM_GlowingPlant_a`—source pack references missing `MI_Atlas`.

### Candidates (in Fab library, not yet in project)

Add to project, then test against the current pick. If one fails, note why under **Rejected** and move on.

| Theme | Candidate (Fab pack) | Notes |
| --- | --- | --- |
| Gothic | **Pack of 12 High-Poly Candle 3D Models** | Cathedral feel; may need several per wall |
| Goblin Tunnels | **Medieval Torch** (wall torch) | Direct swap for `SM_Torch` |
| Goblin Tunnels | **Kerosene lamp**, **Cast Iron Barn Lantern** | Old Mine flavor |
| Fungal Warren | **Glowing Blue Lantern Cap - Free** | Glowing mushroom; emissive retinted to spore green |
| Sunken Ruins | **Free Lamp** | Alternative to `SM_Lamp_A` |

**VFX:** **Sparks Embers** for torch fire, dig shatter, and the New Game flare.

**Dressing:** Megascans **Bolete**, **Puffball**, and **Spotted Red Mushrooms** for Fungal Warren.

**Kit:** **Small Dungeon Modular Kit** (Roblox Showcase Prop)—may not ship in Unreal format or match the style. Try it; if it fails, note it and move on.

**Rules for all candidates:** retexture stylized assets toward the grounded look ([Monsters.md](./Monsters.md) art direction); retint emissive glows so none land on reserved colors (teal, emerald, white-hot, red, violet—see [General.md](./General.md)).

## Creature art

Pending (see project `TODO.md`). Until installed, card shows: `Creature art pending — Fab: <pack>` under the summary.

Fonts on this flow: [fonts.md](./fonts.md).
