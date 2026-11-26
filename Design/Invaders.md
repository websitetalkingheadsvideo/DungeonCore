# Invaders — parties, AI, and character sheets

Surface **adventurer parties** that raid the dungeon. Visual roster and Fab picks: [Monsters.md](./Monsters.md#intruders-adventurers). Paragon MOBA reference (stats, wiki links, roster map): [Invaders-Paragons.md](./Invaders-Paragons.md). Store items that change invader behavior: [Store.md](./Store.md). This doc is the **single place** for raid AI, party state, and per-invader data the simulation needs.

**Status:** Raid AI, spawn tables, and runtime character sheets are **not** implemented in UE yet. **Meshes:** Common-line **Firefly party** + Fab knights + partial **Paragon** packs live under `/Game/Invaders/` ([In UE](#in-ue-gameinvaders)). Role → path for spawn wiring remains in [Monsters.md — Intruders](./Monsters.md#intruders-adventurers).

## In UE (`/Game/Invaders/`)

Primary **skeletal mesh** (or static mesh) per custom / Fab adventurer. `—` = not imported. Paragon paths also summarized in [Invaders-Paragons.md](./Invaders-Paragons.md#invader-roster-paragon-meshes). Pipeline: Hi3D / Firefly → CC → AccuRIG → UE unless noted.

### Firefly party (Common line)

Shared kit: charcoal wool, ivory linen, dark leather, brushed steel, muted navy trim ([Monsters.md](./Monsters.md#intruders-adventurers)). CC idle: Open A → `Motion/*_Anim_0_Open_A_UE5`; rig under each folder’s `Post_Process/`.

| Design role | Archetype (draft) | UE mesh | 2D ref / CC source |
| --- | --- | --- | --- |
| **Dwarf Fighter** | Fighter (+ tank bias in `ScoutDuo`) | `Dwarf/Dwarf_Fighter` | [Invader-Dward_v2.png](./Reference%20Images/Invaders/Invader-Dward_v2.png); `imports/hi3d/Invaders/Invader-Dwarf.ccProject` |
| **Elf Bowman** | Scout / Archer | `Elf-Archer/Elf_Archer` | [Invader-Elf.png](./Reference%20Images/Invaders/Invader-Elf.png); `Invader-Elf-Archer.ccProject` |
| **Old Wizard** (**Thelma**) | Mage | `Thelma/Invader_Thelma` | Firefly grey-robe wizard ref; `Invader_Thelma` FBX / CC export |
| **Elf Mage** | Mage (party `GlassCannon` / `FrontBack`) | — | Pipeline WIP: `imports/hi3d/Invaders/Invader-Elf-Mage.*` — **not** in Content yet (archer is separate mesh) |
| **Female Cleric** (optional party slot) | Cleric | — | Paragon **Zinx** or future Firefly cleric — see below |

### Fab / CC humanoids (Common line)

| Design role | UE asset | Notes |
| --- | --- | --- |
| **Paladin** (RPG set) | `Paladin/Paladin` | CC humanoid; retarget idles from `GKnight` (`Retargeted_Anims/anim_Idle_SK_GothicKnight_VA_Paladin`). Tank / Cleric flavor for templates — assign in spawn table. |
| **Gothic Knight** | `GKnight/Meshes/SK_GothicKnight_VA` … `VF` | Fab modular set (body/head/cape/parts under `Meshes/Parts/`). Default spawn: **VA** unless template picks variant. Bruiser / Tank alt per [Monsters.md](./Monsters.md#intruders-adventurers). |
| **Hero** | `HeroicWarrior/SM_HeroicWarrior` | **Static mesh only** — no skeleton / ABP yet ([TODO.md](../TODO.md)). |

### Paragon packs (migrated subset)

Mounted under `Invaders/Paragon*` (hero mesh + Epic anim sets). Use for **Paragon line** raids; same trap/party AI as Common.

| Design role | Folder | Primary mesh |
| --- | --- | --- |
| Champion | `ParagonTerra` | `Terra` |
| Tank | `ParagonGreystone` | `Greystone` |
| Bruiser | `ParagonKwang` | `Kwang` |
| Mage | `ParagonPhase` | `Phase` |
| Mage (alt) | `ParagonAurora` | `Aurora` |
| Archer | `ParagonSparrow` | `Sparrow` |
| Cleric | `ParagonZinx` | `Zinx` |
| — (extra migrated) | `ParagonGideon` | `Gideon` — no DC role row yet; candidate Bruiser/Mage |

**Not in project:** Fab **Wanderer**, **Assassin**, **Free Medieval Assassin Girl** — still mount TBD ([Monsters.md](./Monsters.md#intruders-adventurers)).

### Composition hooks (Firefly + Fab)

| Template | Suggested Common meshes (when rigged) |
| --- | --- |
| `ScoutDuo` | `Dwarf_Fighter` + `Elf_Archer` |
| `FrontBack` | `Dwarf_Fighter` or `Paladin` + `Invader_Thelma` or (later) `Elf_Mage` |
| `Classic` | Tank slot: `Paladin` or `SK_GothicKnight_VA` + damage: `Elf_Archer` + support: `Invader_Thelma` or `Zinx` |

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
| **Raid wave** | One surface incursion: one `PartyId`, spawns near the dungeon mouth. Size and power depend on **invader line** ([below](#raid-scaling-core-level)). |
| **Invader line** | `Common` (Fab / custom — Wanderer, Hero, Assassin, Firefly party) vs `Paragon` (Epic MOBA heroes). Same party AI; very different combat sheets. |
| **Paragon-caliber** | Paragon line only: full MOBA-style kit on the sheet ([Character-Sheets.md](./Character-Sheets.md)); **not** full wiki level-15 numbers until the `ReferenceTier` band allows it. |

## Raid scaling (core level)

Surface response scales with [Dungeon Core Levels.md](./Dungeon%20Core%20Levels.md). Two **spawn lines** share trap / party AI ([party state](#party-state-runtime-not-on-sheet)) but differ in **party size rules** and **power**.

| Line | Meshes | Power (vs Paragon at same core) | Party size |
| --- | --- | --- | --- |
| **Common** | Non-Paragon intruders ([Monsters.md](./Monsters.md#intruders-adventurers)) | **Much lower** — `Fodder` stats, thin kits (see [Character-Sheets.md](./Character-Sheets.md)) | **Parties from core 1** (2–3 typical; solo Wanderer OK) |
| **Paragon** | Paragon invaders | Full kits; numbers ramp by core | **Solo → duo → trio** by core level |

### Common line (Fab / custom)

- **From core level 1:** raids may be **parties of 2 or 3** using Common meshes (e.g. Wanderer + Hero, or full **Firefly party** when rigged). Solo **Wanderer** remains valid cheap fodder.
- **Party AI always on** when size > 1: `PartyId`, cohesion, witness learning, glyph reunite — same as Paragon groups.
- **Power:** always treat as **well below** a Paragon hero. Default `ReferenceTier` = `Fodder`; apply `CommonPowerScalar` **≈ 0.4** on vitals and damage vs a Paragon-derived sheet at the same tier (tune in combat). **One** simple active or none; passives optional — not full MOBA kits.
- **Frequency:** Common waves are the **bread and butter** (often, forgiving). They teach traps and party behavior before Paragon champions show up.

### Paragon line — party size

| Core level | Paragon raiders per wave | Party AI (size 1) |
| --- | ---: | --- |
| **1–4** | **1** (solo) | Unit-only reactions (no shared memory). |
| **5–7** | **2** | Full party AI. |
| **8–10** | **3** | Full party AI; displacement glyph spec assumes **two witnesses** (see below). |

Launch slice caps Paragon at **three** per wave (no 4–5 stacks). Larger warbands are future / reputation events.

### Paragon line — reference tier per core band

Combat stats: [Character-Sheets.md — Reference tier](./Character-Sheets.md#reference-tier). Paragon meshes use full kits; **numbers** ramp so core 1–2 is not fighting wiki level-15 veterans.

| Core level | Default `ReferenceTier` | Notes |
| --- | --- | --- |
| **1–2** | `Fodder` → `Standard` | Rare **solo** Paragon scout / champion; most early surface traffic is **Common parties**. |
| **3–4** | `Standard` | Paragon solos more common; still one body per Paragon wave. |
| **5–7** | `Standard` → `Veteran` | Paragon duos; at **7**, one slot may roll `Veteran`. |
| **8–10** | `Veteran` | Paragon trios; support slot may stay `Standard` for balance. |

Optional runtime `SpawnPowerScalar` (0.85–1.15) stacks on top for depth / Randomness — not a substitute for tier bands or `CommonPowerScalar`.

### Composition templates (spawn table)

Random mesh triples are forbidden. Each wave picks from **templates** (line-specific meshes):

| Size | Templates (pick one) | Roles |
| --- | --- | --- |
| **2** | `FrontBack` | Tank or Bruiser + Mage, Archer, or Cleric |
| **2** | `ScoutDuo` | Scout/Assassin + Fighter (Firefly dwarf + elf fits here) |
| **3** | `Classic` | Tank + damage (Hero/Bruiser/Mage) + Cleric or Scout |
| **3** | `GlassCannon` | Tank + Mage + Archer (high trap payoff if split) |

- **Common:** templates use Fab / Firefly archetypes only; sizes **2–3** from core 1.
- **Paragon:** same templates but Paragon meshes per [Invaders-Paragons.md](./Invaders-Paragons.md); size capped by core level table above.

### First raid vs ongoing spawns

| Raid # | `InvaderLine` | Rule |
| --- | --- | --- |
| **1st wave ever** (this save) | **Common only** | **Hard rule — never Paragon.** Scripted or table-forced: e.g. **2** `Fodder` units, naive trap knowledge. Sets player expectation that Fab parties come first; Paragon is “real heroes” later. |
| **2nd wave onward** | **Random** | Roll `Common` vs `Paragon` from spawn weights (below). Still obey **party size** rules per line and core level. |

Runtime flag: `bFirstSurfaceRaidConsumed` (or raid count ≥ 1) gates the guarantee. Tutorial does not spawn surface invaders; this applies to **first organic lure** after [Tutorial.md](./Tutorial.md) hand-off.

### Spawn economy (draft)

- **Line roll (after 1st wave):** weighted random each wave — draft **60% Common / 40% Paragon** at low core, shifting toward **40 / 60** by core 8–10 (tune). Reputation may bias later.
- **Common parties:** lower essence per kill than Paragon; good for [adventurer paradox](./Overall.md) traffic without wipes.
- **Paragon waves:** **fewer, heavier** as party size grows — do not triple Paragon spawn rate when duos/trios unlock; bump reward per cleared Paragon wave slightly.
- Failed raids that retreat: partial XP (both lines).
- **Reputation** (TBD): too lethal → longer gaps; too easy → Common spam — separate knob from line weights.

### Tutorial / first lure

[Tutorial.md](./Tutorial.md) defers **lure adventurers** to free play. The **first** surface raid must match [First raid vs ongoing spawns](#first-raid-vs-ongoing-spawns) (**Common** duo, no Paragon). From the **second** raid on, line is random. Paragon **duos** only when core **≥ 5**; Paragon **trios** when core **≥ 8**.

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

### Identity and combat

| Field | Notes |
| --- | --- |
| `ArchetypeId` | Wanderer, Tank, Cleric, Scout, Mage, etc. ([Monsters.md](./Monsters.md#intruders-adventurers)) |
| `InvaderLine` | `Common` \| `Paragon` — party-size rules and power scalar ([raid scaling](#raid-scaling-core-level)) |
| `Mesh / rig` | Asset path — out of scope here |
| Combat stats, ratings, abilities | Shared schema: [Character-Sheets.md](./Character-Sheets.md) (`EntityKind = Invader` + rows below) |

### Behavior (required for trap reactions)

| Field | Type | Default source | Purpose |
| --- | --- | --- | --- |
| `Caution` | 0–1 | Archetype | Base willingness to avoid unknown floor tiles and chests. |
| `CohesionBias` | −1…+1 | Archetype | Added to party cohesion when choosing follow vs press on. |
| `FollowerWeight` | 0–1 | Archetype | Chance to step on **enter** glyph after ally vanishes (before knowledge). |
| `HelperWeight` | 0–1 | Archetype | Mimic stick: help held ally vs continue. |
| `ScoutRating` | 0–1 | Archetype | Spot Mirage Veil / mimic tell before contact (player-only tells still hidden). |
| `LootGreed` | 0–1 | Archetype | Baseline lure toward **Loot Bait**, visible treasure, idol pickups — independent of trap caution. |

**Loot pull (runtime):** per-unit draw toward loot objectives uses **`LootGreed`** (sheet) × **`Party.TrustLoot`** (party state, default **1.0** until mimic bite or similar lowers it). **`Caution`** and trap-literacy tags affect **floor/trap** choices separately — a unit can be cautious (0.6) and still greedy (0.5) like an “Indy” hero: thinks about traps, still takes the idol when temptation wins. Flagship bait (**snare → bait**, idol vault) picks the pathing unit with highest effective pull unless AI overrides (scout hangs back, tank ordered forward).

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

| Archetype | Caution | CohesionBias | FollowerWeight | HelperWeight | ScoutRating | LootGreed | Typical starting tags |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| Tank / Bruiser | 0.3 | +0.3 | 0.55 | 0.7 | 0.2 | 0.35 | `Knows_LeyTraps` (veterans only) |
| Cleric | 0.4 | +0.5 | 0.65 | 0.85 | 0.3 | 0.25 | — |
| Fighter / Hero | 0.35 | +0.2 | 0.45 | 0.6 | 0.25 | **0.50** | — |
| Scout / Assassin | 0.75 | −0.2 | 0.15 | 0.35 | 0.85 | 0.30 | `Knows_MimicRisk` (weak) |
| Mage | 0.6 | −0.1 | 0.25 | 0.4 | 0.4 | 0.30 | — |
| Wanderer (fodder) | 0.2 | 0 | 0.35 | 0.45 | 0.15 | 0.65 | none |

**Firefly party archetypes** (meshes in UE — see [In UE](#in-ue-gameinvaders)): `Dwarf_Fighter` → Fighter (tank-leaning); `Elf_Archer` → Scout / Archer; `Invader_Thelma` → Mage. **Elf Mage** when imported → Mage. **Paladin** / **Gothic Knight** → Tank or Bruiser per spawn template. **Zinx** → Cleric when Paragon line or mixed party.

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
3. **Witnessed units** → short halt (~1s reaction anim / bark).
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
| **Loot Bait** | — | Highest-`LootGreed` units lead toward bait | Slower approach when `Party.TrustLoot` low; scouts may refuse |
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
- [ ] Raid spawner: force Common on raid #1; then weighted `InvaderLine` roll; `PartySize` (Common 2–3 from L1; Paragon 1/2/3 by core); composition template; `ReferenceTier`; `CommonPowerScalar`; `bFirstSurfaceRaidConsumed`
- [ ] Scout party event → seed `LayoutIntel` + tags on **next** raid only
- [ ] Telemetry / bark hooks for player readability (optional but recommended)

## Open questions

- Exact cohesion formula and whether the player sees party tier names (LitRPG flavor).
- Per-faction tag sets (dwarves +glyph lore?) vs only veteran tier.
- Whether **Call** ping reveals exit room through fog to allies (strong) or only direction (weaker).
- Raid spawn cadence vs essence income (pair with reputation when it exists).

## Cross-links

- Combat sheet schema (shared with monsters): [Character-Sheets.md](./Character-Sheets.md)
- Paragon reference: [Invaders-Paragons.md](./Invaders-Paragons.md)
- **UE asset paths (this doc):** [In UE](#in-ue-gameinvaders)
- Role → path summary: [Monsters.md — Intruders](./Monsters.md#intruders-adventurers)
- Trap catalog: [Store.md](./Store.md), [store_items.md](./store_items.md)
- Scout intel event: [Random Events.md](./Random%20Events.md)
- Genre balance (bait vs lethality): [Overall.md](./Overall.md)
- Core level XP curve & milestones: [Dungeon Core Levels.md](./Dungeon%20Core%20Levels.md)