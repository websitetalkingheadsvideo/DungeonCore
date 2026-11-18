# Invaders — parties, AI, and character sheets

Surface **adventurer parties** that raid the dungeon. Visual roster and Fab picks: [Monsters.md](./Monsters.md#intruders-adventurers). Store items that change invader behavior: [Store.md](./Store.md). This doc is the **single place** for raid AI, party state, and per-invader data the simulation needs.

**Status:** design only — no invader party system or character sheets are implemented in UE yet.

## Goals

- Invaders **react** to traps and each other (split, reunite, avoid, gamble), not walk through teleports mindlessly.
- Reactions scale with **knowledge** (per unit + shared party memory), not only random rolls.
- Character sheets stay **small**: a few tags and floats, not a full LitRPG stat block.
- One implementation path for all store-driven invader interactions (glyphs, mimics, veils, bait, spores, etc.).

## Terms

| Term | Meaning |
| --- | --- |
| **Party** | One raid group sharing `PartyId`, objective, and party memory for this run. |
| **Unit / invader** | One adventurer instance with a role archetype and character sheet. |
| **Knowledge tag** | Boolean or tiered fact about dungeon mechanics (e.g. knows displacement pairs). |
| **Trap literacy** | Set of knowledge tags relevant to traps and dungeon fixtures. |
| **Witnessed** | Unit had line-of-sight (or was within N cells) when the event fired. |

## Party state (runtime, not on sheet)

| Field | Type | Purpose |
| --- | --- | --- |
| `PartyId` | id | Links units; memory is per party per run. |
| `Objective` | enum | Default: reach core. Overrides: bait room, decoy core, flee. |
| `Cohesion` | 0–1 | How hard the party tries to stay together. Recomputed on damage, ally down, cleric present, progress. |
| `Formation` | enum | Column, scatter, hold — affects who steps on floor traps first. |
| `PartyMemory` | set of tags | Facts the whole party shares after anyone learns them this run (see below). |
| `TrustLoot` | 0–1 | Mimic / bait wariness after a bad chest (see [Store.md — Mimic](./Store.md#mimic)). |
| `LayoutIntel` | optional map | Filled by [Scout party](./Random%20Events.md) event or veteran spawn flag. |

## Character sheet (per invader)

These fields belong on the **data asset or spawn table row** for each intruder archetype, plus per-instance overrides when spawning a named party.

### Identity and combat (existing / TBD)

| Field | Notes |
| --- | --- |
| `ArchetypeId` | Wanderer, Tank, Cleric, Scout, Mage, etc. ([Monsters.md](./Monsters.md#intruders-adventurers)) |
| `Mesh / rig` | Asset path — out of scope here |
| Combat stats | HP, armor, DPS — TBD in combat doc |

### Behavior (required for trap reactions)

| Field | Type | Default source | Purpose |
| --- | --- | --- | --- |
| `Caution` | 0–1 | Archetype | Base willingness to avoid unknown floor tiles and chests. |
| `CohesionBias` | −1…+1 | Archetype | Added to party cohesion when choosing follow vs press on. |
| `FollowerWeight` | 0–1 | Archetype | Chance to step on **enter** glyph after ally vanishes (before knowledge). |
| `HelperWeight` | 0–1 | Archetype | Mimic stick: help held ally vs continue. |
| `ScoutRating` | 0–1 | Archetype | Spot Mirage Veil / mimic tell before contact (player-only tells still hidden). |

### Trap literacy (knowledge tags on sheet)

**Spawn tags** — granted when the unit spawns (veteran raids, role, or event):

| Tag | Meaning |
| --- | --- |
| `Knows_LeyTraps` | Generic: powered rooms may contain floor traps; slow approach. |
| `Knows_DisplacementGlyphs` | Enter/exit pairs; do not chain step enter; reunite via doors or ping. |
| `Knows_MirageVeil` | Fake walls; scout checks edges. |
| `Knows_MimicRisk` | Chests may be mimics; lower `TrustLoot` baseline. |
| `Knows_SporeSleep` | Spore rooms = sleep; avoid or prepare. |
| `Knows_DecoyCore` | Cores can be false objectives (after one decoy seen). |

**Party memory tags** — same string ids; copied into `PartyMemory` when any member **witnesses** the lesson (see [Learning](#learning-witnessing-and-events)).

Sheets list **starting tags** only. Runtime union = `Unit.StartingTags ∪ Party.PartyMemory`.

### Archetype defaults (placeholder tuning)

| Archetype | Caution | CohesionBias | FollowerWeight | HelperWeight | ScoutRating | Typical starting tags |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Tank / Bruiser | 0.3 | +0.3 | 0.55 | 0.7 | 0.2 | `Knows_LeyTraps` (veterans only) |
| Cleric | 0.4 | +0.5 | 0.65 | 0.85 | 0.3 | — |
| Fighter / Hero | 0.35 | +0.2 | 0.45 | 0.6 | 0.25 | — |
| Scout / Assassin | 0.75 | −0.2 | 0.15 | 0.35 | 0.85 | `Knows_MimicRisk` (weak) |
| Mage | 0.6 | −0.1 | 0.25 | 0.4 | 0.4 | — |
| Wanderer (fodder) | 0.2 | 0 | 0.35 | 0.45 | 0.15 | none |

Custom Firefly party ([Monsters.md](./Monsters.md)): assign archetypes (Dwarf → Tank, Cleric → Cleric, Elf → Scout, Wizard → Mage).

## Learning, witnessing, and events

| Source | Effect |
| --- | --- |
| **Witness** ally teleported, mimic bite, veil revealed, spore sleep | Add matching tag to `PartyMemory` if not already present. |
| **Scout party** ([Random Events](./Random%20Events.md)) | Next raid: `LayoutIntel` + `Knows_DisplacementGlyphs` and `Knows_MirageVeil` on party spawn (if those fixtures were mapped). |
| **Veteran spawn tier** | Champion+ raids: start with `Knows_LeyTraps`; small chance extra glyph/veil tags. |
| **Failed raid survived** | Next party from same faction: +1 tag the survivors reported (meta TBD). |

## Reaction pipeline (all traps)

When a trap or fixture fires:

1. **Apply effect** to victim (teleport, stick, slow, sleep, …).
2. **Emit party event** (`AllyTeleported`, `AllyStuck`, `TrapTriggered`, …) with location, trap type, witness list.
3. **Witnessed units** →短 halt (~1s reaction anim / bark).
4. **Choose intent** per non-victim unit (priority order):
   - If `PartyMemory` has relevant **Knows_*** → use **educated** branch (table below).
   - Else roll from archetype weights + `Cohesion` + `CohesionBias`.
5. **Teleported / stuck ally** → `Call` ping so allies path toward **ally room**, not blindly onto trap tile (unless educated follow with known safe exit).

No unit should auto-path onto **enter** in the same frame as the victim; minimum one reaction beat.

## Store items → invader behavior

| Store item | Knowledge tag | Naive behavior | Educated behavior (`Knows_*`) |
| --- | --- | --- | --- |
| **Displacement Glyph** | `Knows_DisplacementGlyphs` | Halt; high `FollowerWeight` ally may step **enter**; other holds or wanders | **Avoid enter**; path to reunite via doors toward ping / exit room |
| **Mirage Veil** | `Knows_MirageVeil` | Treat fake edge as wall | Scout checks; party ignores after one reveal |
| **Mimic** | `Knows_MimicRisk` + `TrustLoot` | Open bait like Loot Bait | Slow open; scouts may refuse; after bite, `TrustLoot` drops |
| **Loot Bait** | — | Draw toward room | Same; educated: slightly slower if `TrustLoot` low |
| **Drag Lattice** | `Knows_LeyTraps` | Walk through, slower | Optional: path around if alternate route known (`LayoutIntel`) |
| **Spore Generator** | `Knows_SporeSleep` | Enter room, fall asleep | Avoid room or send one expendable first |
| **Crystal Barricade** | — | Blocked path; break or reroute | — |
| **Decoy Core** | `Knows_DecoyCore` | Treat as core objective | After first decoy touched, ignore dim copies |
| Damage traps (beam, plate, grav, cart) | `Knows_LeyTraps` | Trigger like fodder | Slightly wider berth when pathing (TBD) |

Detailed spec for displacement (reference for other traps):

### Displacement Glyph — party of three

**Victim** on **enter** → appears on **exit**; disorient ~0.5s; then fight/flee; **Call** ping.

**Two witnesses in entry room:**

- Without `Knows_DisplacementGlyphs`: cohesion + `FollowerWeight` → at most **one** follower attempts **enter** within ~3s; third often holds or takes alternate route.
- With tag: **no** follower uses **enter**; both path toward victim via **normal connectivity** toward exit room / ping.

**Exit in another room, no witness:** search current room briefly, then resume objective unless ping received.

## What to build (engineering checklist)

- [ ] `UInvaderDefinition` or data table: archetype defaults + sheet fields above
- [ ] `FInvaderParty` runtime: cohesion, memory, trust
- [ ] Party event bus from trap / fixture actors
- [ ] Intent switch: `ContinueObjective | FollowViaEnter | ReuniteViaPath | HelpAlly | AvoidTrap | Search`
- [ ] Tag query: `HasKnowledge(Unit, Party, TagId)`
- [ ] Spawn tables: map [Monsters.md](./Monsters.md) roles → archetype row
- [ ] Scout party event → seed `LayoutIntel` + tags on **next** raid only
- [ ] Telemetry / bark hooks for player readability (optional but recommended)

## Open questions

- Exact cohesion formula and whether the player sees party tier names (LitRPG flavor).
- Per-faction tag sets (dwarves +glyph lore?) vs only veteran tier.
- Whether **Call** ping reveals exit room through fog to allies (strong) or only direction (weaker).
- Sync with tutorial first adventurer beat ([Tutorial.md](./Tutorial.md)) — naive party guaranteed.

## Cross-links

- Assets: [Monsters.md — Intruders](./Monsters.md#intruders-adventurers)
- Trap catalog: [Store.md](./Store.md), [store_items.md](./store_items.md)
- Scout intel event: [Random Events.md](./Random%20Events.md)
- Genre balance (bait vs lethality): [Overall.md](./Overall.md)