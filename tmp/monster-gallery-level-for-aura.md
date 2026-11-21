# Aura Plan prompt — Monster tier gallery level (Dungeon Core)

Copy everything below the line into **Aura → Plan mode**. Attach `@Design/Monsters.md` (or paste the mesh table). Optional context: `L_NewGame` creature row pattern (`NewGame > Creatures`), `BP_DungeonGrid` `TextRenderActor` label pattern in TryPlace.

---

Plan a new Unreal level that displays every dungeon-family creature from the **In UE** grid in [Monsters.md](Design/Monsters.md), sorted by family and tier.

## Mode

**Plan only first** — no edits until the plan is approved. Output must include:

1. Proposed level asset name + `/Game/Maps/` (or project map) path
2. Outliner hierarchy (folders per row / per family)
3. Layout diagram (ASCII or mermaid): 3 rows × 2 families per row; within each family, 5 slots left→right: Worker, Minion, Elite, Boss, Hero
4. Per-creature spawn spec: full `/Game/Monsters/...` skeletal mesh path, actor class choice (`SkeletalMeshActor` vs packaged `BP_` if one exists), idle animation approach (Single Node vs ABP — match `L_NewGame` `elite_orc` pattern where applicable)
5. Label implementation: `TextRenderActor` at feet vs `WidgetComponent`; text content; size; yaw (billboard to fixed camera vs world-flat on floor)
6. Spacing math: use mesh bounds so creatures do not overlap; note scale normalization if heights differ wildly (spiders vs Paragon heroes)
7. Lighting + floor: minimal playable gallery (not menu UI) — suggest floor mesh/material consistent with Crystalline Ley-Line art pillar ([General.md](Design/General.md) — clean obsidian floor, readable labels)
8. Step-by-step build order + PIE validation checklist
9. **Out of scope** and **open decisions** (call out anything you cannot infer)

## Layout (default unless human overrides)

**3 rows, 2 families per row** (left family | right family):

| Row | Left family | Right family |
| --- | --- | --- |
| 1 | **Orcs** | **Evil Fairies** |
| 2 | **Fungal** | **Undead** |
| 3 | **Mutants** | **Spiders** |

Within each family block, place **exactly five positions** in tier order:

**Worker → Minion → Elite → Boss → Hero** (left to right along +X or as you choose with consistent row convention).

Add a **prominent family title** above or behind each family block (e.g. large `Orcs`, `Evil Fairies`) — separate from per-creature tier labels.

## Per-creature foot label

- Each spawned character gets a **label at the ground in front of their feet** (positive forward from actor facing, on floor plane).
- Label text = **tier name only**: `Worker`, `Minion`, `Elite`, `Boss`, `Hero` (not family name in the foot label).
- Readable at default editor camera; reuse project conventions from store item labels if applicable (`TextRender`, world size ~40, center alignment — see `BP_DungeonGrid` TryPlace).

## Canonical meshes (from Monsters.md — use these paths)

Prefix all with `/Game/Monsters/`.

### Orcs
| Tier | Asset |
| --- | --- |
| Worker | `Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion` |
| Minion | `Orcs/MetaGoblin/MHC_MetaGoblin` |
| Elite | `Orcs/elite_orc` |
| Boss | `Orcs/OrcBoss/Orc_Boss_unreal` |
| Hero | `Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash` |

### Evil Fairies
| Tier | Asset |
| --- | --- |
| Worker | `EvilFairies/dark_fairy_worker_1` |
| Minion | `EvilFairies/dark_fairy_Minion_v1` |
| Elite | `EvilFairies/dark_fairy_elite_v1` |
| Boss | `EvilFairies/dark_fairy_boss_v1` |
| Hero | `EvilFairies/ParagonFey/Characters/Heroes/Fey/Skins/Opaline/Meshes/Fey_Opaline` |

### Fungal
| Tier | Asset |
| --- | --- |
| Worker | `Fungal/Fungal_Worker_v2` |
| Minion | `Fungal/Fungal_Minion` |
| Elite | `Fungal/SK_FungiInfestedKnight` |
| Boss | `Fungal/SK_FungalMyconidGolem` |
| Hero | `Fungal/FungalQueen/Fungal_Queen` |

### Undead
| Tier | Asset |
| --- | --- |
| Worker | `Undead/Zombie` |
| Minion | `Undead/skeleton_v1` |
| Elite | `Undead/mummy_v1` |
| Boss | `Undead/litch_v1` |
| Hero | `Undead/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog` |

### Spiders
| Tier | Asset |
| --- | --- |
| Worker | `Spiders/Argiope_v3` |
| Minion | `Spiders/Blue_Tarantula_v3` |
| Elite | `Spiders/Karakurt` |
| Boss | `Spiders/Tarantula` |
| Hero | `Spiders/Peacock` |

### Mutants
| Tier | Asset |
| --- | --- |
| Worker | `Mutants/mutant_worker_v2` |
| Minion | `Mutants/Character_001/Meshes/Monster_001` |
| Elite | `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster` |
| Boss | `Mutants/Hollow_Walker/Meshy_AI_The_Hollow_Walker_All_Animations` |
| Hero | **— none in project** |

**Mutants Hero:** **Rampage** — `/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage`.

## Engineering constraints

- **Do not edit** `Design/*.md`.
- Prefer **level-placed actors** (like `L_NewGame` showcase) unless a small data-driven spawner clearly reduces maintenance; if DataTable-driven, still keep one level asset.
- Failed mesh load: **log error**, no silent cube substitute.
- **Exceptions:** Paragon heroes and spiders may need per-asset idle anim; Hollow Walker uses embedded Meshy anims; Narbash = full Paragon set (no CC skeleton forcing).
- Grid scale reference: 1 gameplay block ≈ 300 UU unless a file says otherwise.
- Do not add Third Person template content.

## Acceptance criteria

1. New level opens in editor; 6 families visible in 3×2 layout.
2. Each placed asset matches Monsters.md table (29 skeletal placements + Mutants Hero handling).
3. Every creature has foot tier label (`Worker`…`Hero`).
4. Families identifiable (title or grouping in Outliner).
5. PIE: all meshes render; missing asset logs error in Output Log.
6. Level is not wired to New Game flow unless plan explicitly adds a debug map entry.

## References to inspect in project

- `L_NewGame` — `NewGame > Creatures` showcase actors, `elite_orc` Single Node idle
- [Design/Monsters.md](Design/Monsters.md) — full grid
- [Design/New Game.md](Design/New%20Game.md) — prior showcase mesh picks (may differ from full tier gallery; **this level shows all tiers**)

## Level asset (confirmed)

`L_MonsterTierGallery` → `/Game/Maps/Dev/L_MonsterTierGallery`.

## Row pairing (confirmed)

Row 1: Orcs | Evil Fairies · Row 2: Fungal | Undead · Row 3: Mutants | Spiders.

---

**Still for Aura to propose in the plan** (not blocking):

- Exact spacing between families and between rows (bounds-based)
- Default actor yaw (suggest all face a fixed overview camera)
- Foot labels: billboard to overview camera vs flat on floor
- Invaders: **out of scope**