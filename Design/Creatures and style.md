# Environment and Enemy Matrix

Biome (skin) and creature family are **independent choices**. The player can pair any biome with any creature family for variety and replayability. The **core room** uses the selected skin.

## Lighting rule

Every skin has its own lighting palette, but all skins share a **dramatic** treatment: strong key light, high contrast, deep shadows.

## Biomes

| Biome | Visual aesthetic | Strategic layout impact | Natural-fit family (flavor only) |
| --- | --- | --- | --- |
| **Gothic** | Ruined cathedral tiles and pillars; warm oil-lamp gold against ink-black shadows | Line-of-sight choke points: rigid grids, heavy doors, long corridors—straight-line Prism Beam setups | Undead |
| **Goblin Tunnels** | Narrow paths in raw basalt; timber supports vs unstable crystal veins; torch amber | Melee bottlenecks: winding corridors, short sightlines—AoE stuns and physical staggers over geometry traps | Orcs |
| **Fungal Warren** | Organic caverns; bioluminescent spores and glowing webs that slow movement; spore green | Slowing kill zones: wide irregular rooms—chemical traps from flora; hard to align mirror/beam traps | Spiders, Fungal |
| **Sunken Ruins** | Flooded aqueducts and crystal chambers; teal through waterlogged masonry | Terrain hazards: water splits tiles—energy bridges or gravity drops into channels | Evil Fairies |

Natural fit is a suggestion, not a lock. Creature families: [Monsters.md](./Monsters.md).

Concrete mesh and material paths for each theme: [Theme.md](./Theme.md). Showcase targets that are not all in the project yet: [New Game.md](./New%20Game.md).

## Core loop: encountering factions

Expansion outward from the safe core room moves through these beats:

1. **Blind dig** — Worker shatters a basalt wall block into fog of war.
2. **Unintended breach** — Worker breaks into a pocket of another biome; floor/wall tiles **morph** to that biome's kit.
3. **Territorial defense** — The pocket's resident creature family tries to follow breach lines up the capillaries to destroy the geode core.

Genre framing and progression hooks: [Overall.md](./Overall.md).
