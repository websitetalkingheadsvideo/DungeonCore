# Dungeon Core Levels

Player-facing **core level** (1–10). The geode levels up from **experience points (XP)** earned through play, like a standard RPG level track—not per-creature ranks. **XP is separate from essence/mana**: XP is never spent, and leveling raises the core's **base mana per round** (see below). See [Overall.md](./Overall.md) (essence economy).

## XP curve

- **XP to advance** from level *n−1* → *n* (for *n ≥ 2*): **2000 × 2^(n−2)** (2000, 4000, 8000, …).
- Level **1** is the starting state (**0** XP).
- **Cumulative XP** to **be** level *L* = sum of all prior tier costs (level 10 total **1,022,000**).

## XP sources (draft)

Not finalized; amounts TBD.

- **Killing intruders** (main source)
- **Digging tunnels and building rooms** (main source)

## Mana per round

**Base ley drip** each planning round (before workers, traps, kills, or events):

**Mana per round** = **20 × Core level × Dungeon strata (max)** from the table.

Kill absorption, veins, and events are additive; this column is only the passive core income at that level.

## Milestones (levels 4, 7, 10)

Each milestone grants:

1. **+1 dungeon stratum** — core may plan, power, and breach **one deeper vertical layer** (stacked under the core room; fog-of-war dig extends downward).
2. **+1 core control** — +1 **simultaneous powered biome pocket** the core can keep lit and defended (ley-line state active at once).
3. **+1 boss chamber** — cap on **boss-tier creatures** the core may assign to guarded chambers at once (see table).

Levels between milestones still earn XP and future perks; they do not add strata / control / boss slots unless noted.

## Progression table

| Core level | XP to advance (from previous) | Cumulative XP | Mana per round | Dungeon strata (max depth) | Powered pockets (max) | Boss chambers (max) | Notes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| **1** | — | 0 | 20 | 1 | 1 | 1 | Tutorial band; core room + first capillaries only |
| **2** | 2,000 | 2,000 | 40 | 1 | 1 | 1 | |
| **3** | 4,000 | 6,000 | 60 | 1 | 1 | 1 | |
| **4** | 8,000 | 14,000 | 160 | **2** | **2** | **2** | **Milestone** — second stratum, +1 pocket, +1 boss |
| **5** | 16,000 | 30,000 | 200 | 2 | 2 | 2 | |
| **6** | 32,000 | 62,000 | 240 | 2 | 2 | 2 | |
| **7** | 64,000 | 126,000 | 420 | **3** | **3** | **3** | **Milestone** |
| **8** | 128,000 | 254,000 | 480 | 3 | 3 | 3 | |
| **9** | 256,000 | 510,000 | 540 | 3 | 3 | 3 | |
| **10** | 512,000 | 1,022,000 | 800 | **4** | **4** | **4** | **Milestone** — cap for launch slice |

## Implementation notes

- **Dungeon strata** = vertical build layers the core may energize; breaches from workers can open pockets on any unlocked stratum.
- **Powered pockets** = distinct biome/resident regions that stay **powered** (bright ley grid) without brownouts; dormant pockets beyond the cap stay **dormant** until the player powers down another.
- **Boss chambers** = active **Boss**-tier assignments per [Monsters.md](./Monsters.md); Elites do not count toward this cap.
- **Mana per round** is passive ley drip only; tune kill/vein/event essence separately.
- **Core size** (draft): the core grows physically as it levels, starting small at level 1. Sizes TBD; see [General.md](./General.md) "Grid and scale".
