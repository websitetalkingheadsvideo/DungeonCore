# Paragon heroes — reference data for invaders and Random spawns

Epic released Paragon (2016–2018) hero assets to UE; Dungeon Core uses them as **invader meshes** and, in three families, as **dungeon Random** specials. Paragon was a **third-person MOBA** (5v5, lanes, cards/Power progression). There is **no official Epic “character sheet” PDF**, but the community preserved **per-hero stat and ability pages** that are the right source when tuning combat and AI.

**Runtime invader fields** (caution, trap literacy, party behavior) live in [Invaders.md](./Invaders.md). **Unified combat sheets** (vitals, ratings, compact abilities): [Character-Sheets.md](./Character-Sheets.md). **Asset paths and roles** live in [Monsters.md](./Monsters.md). This doc is the **Paragon-only** cheat sheet: where to look up numbers, how to translate them into Dungeon Core, and roster maps for **assigned** heroes plus the **full owned catalog**.

**Asset scope:** You own the **complete Epic Paragon hero catalog** (all released playable heroes + their packs/skins). Only a **subset is migrated** into this UE project under `/Game/` ([checklist below](#owned-catalog-vs-in-project)). More invader roles, Random specials, or alternate casts can swap in **without new Fab purchases** — migrate the pack when a row gets a design seat.

**Status:** design reference — combat numbers in UE are TBD; values below are **last-live Paragon** (≈2018), not balanced for the dungeon grid yet.

---

## External sources

| Source | URL | Use |
| --- | --- | --- |
| **Paragon Wiki (Fandom)** — heroes index | https://paragon.fandom.com/wiki/Heroes | One page per hero: traits, role bars, base stats + per-level scaling, full ability tables (damage, power scaling, mana, CD, range, CC). |
| **Abilities (system)** | https://paragon.fandom.com/wiki/Abilities | 3 basics + 1 ultimate; ability point cadence by hero level. |
| **Narendur’s hero numbers** | https://docs.google.com/spreadsheets/d/1_J9pKPGp1ddmSpNB_jcFhjjq_OPSILDmjdwzqre3i3U/edit | Community spreadsheet — compare HP/DPS/scaling across all heroes. |
| **Narendur patch v42 sheet** | https://docs.google.com/spreadsheets/d/1Bl28JtM51bWBczbJE2OMoVcK8q5ug4FHXsbQetLUKuA/edit | PTR/datamine snapshot; late balance pass. |
| **Paramine (game file extract)** | https://github.com/Teybeo/Paramine | Machine-readable dumps; treated as accurate vs post-processed API. |
| **ParagonAPI (community)** | https://github.com/rizzn/ParagonAPI | Heroes, abilities, items, roles — JSON/API style. |
| **Into the Valley (archived guides)** | https://intothevalley.se/paragon/ | Build guides and ability writeups (e.g. Sparrow, deck/Power economy). |
| **Paragon bots (Epic GDC-style)** | http://www.gameaipro.com/GameAIPro3/GameAIPro3_Chapter32_Paragon_Bots_A_Bag_of_Tricks.pdf | One parameterized behavior tree for all heroes — useful for **role intent**, not DPS. |

Paragon’s **card deck / Power** loop (≈60 Power, ~6.5 physical or energy damage per point, etc.) is **not** copied into Dungeon Core unless a raid mode explicitly adds it; use wiki ability **base damage and scaling** only as **relative power** between heroes.

---

## Paragon ability model (quick)

| Item | Paragon rule |
| --- | --- |
| Kit | LMB basic, RMB alt, **Q / E / R** (three actives) + **ultimate** (often bound separately). |
| Levels | Each basic: 4 ranks; ultimate: 3 ranks. |
| Learn timing | First point at level 1; ult first at hero level 5 (typical pattern — see wiki). |
| Damage types | **Basic damage** (auto attacks) vs **ability damage** (powers); separate armor types in Paragon. |
| Scaling | Most abilities list **power scaling** multipliers on energy/physical power from cards — strip or remap for dungeon combat. |

---

## Using Paragon data in Dungeon Core

### What to import

| From Paragon wiki | Dungeon Core use |
| --- | --- |
| Role traits (Tank, Carry, Ganker, …) | Maps to `ArchetypeId` + spawn table row. |
| Mobility / durability overview bars (1–10) | Hints for `Caution`, `ScoutRating`, `FollowerWeight` in [Invaders.md](./Invaders.md). |
| Base HP, move speed, basic attack | Starting combat stat band; **must be rescaled** to grid (see below). |
| Signature CC, dash, heal, ult | 1–3 **raid behaviors** (e.g. Greystone stone form = survive burst; Phase blink = ignore some traps once). |
| Low mobility carries (Sparrow) | Higher dependence on party cohesion; educate AI to stay behind tank. |

### What to leave out (unless scoped later)

- Full Q/E/R level tables in the data asset (too heavy for [Invaders.md](./Invaders.md) sheet design).
- Card decks, lifesteal, CDR metas.
- Structure damage and last-hit jungle rules.

### Grid and units

- Paragon move speeds are typically **~655–680** unreal units; ability ranges are often **hundreds–2500+** u.
- Dungeon Core: **1 gameplay block = 10 ft = 300 U** ([project C++ convention](../.cursorrules)).
- **No fixed conversion** is defined yet. When implementing, pick anchor heroes (e.g. Sparrow range vs one room width) and document the formula in the combat spec.

### Random dungeon Paragons (Narbash, Fey, Sevarog)

These are **defenders**, not surface invaders. Use the same wiki kits for **encounter design** (aoe, buff, knockback, fly-up), not invader trap-literacy tables unless a future event raids with them.

---

## Invader roster (Paragon meshes)

Paths from [Monsters.md — Intruders](./Monsters.md#intruders-adventurers). Wiki links are the **canonical deep stat sheet** for each hero.

| DC role | Paragon hero | UE primary path | Paragon wiki | Paragon lane / style (wiki) | Dungeon Core notes |
| --- | --- | --- | --- | --- | --- |
| **Champion** | **Terra** | `/Game/Invaders/ParagonTerra` | [Terra](https://paragon.fandom.com/wiki/Terra) | Fighter / solo; earth shields, sustained frontline | Map to **Hero / Fighter** archetype; high `CohesionBias`, moderate `Caution`. Veteran: `Knows_LeyTraps`. |
| **Tank** | **Greystone** | ParagonGreystone (invaders pack) | [Greystone](https://paragon.fandom.com/wiki/Greystone) | Solo / jungle durable; stone ult, self-heal, split kit | **Tank** defaults in [Invaders.md](./Invaders.md); low `Caution`, high `HelperWeight`. |
| **Bruiser** | **Kwang** | `/Game/Invaders/ParagonKwang` | [Kwang](https://paragon.fandom.com/wiki/Kwang) | Carry / bruiser; sword swing, lane pressure | **Tank / Bruiser** row; aggressive pathing, lower trap avoidance than Mage. |
| **Mage** | **Phase** | ParagonPhase | [Phase](https://paragon.fandom.com/wiki/Phase) | Mid/support; dimensional mobility, poke, team save | **Mage** row; higher `Caution`; candidate for educated `Knows_DisplacementGlyphs` on veterans (blinks). |
| **Mage (alt mesh)** | **Aurora** | `/Game/Invaders/ParagonAurora` | [Aurora](https://paragon.fandom.com/wiki/Aurora) | Mid; ice root, mobility, teamfight ult | Same archetype as Phase; **ice / root** kit → prefer **hold** formation vs scatter. |
| **Archer** | **Sparrow** | `/Game/Invaders/ParagonSparrow` | [Sparrow](https://paragon.fandom.com/wiki/Sparrow) | Safe-lane carry; charged shot, no escape | Low mobility in Paragon → **stay behind tank**, high party cohesion dependency; `ScoutRating` moderate (line of sight, not traps). |
| **Cleric** | **Zinx** | `/Game/Invaders/ParagonZinx` | [Zinx](https://paragon.fandom.com/wiki/Zinx) | Support; sustain, poke (sci-fi default skin — use non-sci-fi skin if fit) | **Cleric** row; highest `HelperWeight` / `CohesionBias`; anchor for reunite-after-glyph behavior. |

**Alternates (non-Paragon):** Warrior Queen (Champion), Paladin RPG / Gothic Knight (Tank/Hero), Firefly party — no Paragon sheet; see [Monsters.md](./Monsters.md).

### Suggested trap-literacy flavor (optional spawn tags)

Not from Paragon canon — **Dungeon Core** hooks only. Tune in data tables.

| Hero | Optional veteran tags | Rationale (kit flavor) |
| --- | --- | --- |
| Phase, Aurora | `Knows_DisplacementGlyphs` | Mobility / blink mindset. |
| Sparrow | — | Stays with party; learns from witnesses like default Mage. |
| Greystone, Terra, Kwang | `Knows_LeyTraps` | Frontliners who’ve survived powered rooms. |
| Zinx | — | Supports learn via party memory; high follow-to-heal. |
| Scout / Assassin (Fab today) | `Knows_MimicRisk` | **Owned pool:** Kallari, Countess, Yin, Revenant — migrate when Assassin seat goes Paragon ([full catalog](#owned-catalog-vs-in-project)). |

---

## Dungeon Random roster (Paragon defenders)

| Family | Hero | UE mesh (see [Monsters.md](./Monsters.md)) | Paragon wiki | Paragon role (wiki) | Encounter notes |
| --- | --- | --- | --- | --- | --- |
| **Orcs** | **Narbash** | `Orcs/ParagonNarbash/.../Narbash` | [Narbash](https://paragon.fandom.com/wiki/Narbash) | Support tank; drums, heals, knockbacks | Full Paragon anim set — no CC skeleton. War drum bones — product call in [TODO.md](../TODO.md). |
| **Evil Fairies** | **The Fey** (Opaline default) | `.../Fey_Opaline` | [The Fey](https://paragon.fandom.com/wiki/The_Fey) | Support zoner; bramble, fly, slow | Skins: Opaline (default), AutumnKeeper, Frostbloom, Nightshade — [Monsters.md](./Monsters.md). |
| **Undead** | **Sevarog** | `Undead/ParagonSevarog/.../Sevarog` | [Sevarog](https://paragon.fandom.com/wiki/Sevarog) | Jungle tank; hammer, soul stacks, **fly** ult | Large silhouette; good **boss-feel** Random despite replacing any tier. |

---

## Stat snapshot template (fill from wiki per hero)

When filling sheets, follow [Character-Sheets.md — Derivation from Paragon](./Character-Sheets.md#derivation-from-paragon). Wiki **Base Stats** at level 1 / 10 / 15 map into the shared struct. Field glossary:

| Field | Paragon meaning | UE field (TBD) |
| --- | --- | --- |
| Base Health (+/level) | EHP pool | `MaxHealth` after rescale |
| Basic / Ability Defense (+/level) | Physical vs ability mitigation | Armor types TBD |
| Basic Attack Damage (+/level) | Auto DPS | `BasicDamage` |
| Attacks Per Second (+/level) | Attack speed | `AttackPeriod` |
| Movement Speed | Locomotion | `MaxWalkSpeed` after rescale |
| Base Mana (+/level) | Only if abilities use mana in DC | Optional |

**Reference comparisons** (level 1, wiki — for tuning ratios only):

| Hero | ~HP | ~Move | ~Basic dmg | Style |
| --- | ---: | ---: | ---: | --- |
| Rampage (wiki example tank) | 713 | 655 | 63.2 | Fat tank baseline in MOBA |
| TwinBlast (ranged carry) | 574 | 665 | 40 | Squishy DPS |
| Countess (assassin) | 566 | 655 | 52.9 | Burst melee |

Use Narendur’s sheet or side-by-side charts for assigned heroes without transcribing every column here.

---

## Owned catalog vs in project

**Rule:** Wiki + spreadsheets cover **all 36** released heroes. **UE paths** exist only after pack migration ([asset-library-first](../.cursor/rules/asset-library-first.mdc) — copy heroes actually used, not the whole vault).

### Currently in `/Game/` (assigned)

| Hero | Dungeon Core seat | Typical mount |
| --- | --- | --- |
| Terra | Invader — Champion | `/Game/Invaders/ParagonTerra` |
| Greystone | Invader — Tank | `/Game/Invaders/ParagonGreystone` |
| Kwang | Invader — Bruiser | `/Game/Invaders/ParagonKwang` |
| Phase | Invader — Mage | `/Game/Invaders/ParagonPhase` |
| Aurora | Invader — Mage (alt mesh) | `/Game/Invaders/ParagonAurora` |
| Sparrow | Invader — Archer | `/Game/Invaders/ParagonSparrow` |
| Zinx | Invader — Cleric | `/Game/Invaders/ParagonZinx` |
| Narbash | Defender — Orcs **Random** | `/Game/Monsters/Orcs/ParagonNarbash/...` |
| The Fey | Defender — Evil Fairies **Random** (Opaline default) | `/Game/Monsters/EvilFairies/ParagonFey/...` |
| Sevarog | Defender — Undead **Random** | `/Game/Monsters/Undead/ParagonSevarog/...` |

**10 heroes** in project today · **26** owned but not migrated (or migrated elsewhere on disk — still no `/Game/` seat).

### Full owned roster (migrate on demand)

Fandom index: https://paragon.fandom.com/wiki/Heroes. **In UE** = migrated into this project. **DC seat** = current design assignment; `—` means no seat yet (casting notes are suggestions only, often from [library.md](../library.md)).

| Hero | In UE | DC seat | Paragon wiki | MoBA role (wiki summary) | Casting notes (optional) |
| --- | ---: | --- | --- | --- | --- |
| Aurora | ✓ | Mage alt | [Aurora](https://paragon.fandom.com/wiki/Aurora) | Mid; ice CC | — |
| Countess | | — | [Countess](https://paragon.fandom.com/wiki/Countess) | Assassin / burst | Invader **Assassin** candidate |
| Crunch | | — | [Crunch](https://paragon.fandom.com/wiki/Crunch) | Jungler; tanky | Brute / elite |
| Dekker | | — | [Dekker](https://paragon.fandom.com/wiki/Dekker) | Support | Sci-fi pump-gun; weak art fit unless skin |
| Drongo | | — | [Drongo](https://paragon.fandom.com/wiki/Drongo) | Carry | Gunner; weak fantasy fit |
| Feng Mao | | — | [Feng Mao](https://paragon.fandom.com/wiki/Feng_Mao) | Solo / jungler | Sparrow-like melee; invader duelist |
| Gadget | | — | [Gadget](https://paragon.fandom.com/wiki/Gadget) | Mid | Sci-fi turret; weak fit |
| Gideon | | — | [Gideon](https://paragon.fandom.com/wiki/Gideon) | Mid mage | Dark caster; mage / boss vibe |
| Greystone | ✓ | Tank | [Greystone](https://paragon.fandom.com/wiki/Greystone) | Solo durable | — |
| GRIM.exe | | — | [GRIM.exe](https://paragon.fandom.com/wiki/GRIM.exe) | Carry | Robot; poor fit |
| Howitzer | | — | [Howitzer](https://paragon.fandom.com/wiki/Howitzer) | Mid | Artillery; poor fit |
| Iggy & Scorch | | — | [Iggy & Scorch](https://paragon.fandom.com/wiki/Iggy_%26_Scorch) | Mid / jungler | Mech; poor fit |
| Kallari | | — | [Kallari](https://paragon.fandom.com/wiki/Kallari) | Assassin / jungler | **Assassin** candidate |
| Khaimera | | — | [Khaimera](https://paragon.fandom.com/wiki/Khaimera) | Jungler | Beast elite / Random |
| Kwang | ✓ | Bruiser | [Kwang](https://paragon.fandom.com/wiki/Kwang) | Bruiser carry | — |
| Lt. Belica | | — | [Lt. Belica](https://paragon.fandom.com/wiki/Lt._Belica) | Mid | Sci-fi cop; poor fit |
| Morigesh | | — | [Morigesh](https://paragon.fandom.com/wiki/Morigesh) | Mid | Witch; fairy / fungal boss alt |
| Murdock | | — | [Murdock](https://paragon.fandom.com/wiki/Murdock) | Carry | Tanky ranged; gunner |
| Muriel | | — | [Muriel](https://paragon.fandom.com/wiki/Muriel) | Support | Angel support; cleric alt |
| Narbash | ✓ | Orc Random | [Narbash](https://paragon.fandom.com/wiki/Narbash) | Support tank | — |
| Phase | ✓ | Mage | [Phase](https://paragon.fandom.com/wiki/Phase) | Support / mid | — |
| Rampage | | — | [Rampage](https://paragon.fandom.com/wiki/Rampage) | Tank / jungler | Big tank; orc boss alt |
| Revenant | | — | [Revenant](https://paragon.fandom.com/wiki/Revenant) | Carry assassin | **Assassin** / ranged carry |
| Riktor | | — | [Riktor](https://paragon.fandom.com/wiki/Riktor) | Solo / jungler | Chain tank |
| Sevarog | ✓ | Undead Random | [Sevarog](https://paragon.fandom.com/wiki/Sevarog) | Jungle tank | — |
| Serath | | — | [Serath](https://paragon.fandom.com/wiki/Serath) | Carry | Angel carry |
| Shinbi | | — | [Shinbi](https://paragon.fandom.com/wiki/Shinbi) | Mid | Wolf rider |
| Sparrow | ✓ | Archer | [Sparrow](https://paragon.fandom.com/wiki/Sparrow) | Carry | — |
| Steel | | — | [Steel](https://paragon.fandom.com/wiki/Steel) | Support / solo | Robot knight; **Tank** alt if retextured |
| Terra | ✓ | Champion | [Terra](https://paragon.fandom.com/wiki/Terra) | Fighter | — |
| The Fey | ✓ | Fairy Random | [The Fey](https://paragon.fandom.com/wiki/The_Fey) | Support zoner | — |
| TwinBlast | | — | [TwinBlast](https://paragon.fandom.com/wiki/TwinBlast) | Carry | Gunner; poor fit |
| Wukong | | — | [Wukong](https://paragon.fandom.com/wiki/Wukong) | Carry / jungler | Melee carry |
| Wraith | | — | [Wraith](https://paragon.fandom.com/wiki/Wraith) | Mid | Sci-fi; poor fit |
| Yin | | — | [Yin](https://paragon.fandom.com/wiki/Yin) | Carry | Duelist; assassin-adjacent |
| Zinx | ✓ | Cleric | [Zinx](https://paragon.fandom.com/wiki/Zinx) | Support | — |

**Open (from [TODO.md](../TODO.md)):** Paragon **skin policy** — fixed vs random roll at spawn; migrate packs only when a hero gets a seat. **Migration** — track intended `/Game/Invaders/Paragon*` vs `/Game/Monsters/.../Paragon*` paths in [Monsters.md](./Monsters.md) when a new row is assigned.

---

## Cross-links

- Invader AI and sheet schema: [Invaders.md](./Invaders.md)
- Intruder table and Random tier: [Monsters.md](./Monsters.md)
- Character powers backlog: [TODO.md](../TODO.md) — “Design abilities for every creature family” and invader roles