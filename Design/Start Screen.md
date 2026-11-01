# Start Screen

## Idle state (unpowered core)

On launch: dead silent, deep graphic shadows.

**Nexus core**

- Massive unpolished geode, center of a dark basalt chamber
- Slow continuous spin on vertical axis
- Faint amethyst or sapphire pulse in crystal fractures; soft ambient light on obsidian floor tiles

**Text menu**

- Options: Continue, New, Load, Options
- Crisp translucent sharp-bordered text, left of core—matches in-game tactical HUD layout

## Dynamic hover states (sensing the network)

Cursor over each option sends a small shockwave through the room—a preview of the choice.

### Hover: New (spark of creation)

- Geode locks axis and spins faster
- One bright capillary of liquid light shoots from the core base across the floor to the front wall
- Hit wall blocks hairline-fracture into a faint glowing geometric grid—uncarved canvas before first worker

### Hover: Continue / Load (memory pulse)

- Slow expanding horizontal ring of pale teal light sweeps the room
- Ring briefly reveals a phantom wireframe dungeon layout on floor and walls; fades after ~1 s

### Hover: Options (calibration)

- Core light shifts amethyst → stable industrial emerald
- Two satellite crystal shards orbital-lock and spin around the core, reacting to mouse movement

## Click execution (chain reaction)

Instant, un-skippable transition into gameplay or overlay UI—no generic loading screen.

| Action | Result |
| --- | --- |
| **New Game** | Core flares white-hot ley-energy; acoustic shatter; front wall fractures to dust; camera pulls through breach into first-room dig UI |
| **Load / Options** | Core drifts back and scales down; stone or crystal slate glides in from the right for saves or settings without breaking the scene |

---

## New Game setup screen — implemented (2026-10-29)

### Camera and layout

- Hero shot: real 3×3 core chamber (`ADungeonShowcaseActor`)
- Camera: pitch −28°, yaw 0, arm 1300 UU, target offset (0, −340, +60) from chamber centre—core ~63% across frame
- Camera-side (min-X) wall open as cutaway
- Card: left-anchored, 56 px inset, ≤ ~30% of 1920 frame; tiles 2×2 grid, 190×54
- Scrim: 55% alpha, left 30% of frame only

### Interaction defaults

- Defaults: theme **Gothic**, family **Orcs**
- Theme tile click: re-skins floor/wall materials, four corner accent meshes, fill-light colour in place
- Family tile click: swaps creature on block (1, 2)

### Theme kits

| Theme | Floor | Wall | Accent | Fill light |
| --- | --- | --- | --- | --- |
| **Gothic** | GothicCathedral `MI_Floor_Tile_B` | `MI_Stone_Tile_A` | `SM_Pillar_A` | Oil-lamp gold (1.0, 0.78, 0.45) |
| **Goblin Tunnels** | FantasyDungeon `MI_Dirt_Floor_v01` | GreenCave `MI_LargeWall_b` | `SM_SupportBeam` | Torch amber (1.0, 0.55, 0.2) |
| **Fungal Warren** | GreenCave `MI_FlatStone_a` | `MI_LargeWall_b` | Crystal_Cave `SM_CrystalCluster_D_01` @ 160 UU | Spore green (0.35, 1.0, 0.45) |
| **Sunken Ruins** | `MI_FlatStone_a` | `MI_Stone_Tile_A` | CavernMahal `SM_ColosseumPillar_01` (circular) | Teal (0.2, 0.9, 0.8) |

Accents default **250 UU** unless noted.

### Wall fixtures

One per standing wall; camera-side wall is cutaway.

| Theme | Mesh | Scale | Mount | Cast shadow |
| --- | --- | --- | --- | --- |
| Gothic | `SM_Oil_Lamp` | 50 UU tall | 155 | 45 |
| Goblin | `SM_Torch` | 70 UU | 150 | 50 |
| Fungal | `SM_LargeMushrooms_a` | 110 UU, floor-seated | 0 | 35 |
| Sunken | `SM_Lamp_A` (Ashes of Bazaar) | 55 UU | 40 | 40 |

Each fixture has a point light using `AccentLightColor`.

**Rejected:** Lost_Cave `SM_GlowingPlant_a`—source pack references missing `MI_Atlas`.

### Creature art

Pending (see project `TODO.md`). Until installed, card shows: `Creature art pending — Fab: <pack>` under the summary.

Fonts on this flow: [fonts.md](./fonts.md).
