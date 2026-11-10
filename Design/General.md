# Crystalline Ley-Line Arcana

The art pillar shifts the classic dungeon core away from mud and torches into a pristine, high-concept space: raw geometry, refracted light, and cosmic energy. Strategy and planning are the mechanical focus, so visuals lean into **clean boundaries**—grid layouts stay readable, not cluttered.

## Grid and scale

- The dungeon is built from **cubes**: one cube = one grid cell = one dig block.
- **Cube size: 300 UU** (~10 ft) on every side. Room meshes, walls, floors, and dig blocks all snap to this grid.
- **Starting room (core chamber):** 3×3 cubes (900 × 900 UU floor), one cube tall (300 UU).
- **Core placement:** the core sits in the **center cube** (1, 1); the eight surrounding cubes form the rest of the chamber.
- **Core size grows with core level** (draft): the core starts small at level 1 and grows as it levels up. Exact sizes TBD. See [Dungeon Core Levels.md](./Dungeon%20Core%20Levels.md).

## Room tiles and environmental textures

Floor and wall materials come from the selected **skin** ([Creatures and style.md](./Creatures%20and%20style.md)). The rules below apply on top of every skin.

### Lighting

Each skin has its own palette; all share a **dramatic** treatment—strong key light, high contrast, deep shadows.

### Floors (foundation)

Tile borders are **hair-thin fractures** that softly pulse when a room is powered—not painted grid lines.

### Walls (geode effect)

Digging a new room **exposes** glowing crystal clusters in the cut face, regardless of skin.

### Doors and thresholds

Floating vertical stone monoliths carved with interlocking celestial rings. When open, segments drift apart and a thin sheet of shimmering translucent energy fills the gap.

## Core crystal

- **One mesh** for all biomes; one master material with parameters `CoreColor`, `PulseIntensity`, `FractureGlow`, `StateOverride`.
- **Body is always frosted white quartz** (low emission). `CoreColor` tints only fracture glow, pulse, and the core's point light, so the body stays a neutral canvas.
- Each biome has a material instance with a default `CoreColor` that harmonizes with its fill light.
- The player can override `CoreColor` from a curated palette (~6–8 per skin) on the New Game card; the choice is saved.
- The core's point light follows `CoreColor`.
- Start screen uses **sapphire** as the default pulse before any save exists ([Start Screen.md](./Start%20Screen.md)).
- Biome variety comes from the pedestal and surrounding dressing, not the crystal.
- **Reserved colors (not in the palette):** teal, emerald, white-hot, red, **violet/amethyst** (Will / guidance only; see [Will.md](./Will.md)).
- Bright blooming white is `StateOverride` only (overload, New Game flare); body emission never reaches it.
- Sunken Ruins has a teal fill light (room dressing is fine), but its default `CoreColor` must not be teal—use deep blue or seafoam-silver.
- Color is **cosmetic only** for now. Gameplay affinity is parked in [Future.md](./Future.md).

## Ley-line grid and zone control

The core’s radius of influence reads like a growing circulatory system: liquid-light capillaries on floors and pillars.

| Zone state | Visual |
| --- | --- |
| **Powered** | Stable, bright, clean lines on grid boundaries |
| **Dormant / unpowered** | Dark, dim, cracked stone paths |
| **Under attack / overloaded** | Fracturing nodes; white-hot lightning between crystals |

## Trap concepts (geometric and resonance)

| Trap | Behavior |
| --- | --- |
| **Prism Beam** | Gem node on a pillar fires a beam that refracts off wall mirrors into a geometric kill web. The beam shows faintly in build mode and once built, so the player can read the web. |
| **Resonance Plate** | Quartz floor tile; harmonic vibration shatters non-magical kinetic armor and staggers enemies |
| **Gravity Well** | Ceiling geode reverses local gravity—pin to ceiling crystals, then drop |
| **Runaway Cart** | Mine cart released down a built rail line; crushes everything on the track. Natural fit for Goblin Tunnels (Old Mine kit) |

## Entity and defender aesthetic (core constructs)

| Construct | Look and motion |
| --- | --- |
| **Unshackled Golems** | Floating obsidian shards locked by a central ley-energy point; rigid, sudden mechanical moves |
| **Shard Sentinels** | Small darting prisms or wisps; no faces—track via light distortion and searchlight cones. Never violet and never the slow facet shimmer (both reserved for [Will](./Will.md)), so a defender can't be mistaken for the guide |

Biome-specific dressing and resident creatures: [Creatures and style.md](./Creatures%20and%20style.md).
