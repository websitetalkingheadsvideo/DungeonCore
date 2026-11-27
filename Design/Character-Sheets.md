# Character sheets — defenders and invaders

Single schema for **dungeon monsters** (by family + tier) and **surface invaders** (by archetype). Paragon MOBA wiki stats are the **reference ruler** when a mesh is a Paragon hero; Fab/Hi3D creatures use the same fields with **tier baselines** instead.

**Trap / party behavior** (invaders only): still defined in [Invaders.md](./Invaders.md) — caution, cohesion, trap-literacy tags. This doc is **combat, ratings, and compact abilities**.

**Paragon lookup:** [Invaders-Paragons.md](./Invaders-Paragons.md) · **Meshes / tiers:** [Monsters.md](./Monsters.md)

**Status:** design — no `UDataAsset` in UE yet. Values are **targets** until combat ships.

## On this page

| I need… | Section |
| --- | --- |
| **Archetype → two skills (Common names)** | [Invader archetypes — summary](#invader-archetypes--two-skills-summary) (below) |
| Same + effect tags & Paragon rules | [Invader archetype kits (2 actives)](#invader-archetype-kits-2-actives) |
| **Paragon hero → wiki skill names** | [Invaders-Paragons.md — Paragon two-actives map](./Invaders-Paragons.md#paragon-two-actives-map-wiki-names) |
| Thief lockpick / climb walls | [Expedition gear](#expedition-gear-invaders) |
| Vitals, ratings, derivation | [Combat vitals](#combat-vitals) · [Derivation from Paragon](#derivation-from-paragon) |
| **JSON (schema + examples)** | [character-sheets.json](../Data/character-sheets.json) · [character-sheet.schema.json](../Data/character-sheet.schema.json) |
| **UE C++ (3-layer system)** | [character-sheet-ue-system.md](../tmp/character-sheet-ue-system.md) |

---

## Invader archetypes — two skills (summary)

Each invader `ArchetypeId` gets **Primary** + **Secondary** only (plus basic attack on vitals). **Common / Fab** rows use these display names; **Paragon** rows substitute wiki names from [Invaders-Paragons.md](./Invaders-Paragons.md).

| ArchetypeId | Primary | Secondary |
| --- | --- | --- |
| Wanderer | Opportunist strike | Short sprint |
| Thief | Lockpick | Smoke vanish |
| Assassin | Ambush / execute | Blink or vanish step |
| Hero | Power cleave | Battle shout |
| Champion | Charged slam | Zone control (slow/root) |
| Tank | Guard / damage soak taunt | Shield slam |
| Bruiser | Heavy cleave | Bull rush |
| Fighter | Combo strike | Brace |
| Mage | Ranged burst | Blink or reposition |
| Archer | Piercing shot | Volley or marked target |
| Cleric | Heal pulse | Ward or cleanse |
| Scout | Trap probe / reveal edge | Sprint |

Effect tags and tuning rules: [Invader archetype kits (2 actives)](#invader-archetype-kits-2-actives).

---

## Design goals

| Goal | Choice |
| --- | --- |
| Paragon-like | Keep **dual mitigation** (basic vs ability), **basic attack + cooldown kit**, **four overview ratings** (1–10). |
| Smaller than Paragon | **No** card/Power deck, **no** per-match hero levels, **no** full Q/E/R rank tables on the sheet. |
| One asset shape | Same struct for every unit; **`ArchetypeId` required on every sheet** (defender or invader). Family/tier or invader line add fields on top. |
| Grid fight | One **reference power tier** per sheet row (see [Reference tier](#reference-tier)); scaling from raids / depth is a **spawn modifier**, not 15 MOBA levels. |
| Invader kit size | Every **`ArchetypeId`** row ships **exactly two actives** (`Primary` + `Secondary`); see [Invader archetype kits](#invader-archetype-kits-2-actives). |

---

## Sheet identity

| Field | Type | Monsters | Invaders |
| --- | --- | --- | --- |
| `SheetId` | name | e.g. `Orcs.Elite` | e.g. `Invader.Tank` |
| `EntityKind` | enum | `Defender` | `Invader` |
| `FamilyId` | enum | Orcs, Undead, … | — |
| `Tier` | enum | Worker, Minion, Elite, Boss, Hero | — |
| `ArchetypeId` | enum | **Required** — combat role + two-actives template ([summary](#invader-archetypes--two-skills-summary)) | **Required** — same enum |
| `DisplayName` | text | Player-facing name | Party-facing name |
| `MeshRef` | soft path | From [Monsters.md](./Monsters.md) | From intruder table |
| `ParagonRef` | optional name | e.g. `Sevarog` for Hero | e.g. `Greystone` |
| `InvaderLine` | enum | — | `Common` \| `Paragon` (surface raids only) |
| `StatSource` | enum | `ParagonWiki` \| `TierBaseline` \| `HandTuned` | Same |

When `ParagonRef` is set, **overview ratings** and **vitals** are **derived** from wiki level-1 or level-15 stats (see [Derivation](#derivation-from-paragon)), then tier/family multipliers apply for defenders.

### Defenders — picking `ArchetypeId`

`Tier` sets power; **`ArchetypeId` sets the kit shape** (same table as invaders). Default mapping when authoring a family row:

| `Tier` | Default `ArchetypeId` | Notes |
| --- | --- | --- |
| Worker | **Fighter** | Often `BasicDamage = 0`; abilities optional (dig/build tag). |
| Minion | **Fighter** | Ranged families → **Archer** or **Mage** per [Monsters.md](./Monsters.md). |
| Elite | **Bruiser** | Fungal / fairy casters → **Mage**. |
| Boss | **Tank** or **Bruiser** | Large melee bosses → **Tank**. |
| Hero | **Explicit** | Paragon Hero: match kit (e.g. Sevarog → **Tank**, The Fey → **Mage**, Narbash → **Cleric**). |

JSON defaults: [defenderArchetypeDefaultsByTier](../Data/character-sheets.json) in [character-sheets.json](../Data/character-sheets.json).

---

## Reference tier

Dungeon combat assumes units fight at a single logical tier (no mid-raid leveling in v1):

| `ReferenceTier` | Use when |
| --- | --- |
| `Fodder` | Minions, Wanderers — derive from Paragon **level 1** or tier baseline. |
| `Standard` | Elite invaders, dungeon Elites — Paragon **~level 9–11** equivalent or hand-tuned. |
| `Veteran` | Champion raids, Bosses, Paragon Hero specials — Paragon **level 15** equivalent. |

Spawn code may multiply vitals by `SpawnPowerScalar` (0.85–1.15) from raid depth or Randomness; that multiplier is **runtime**, not stored on the base sheet.

**Surface raids:** **Common** vs **Paragon** lines, party size, and default `ReferenceTier` — see [Invaders.md — Raid scaling](./Invaders.md#raid-scaling-core-level). Common invaders use `Fodder` + `CommonPowerScalar` (~0.4 vs Paragon); Paragon uses core band table.

---

## Overview ratings (Paragon-style, 1–10)

Same meaning as [Paragon wiki](https://paragon.fandom.com/wiki/Heroes) overview bars. Integers; used for UI chips, quick AI weighting, and balance grep.

| Field | Meaning |
| --- | --- |
| `Rating_BasicDamage` | Auto-attack threat |
| `Rating_AbilityDamage` | Cooldown / kit burst |
| `Rating_Durability` | Effective HP + mitigation |
| `Rating_Mobility` | Reposition, chase, escape |

**Copy rule:** If `ParagonRef` is set, take wiki **Base Stats Overview** verbatim when possible. If no Paragon (Fab spider, zombie), set from [Tier defaults](#tier-defaults-non-paragon) and tune once in play.

---

## Combat vitals

All units:

| Field | Type | Notes |
| --- | --- | --- |
| `MaxHealth` | float | Post-rescale HP pool |
| `HealthRegenPerSec` | float | Often `0` for monsters; low for support invaders later |
| `MoveSpeed` | float | **UU/s** — rescale from Paragon (~660) to grid (see [Unit rescale](#unit-rescale)) |

Offense:

| Field | Type | Notes |
| --- | --- | --- |
| `BasicDamage` | float | Damage per basic attack hit |
| `AttackPeriod` | float | Seconds between basic attacks (`1 / APS` from Paragon) |
| `CleavePct` | float | 0–1; fraction of basic damage to secondary targets in melee arc (Paragon often 0.1–0.2) |
| `BasicAffinity` | enum | `Physical` \| `Energy` — which defense applies to autos |

Mitigation (trimmed Paragon):

| Field | Type | Notes |
| --- | --- | --- |
| `BasicDefense` | float | Reduces **basic** and physical-style trap hits |
| `AbilityDefense` | float | Reduces **ability** damage and magical trap hits |

**Mitigation formula (v1 target):** flat reduction then clamp, e.g. `DamageTaken = Raw * (100 / (100 + Defense))` — exact curve in combat code; sheet only stores `BasicDefense` / `AbilityDefense`.

**No mana on sheet (v1):** abilities use **cooldowns only**. Add `MaxEnergy` / `EnergyRegen` later if kits need it.

---

## Compact abilities (not full Paragon tables)

Paragon heroes have four leveled actives plus passives. Dungeon Core trims that for grid raids.

| `EntityKind` | `Abilities[]` rule |
| --- | --- |
| **Invader** | **Always two actives** — `Slot = Primary` and `Slot = Secondary` on the **archetype template** ([table below](#invader-archetype-kits-2-actives)). No passive or ultimate on invader sheets in v1 (ratings + autos carry flavor). |
| **Defender** | **Worker:** optional non-combat tag only. **Minion:** 0–1 active. **Elite+:** up to **two** actives; **Boss / Hero** tiers may add a third or passive when [TODO](../TODO.md) character powers ships. |

Each entry in `Abilities[]`:

| Field | Type | Notes |
| --- | --- | --- |
| `AbilityId` | name | Stable id, e.g. `Greystone.Purge` |
| `Slot` | enum | `Passive`, `Primary`, `Secondary`, `Ultimate` |
| `DisplayName` | text | From Paragon wiki or design |
| `DamageType` | enum | `None`, `Basic`, `Ability` |
| `BaseDamage` | float | At `ReferenceTier`; 0 for pure CC |
| `CooldownSec` | float | |
| `RangeUU` | float | 0 = self / melee |
| `EffectTags` | tag set | See [Effect tags](#effect-tags) |
| `ParagonAbilityKey` | optional | Wiki ability name for traceability |

**Design rule:** Fill **names, cooldowns, ranges, tags, and rough damage** from wiki when `ParagonRef` is set; leave fine tuning to playtests. **Invaders:** pick the **two** wiki abilities that best match the archetype template (rename for display if the mesh is a different hero). **Defenders (non-Paragon):** one or two actives by tier above.

### Invader archetype kits (2 actives)

Canonical pair per `ArchetypeId` ([Monsters.md — Intruders](./Monsters.md#intruders-adventurers)). Spawn rows **must** include both slots; mesh-only roles (e.g. **Dwarf Fighter**, **Wizard**) inherit the matching archetype row.

| ArchetypeId | Primary (`Slot`) | Secondary (`Slot`) | `EffectTags` (typical) |
| --- | --- | --- | --- |
| **Wanderer** | Opportunist strike | Short sprint | `Melee`, `Dash` |
| **Thief** | Lockpick | Smoke vanish | `PickLock`, `Blink` |
| **Assassin** | Ambush / execute | Blink or vanish step | `Melee`, `Execute`, `Blink` |
| **Hero** | Power cleave | Battle shout (self buff) | `Melee`, `AOE`, `BuffAura` |
| **Champion** | Charged slam | Zone control (slow/root) | `Melee`, `AOE`, `Slow` |
| **Tank** | Guard / damage soak taunt | Shield slam | `Melee`, `AOE` |
| **Bruiser** | Heavy cleave | Bull rush | `Melee`, `Dash`, `Cleave` |
| **Fighter** | Combo strike | Brace (damage reduction) | `Melee` |
| **Mage** | Ranged burst | Blink or reposition | `Ranged`, `Ability`, `Blink` |
| **Archer** | Piercing shot | Volley or marked target | `Ranged`, `Line`, `AOE` |
| **Cleric** | Heal pulse | Ward or cleanse | `Heal`, `Shield` |
| **Scout** | Trap probe / reveal edge | Sprint | `Ranged`, `Dash` |

**Paragon line:** same **two slots** — copy damage/cooldown/range from the mapped hero’s wiki kit ([Invaders-Paragons.md — Paragon two-actives map](./Invaders-Paragons.md#paragon-two-actives-map-wiki-names)), not the full Q/E/R/LMB table. Example **Tank** (Greystone): **Make Way** + **Assault the Gates** — no ult on sheet.

**Common line:** same two abilities; scale `BaseDamage` / `CooldownSec` with `CommonPowerScalar` (~0.4); keep tags so trap AI still reads the kit.

### Effect tags

| Tag | Combat / AI hook |
| --- | --- |
| `Melee` / `Ranged` | Attack range bucket |
| `Dash`, `Blink` | Ignores some floor hazards once |
| `Root`, `Slow`, `Stun` | CC duration lives in ability tuning data |
| `Heal`, `Shield` | Support invaders / Narbash-style Hero |
| `AOE`, `Cone`, `Line` | Trap dodge weighting |
| `Summon`, `BuffAura` | Minion family elites |
| `Execute` | Finisher vs low-HP defenders |
| `PickLock` | Opens **locked** chests, doors, vaults without brute force; longer channel than tanks |

Counters (traps / monsters beat them) hang off these tags in the powers TODO — not duplicated on the sheet.

---

## Tier defaults (non-Paragon)

Fab/Hi3D creatures without `ParagonRef` start from **family Elite = 1.0** vitals, then:

| Tier | `MaxHealth` mult | `BasicDamage` mult | `AbilityDamage` mult | `Rating_*` bias |
| --- | ---: | ---: | ---: | --- |
| **Worker** | 0.25 | 0.15 | 0 | Durability −3, Mobility −1 |
| **Minion** | 0.45 | 0.4 | 0.5 | Basic +0, Durability −2 |
| **Elite** | 1.0 | 1.0 | 1.0 | Baseline |
| **Boss** | 3.0 | 1.4 | 1.6 | Durability +3, Basic +2 |
| **Hero** | 1.15 | 1.1 | 1.2 | Use Paragon row when applicable |

Workers may set `BasicDamage = 0` and omit combat `Abilities[]` except a dig/build tag.

**Hero + Paragon:** Ignore minion tier multipliers; use **Veteran** derivation from that hero’s wiki stats, then optional family flavor (e.g. Sevarog slightly tankier than Countess).

---

## Derivation from Paragon

1. Pick wiki hero matching `ParagonRef` ([Invaders-Paragons.md](./Invaders-Paragons.md)).
2. Read **Base Stats** at the level band for `ReferenceTier`:
   - `Fodder` → level **1**
   - `Standard` → level **10** (interpolate between 1 and 15)
   - `Veteran` → level **15**
3. Map fields:

| Paragon wiki | Sheet field |
| --- | --- |
| Health | `MaxHealth` × `HealthScale` |
| Health Regen | `HealthRegenPerSec` × `HealthScale` |
| Basic Attack Damage | `BasicDamage` × `DamageScale` |
| Attacks Per Second | `AttackPeriod = 1 / APS` |
| Basic Defense | `BasicDefense` × `DefenseScale` |
| Ability Defense | `AbilityDefense` × `DefenseScale` |
| Movement Speed | `MoveSpeed` × `MoveScale` |
| Overview bars | `Rating_*` |

4. **Invaders:** copy the **two** wiki actives that match the archetype template ([kits table](#invader-archetype-kits-2-actives)). **Defenders (Boss / Hero with `ParagonRef`):** up to three entries + optional passive/ult until powers v1 locks minion/elite at two.

**Global scalars (initial — tune in combat prototype):**

| Scalar | Suggested start | Notes |
| --- | ---: | --- |
| `HealthScale` | 0.12 | Paragon 500–700 HP → dungeon 60–85 range at Veteran |
| `DamageScale` | 0.18 | Keeps TTK in few seconds vs Standard minions |
| `DefenseScale` | 1.0 | Keep Paragon **ratios** between tanks and carries |
| `MoveScale` | 0.45 | Paragon 660 → ~300 U/s (one block per second) |

Anchor once: **Sparrow** ranged basic at Standard vs **one room width** ([Invaders-Paragons.md](./Invaders-Paragons.md)); adjust `MoveScale` / `RangeUU` together.

---

## Invader extension (behavior)

Same combat block as above, plus **unchanged** invader-only fields from [Invaders.md](./Invaders.md):

- `Caution`, `CohesionBias`, `FollowerWeight`, `HelperWeight`, `ScoutRating`, `LootGreed`
- `StartingKnowledgeTags[]`
- Expedition gear ([below](#expedition-gear-invaders)) — rope, climb, flight for **pit and vertical** encounters

Those floats are **not** on defender sheets unless a Hero-tier Paragon also raids (not planned).

**Party-only (not on sheet):** `TrustLoot` — see [Invaders.md — Party state](./Invaders.md#party-state-runtime-not-on-sheet). Loot pathing uses `LootGreed × TrustLoot` at runtime.

### Expedition gear (invaders)

Fixed kit for raid AI (not loot inventory in v1). One **10 ft** rope = **300 UU** (one gameplay cube) — enough for a standard pit depth in [todo/ideas.md](../todo/ideas.md).

| Field | Type | Notes |
| --- | --- | --- |
| `HasRope` | bool | Enables **rope rescue** action at pit edge; takes time (haul / secure), not instant. |
| `RopeLengthUU` | float | Default **300** when `HasRope`; longer rope = rare veteran / event override later. |
| `HasLockpick` | bool | Enables **lockpick** on flagged locks ([Store.md](./Store.md) locked chests, vault doors); pairs with **Lockpick** active or shorter channel. |
| `CanClimbWalls` | bool | Self-extract from climbable pit / vertical shaft without ally rope (**Thief** default). |
| `HasSustainedFlight` | bool | From kit tag (e.g. fly ultimate as sustained mobility); self-extract or assist rescue; still costs time. |

**Archetype defaults (targets):**

| Archetype | `HasRope` | `RopeLengthUU` | `HasLockpick` | `CanClimbWalls` | `HasSustainedFlight` |
| --- | ---: | ---: | ---: | ---: | ---: |
| **Thief** | false | 0 | **true** | **true** | false |
| Scout | true | 300 | false | false | false |
| Assassin | false | 0 | false | true | false (unless kit grants) |
| Fighter / Hero / Tank / Bruiser | false | 0 | false | false | false (unless kit grants) |
| Cleric / Mage / Archer / Wanderer | false | 0 | false | false | false |
| Paragon with fly ult | — | optional rope per role | false typical | **true** when ult tagged sustained |

Derive `HasSustainedFlight` from `Abilities[].Tags` containing `SustainedFly` when combat powers ship; until then, set by hand on Paragon rows that raid with flight.

### Archetype → combat sketch (invaders)

Default `ReferenceTier` and Paragon anchor for **first pass** (mesh picks in [Monsters.md](./Monsters.md)):

| Archetype | ReferenceTier | Paragon anchor (stats) | Rating skew |
| --- | --- | --- | --- |
| Wanderer | Fodder | Murdock or generic carry L1 | Low all |
| Thief | Standard | — (Fab) | Mobility +2, Durability −3, high `LootGreed` in [Invaders.md](./Invaders.md) |
| Assassin | Standard | Kallari / Countess | Mobility +2, Durability −2 |
| Hero / Champion | Standard–Veteran | Terra, Kwang | Basic +1, Durability +1 |
| Tank | Veteran | Greystone, Rampage | Durability max |
| Bruiser | Standard | Kwang | Basic +2 |
| Mage | Standard | Gideon / Phase | Ability +2, Basic −1 |
| Archer | Standard | Sparrow | Basic +2, Mobility −1 |
| Cleric | Standard | Muriel / Zinx | Ability +1, Durability −1 |
| Scout | Standard | — (Fab) | Mobility +2, `ScoutRating` high in Invaders.md |

**Common line:** `InvaderLine = Common`, `ReferenceTier = Fodder`, vitals × `CommonPowerScalar` (~0.4) vs Paragon anchor at same tier; **same two actives** as the archetype template, weaker numbers. **Paragon line:** derive vitals from wiki per [Invaders.md](./Invaders.md) core band; **still two actives** from the template + hero mapping.

---

## Defender extension (dungeon AI)

Optional fields on `EntityKind == Defender`:

| Field | Type | Notes |
| --- | --- | --- |
| `WorkerRole` | enum | `None`, `Dig`, `Build`, `Both` |
| `AggroStyle` | enum | `Guard`, `Chase`, `Stationary` |
| `LeashRadiusUU` | float | Max chase from spawn |
| `PreferredRange` | enum | `Melee`, `Short`, `Long` |
| `UsesParagonAnimSet` | bool | true → no CC skeleton retarget |

---

## Examples (filled shape)

### `Orcs.Elite` (Fab, no Paragon)

| Field | Value |
| --- | --- |
| ReferenceTier | Standard |
| ParagonRef | — |
| MaxHealth | 72 |
| BasicDefense / AbilityDefense | 18 / 14 |
| BasicDamage | 11 |
| AttackPeriod | 0.95 |
| MoveSpeed | 280 |
| Rating_Basic / Ability / Durability / Mobility | 5 / 2 / 5 / 4 |
| Abilities | *(TODO powers)* Primary: cleave sweep · Secondary: enrage (self `BuffAura`) |

### `Undead.Hero` (Sevarog)

| Field | Value |
| --- | --- |
| ParagonRef | Sevarog |
| ReferenceTier | Veteran |
| StatSource | ParagonWiki @ 15 + Hero mult |
| Ratings | From wiki |
| Vitals | Derived + scalars |
| Abilities | **Siphon** (Primary) · **Subjugate** (Secondary) — tags `AOE`, `Root`, `Slow` |

### `Invader.Tank` (Greystone)

| Field | Value |
| --- | --- |
| ArchetypeId | Tank |
| ParagonRef | Greystone |
| ReferenceTier | Veteran |
| Vitals | Derived |
| Abilities | **Make Way** (Primary) · **Assault the Gates** (Secondary) |
| Invader behavior | [Invaders.md](./Invaders.md) Tank row |

---

## Implementation checklist

- [x] `UDungeonCharacterSheetData` + `UCharacterSheetSubsystem` + runtime components — see [tmp/character-sheet-ue-system.md](../tmp/character-sheet-ue-system.md)
- [ ] `UDungeonMonsterDefinition` : sheet + `FamilyId` + `Tier`
- [ ] `UInvaderDefinition` : sheet + behavior block from Invaders.md
- [ ] One CSV/DT per family tier + invader archetype; Paragon rows automated from wiki template later
- [ ] Combat component reads vitals; GAS or simple cooldown map for `Abilities[]`
- [ ] Document chosen scalars in C++ or `DefaultGame.ini` after anchor test

---

## Cross-links

- Paragon wiki map & owned roster: [Invaders-Paragons.md](./Invaders-Paragons.md)
- Invader trap AI: [Invaders.md](./Invaders.md)
- Tier grid & paths: [Monsters.md](./Monsters.md)
- Powers backlog: [TODO.md](../TODO.md)