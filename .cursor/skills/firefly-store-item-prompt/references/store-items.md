# Store items — prompt anchors

Source table: `Design/Store.md`. Behavior/look: `Design/General.md` (traps, constructs). Icon basenames are fixed for UMG (`/Game/UI/Icons/`).

| # | Item | Icon basename | Slug | Accent (glow) | Silhouette / must-read |
| ---: | --- | --- | --- | --- | --- |
| 0 | Monster Generator | `monster_generator` | `monster-generator` | Warm amber / gold | Ornate **metal cage or summoning pavilion** on stone dais; interior glow (creature silhouette OK); reads as **spawn structure**, not generic chest |
| 1 | Mana Generator | `mana_generator` | `mana-generator` | Amber-gold | **Crystal cluster** rising from **square basalt pedestal** with corner brackets; mana wells up through crystals |
| 2 | Prism Beam Trap | `prism_beam_trap` | `prism-beam-trap` | Teal / sapphire | **Gem node** on short pillar or pintle; hint of **faceted mirror** or prism beside it; beam trap, not a turret blob |
| 3 | Resonance Plate Trap | `resonance_plate_trap` | `resonance-plate-trap` | Cool blue harmonic | **Flush floor plate** — quartz tile in metal frame; **concentric rings** or vibration lines on top; low profile |
| 4 | Gravity Well Trap | `gravity_well_trap` | `gravity-well-trap` | Violet (not Will wisp) | **Ceiling-hung geode** or inverted crystal mass with metal harness; reads upward pull; avoid cute purple fairy |
| 5 | Runaway Cart Trap | `runaway_cart_trap` | `runaway-cart-trap` | Orange / rust spark | **Mine cart** on **short rail segment**; chunky wheels, Goblin-mine mood; motion implied, track visible |
| 6 | Loot Bait | `loot_bait` | `loot-bait` | Soft gold glint | **Treasure lure** — exposed coins/gems on **pedestal or skull platter** with ley-line crystal accent; readable as bait, not hero loot pile |
| 7 | Unshackled Golem | `unshackled_golem` | `unshackled-golem` | Cyan ley core | **Floating obsidian shards** orbiting a **bright ley-energy node**; rigid mechanical facets, no face |
| 8 | Shard Sentinel | `shard_sentinel` | `shard-sentinel` | White / pale prism | **Small darting prism** or wisp bundle; **no face**; searchlight cone or lens flare OK; **not violet**, not slow facet shimmer (reserved for Will) |
| 9 | Spore Generator (trap) | `spore_generator` | `spore-generator` | Pale spore green / cream bioluminescence | **Neutral fungal plant:** round **cap** on thick **stalk** rooted to floor (not floating); faint **spore cloud** around cap; reads as sleep/gas **trap**, not mushroom light like mana shell |

## Files

| Purpose | Path pattern |
| --- | --- |
| Firefly / Hi3D source (canonical mesh ref) | `Design/Reference Images/Dungeon Items/<slug>-vN.png` |
| HUD drag icon (48×48 source) | `Design/Reference Images/icons/<icon-basename>-vN.png` |
| Hi3D output | `imports/hi3d/<slug>/` |

Never overwrite an existing path; increment version.

## Rendering (all items)

Every Firefly prompt includes the **realism block** and **required closer** from [realism.md](realism.md). Accent glow stays **one hue** per row above.

## Shared negatives (Firefly)

**Composition:** dungeon corridor, characters, hands, UI frames, price tags, mana numbers, diorama base, floor plane, vignette, multiple unrelated props.

**Style:** cartoon, chibi, cel shaded, mobile-game toon, clip art, smooth plastic, oversaturated neon, stylized illustration, cute.

**Dressing:** muddy grime piles, torches-only horror, chains without structure, oversized room furniture.