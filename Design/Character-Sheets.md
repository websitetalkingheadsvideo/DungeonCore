# Character sheets — defenders and invaders

Single schema for **dungeon monsters** (by family + tier) and **surface invaders** (by archetype). Paragon MOBA wiki stats are the **reference ruler** when a mesh is a Paragon hero; Fab/Hi3D creatures use the same fields with **tier baselines** instead.

**Trap / party behavior** (invaders only): still defined in [Invaders.md](./Invaders.md) — caution, cohesion, trap-literacy tags. This doc is **combat, ratings, and compact abilities**.

**Paragon lookup:** [Invaders-Paragons.md](./Invaders-Paragons.md) · **Meshes / tiers:** [Monsters.md](./Monsters.md)

**Status:** design — no `UDataAsset` in UE yet. Values are **targets** until combat ships.

---

## Design goals

| Goal | Choice |
| --- | --- |
| Paragon-like | Keep **dual mitigation** (basic vs ability), **basic attack + cooldown kit**, **four overview ratings** (1–10). |
| Smaller than Paragon | **No** card/Power deck, **no** per-match hero levels, **no** full Q/E/R rank tables on the sheet. |
| One asset shape | Same struct for Worker through Boss and for Wanderer through Champion; extra fields gated by `EntityKind`. |
| Grid fight | One **reference power tier** per sheet row (see [Reference tier](#reference-tier)); scaling from raids / depth is a **spawn modifier**, not 15 MOBA levels. |

---

## Sheet identity

| Field | Type | Monsters | Invaders |
| --- | --- | --- | --- |
| `SheetId` | name | e.g. `Orcs.Elite` | e.g. `Invader.Tank` |
| `EntityKind` | enum | `Defender` | `Invader` |
| `FamilyId` | enum | Orcs, Undead, … | — |
| `Tier` | enum | Worker, Minion, Elite, Boss, Random | — |
| `ArchetypeId` | enum | — | Wanderer, Tank, Mage, … |
| `DisplayName` | text | Player-facing name | Party-facing name |
| `MeshRef` | soft path | From [Monsters.md](./Monsters.md) | From intruder table |
| `ParagonRef` | optional name | e.g. `Sevarog` for Random | e.g. `Greystone` |
| `InvaderLine` | enum | — | `Common` \| `Paragon` (surface raids only) |
| `StatSource` | enum | `ParagonWiki` \| `TierBaseline` \| `HandTuned` | Same |

When `ParagonRef` is set, **overview ratings** and **vitals** are **derived** from wiki level-1 or level-15 stats (see [Derivation](#derivation-from-paragon)), then tier/family multipliers apply for defenders.

---

## Reference tier

Dungeon combat assumes units fight at a single logical tier (no mid-raid leveling in v1):

| `ReferenceTier` | Use when |
| --- | --- |
| `Fodder` | Minions, Wanderers — derive from Paragon **level 1** or tier baseline. |
| `Standard` | Elite invaders, dungeon Elites — Paragon **~level 9–11** equivalent or hand-tuned. |
| `Veteran` | Champion raids, Bosses, Paragon Random specials — Paragon **level 15** equivalent. |

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

Paragon heroes have four leveled actives plus passives. Dungeon Core stores **at most three logical abilities + one optional ultimate** per sheet — enough for combat AI and trap counters ([TODO](../TODO.md) character powers).

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

**Design rule:** Fill **names, cooldowns, ranges, tags, and rough damage** from wiki when `ParagonRef` is set; leave fine tuning to playtests. Non-Paragon families get **one passive + one active** until [TODO](../TODO.md) powers pass fills the rest.

### Effect tags

| Tag | Combat / AI hook |
| --- | --- |
| `Melee` / `Ranged` | Attack range bucket |
| `Dash`, `Blink` | Ignores some floor hazards once |
| `Root`, `Slow`, `Stun` | CC duration lives in ability tuning data |
| `Heal`, `Shield` | Support invaders / Narbash-style Random |
| `AOE`, `Cone`, `Line` | Trap dodge weighting |
| `Summon`, `BuffAura` | Minion family elites |
| `Execute` | Finisher vs low-HP defenders |

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
| **Random** | 1.15 | 1.1 | 1.2 | Use Paragon row when applicable |

Workers may set `BasicDamage = 0` and omit combat `Abilities[]` except a dig/build tag.

**Random + Paragon:** Ignore minion tier multipliers; use **Veteran** derivation from that hero’s wiki stats, then optional family flavor (e.g. Sevarog slightly tankier than Countess).

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

4. Copy **top 3 combat-relevant abilities + ult** (or passive that defines the kit) into `Abilities[]` with wiki damage at the same level band.

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

- `Caution`, `CohesionBias`, `FollowerWeight`, `HelperWeight`, `ScoutRating`
- `StartingKnowledgeTags[]`

Those floats are **not** on defender sheets unless a Random Paragon also raids (not planned).

### Archetype → combat sketch (invaders)

Default `ReferenceTier` and Paragon anchor for **first pass** (mesh picks in [Monsters.md](./Monsters.md)):

| Archetype | ReferenceTier | Paragon anchor (stats) | Rating skew |
| --- | --- | --- | --- |
| Wanderer | Fodder | Murdock or generic carry L1 | Low all |
| Assassin | Standard | Kallari / Countess | Mobility +2, Durability −2 |
| Hero / Champion | Standard–Veteran | Terra, Kwang | Basic +1, Durability +1 |
| Tank | Veteran | Greystone, Rampage | Durability max |
| Bruiser | Standard | Kwang | Basic +2 |
| Mage | Standard | Gideon / Phase | Ability +2, Basic −1 |
| Archer | Standard | Sparrow | Basic +2, Mobility −1 |
| Cleric | Standard | Muriel / Zinx | Ability +1, Durability −1 |
| Scout | Standard | — (Fab) | Mobility +2, `ScoutRating` high in Invaders.md |

**Common line:** `InvaderLine = Common`, `ReferenceTier = Fodder`, vitals × `CommonPowerScalar` (~0.4) vs Paragon anchor at same tier; thin `Abilities[]`. **Paragon line:** derive from wiki per [Invaders.md](./Invaders.md) core band.

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
| Abilities | *(TODO powers)* one `Cleave`-tag melee active |

### `Undead.Random` (Sevarog)

| Field | Value |
| --- | --- |
| ParagonRef | Sevarog |
| ReferenceTier | Veteran |
| StatSource | ParagonWiki @ 15 + Random mult |
| Ratings | From wiki |
| Vitals | Derived + scalars |
| Abilities | Hammer slam, Soul stacks passive, Fly ult — tags `AOE`, `Dash` |

### `Invader.Tank` (Greystone)

| Field | Value |
| --- | --- |
| ArchetypeId | Tank |
| ParagonRef | Greystone |
| ReferenceTier | Veteran |
| Vitals | Derived |
| Invader behavior | [Invaders.md](./Invaders.md) Tank row |

---

## Implementation checklist

- [ ] `UCharacterSheetData` or row struct mirroring tables above
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