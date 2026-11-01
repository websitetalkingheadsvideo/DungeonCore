# Random Events

Unplanned moments during play—good and bad—that keep a run from feeling scripted. Set on the [New Game](./New%20Game.md) card.

## Setting

**Random Events: Low / Medium / High** (default **Medium**). Saved with the game.

- The setting controls **how often** events happen, not which ones. Good and bad scale together.
- **Low** = fewer ups and downs (calmer, more predictable run). **High** = more swings both ways.

| Setting | Rough frequency (placeholder) |
| --- | --- |
| Low | ~1 event per 20 min |
| Medium | ~1 event per 10 min |
| High | ~1 event per 5 min |

## Rules

- **Not during the tutorial.** No events fire until the [Tutorial](./Tutorial.md) ends or is skipped.
- **Not during core defense.** Events wait until a fight ends.
- **One at a time.** No stacking; a short cooldown follows each event.
- **No streaks.** Never more than two bad events in a row; the next one is good or neutral.
- **Never fatal.** A bad event can hurt, but can't destroy the core on its own.
- **Scales with the dungeon.** Rewards and threats grow with dungeon size, so late events still matter.
- **Biome-aware.** Some events only happen in certain biomes (see below).

## Delivery

- [Will](./Will.md) announces each event with one telemetry ping: **what** happened, **why it matters**, **where** (camera can jump to it).
- Bad events with a fuse (e.g. a monster pack approaching) get a short warning first, using Will's scout sense one room ahead.
- Choice events open a small slate (no modal; game keeps running or auto-pauses briefly—TBD).

## Event pool (first pass)

### Boons (good)

| Event | Effect |
| --- | --- |
| **Wandering worker tribe** | A small group of workers from a family joins the dungeon for free |
| **Free mana stone** | A mana stone appears in fog; dig to it to claim a generator (ties to [Future.md](./Future.md) mana generators) |
| **Ore vein** | Next dig hits a vein: essence burst or rare material (ties to Future.md ore veins) |
| **Ley surge** | Essence income boosted for a few minutes |
| **Stray creature** | A lone creature wanders in and can be absorbed for a new pattern |
| **Abandoned cache** | Old loot found in fog; usable as invader bait |

### Banes (bad)

| Event | Effect |
| --- | --- |
| **Cave-in** | A tunnel section collapses; must be re-dug |
| **Wandering monster pack** | Hostile pack enters from fog and heads for the core (warning first) |
| **Ley drought** | Essence income reduced for a few minutes |
| **Scout party** | Invader scouts map part of the dungeon; the next raid knows the layout |
| **Unstable vein** | A crystal vein in a wall overloads; white-hot zone state until fixed |

### Choices (mixed)

| Event | Choice |
| --- | --- |
| **Strange relic** | Absorb for a big essence gain, but it draws stronger invaders |
| **Lost traveler** | Let them go (reputation up) or absorb them (essence, reputation down) |
| **Rival echo** | A faint ley voice offers a trade: territory now for a debt later |

### Biome-specific

| Biome | Event |
| --- | --- |
| Gothic | **Restless dead**: a crypt wall cracks; undead wander out (fight or absorb) |
| Goblin Tunnels | **Old mine shaft**: a rail line found in fog (ties to Future.md rails) |
| Fungal Warren | **Spore bloom**: spores spread; slows everyone in affected rooms for a while |
| Sunken Ruins | **Flood surge**: water rises in low rooms; some tiles cut off temporarily |

## Open questions

- Frequency numbers are placeholders; tune in playtest.
- Do choice events pause the game?
- Do events scale with the reputation / danger meter too, or only dungeon size?
- Are some events one-time per run (e.g. Rival echo)?
