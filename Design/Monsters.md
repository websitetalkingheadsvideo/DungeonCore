# Monsters — Asset Mapping

Creature picks by family. **Worker** = digs and builds (non-combat or barely), **Minion** = cheap fodder spawned in numbers, **Elite** = tougher and fewer, holds chokepoints, **Boss** = one per chamber, the big threat. **Random** = a family-specific special that can replace any planned spawn (see below). Any family can be paired with any biome ([Creatures and style.md](./Creatures%20and%20style.md)).

**Tier rule:** Every family has **Worker, Minion, Elite, Boss** plus one **Random** column in the UE grid. **Not in UE yet** only documents gaps and alternates — not duplicate tier rows.

## Random tier (special spawn)

Each family has **at most one** named special in the **Random** column (working category name; not the same as [Random Events](./Random%20Events.md)).

**When it rolls:** On every creature spawn, **before** resolving the intended family/tier, roll once for the family's Random special. If it succeeds, spawn the **Random** creature **instead of** whatever was scheduled — same family as the spawn context.

**Chance** — controlled by the **Randomness** setting on the New Game card (saved with the run). Default **Medium**.

| Randomness | Chance any Random special replaces the spawn |
| --- | ---: |
| Low | 0.1% |
| Medium (default) | 1% |
| High | 5% |

**Spawn logic** (roll + replace) is **not built in UE yet** — independent of [Random Events](./Random%20Events.md) and [New Game](./New%20Game.md) **Randomness**. **Meshes** below are in the project unless noted.

| Family           | Random name                   | In UE (mesh) | Role (draft) |
| ---------------- | ----------------------------- | ------------ | ------------ |
| **Evil Fairies** | **Fey** (Paragon) | Yes — full `ParagonFey` pack; **default skin Opaline** (`Fey_Opaline`) | Choose-a-fairy encounter ([TODO.md](../TODO.md)) |
| **Undead**       | **Sevarog** (Paragon) | Yes — `ParagonSevarog/.../Meshes/Sevarog` | — |
| **Orcs**         | **Narbash** (Paragon) | Yes — `ParagonNarbash/.../Meshes/Narbash` | Orc **Random** only; war drum on mesh — keep / hide? ([TODO.md](../TODO.md)) |
| **Fungal**       | **Fungal Queen** | Yes — `FungalQueen/Fungal_Queen` | — |
| **Spiders**      | **Peacock** | Yes — `Spiders/Peacock` (+ anims) | — |
| **Mutants**      | — | **No** — name + asset TBD | — |

## Art direction

- **No stylized cartoon assets.** Grounded, dramatic fidelity to match the skins.
- **Hi3D humanoids** = stylized realism. **Undead with visible faces:** Hi3D → **3D Wrap** (head) → CC → AccuRIG → UE. Other humanoids: Hi3D (Advanced face ~300k+) → CC (no HD Rewrap if Wrap done) → AccuRIG → UE.
- Custom designs from **Hi3D**; Fab fills gaps. Stylized Fab only if **retextured** to match.
- Canonical 2D refs: `Design/Reference Images/`. `imports/` paths are local-only (gitignored).

## In UE (`/Game/Monsters/`)

Skeletal mesh or primary rig per tier. `—` = tier not imported yet.

| Family           | Worker                                              | Minion                                     | Elite                                                     | Boss                                                              | Random                                                                       |
| ---------------- | --------------------------------------------------- | ------------------------------------------ | --------------------------------------------------------- | ----------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| **Orcs**         | `Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion` | `Orcs/3dGoblin/SM_3dGoblin` (static)       | `Orcs/elite_orc`                                          | `Orcs/OrcBoss/Orc_Boss_unreal`                                    | **Narbash** — `Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash` |
| **Evil Fairies** | `EvilFairies/dark_fairy_worker_1`                   | `EvilFairies/dark_fairy_Minion_v1`         | `EvilFairies/dark_fairy_elite_v1`                         | `EvilFairies/dark_fairy_boss_v1`                                  | **Fey (Opaline)** — `EvilFairies/ParagonFey/Characters/Heroes/Fey/Skins/Opaline/Meshes/Fey_Opaline` |
| **Fungal**       | `Fungal/Fungal_Worker_v2`                           | `Fungal/Fungal_Minion`                     | `Fungal/SK_FungiInfestedKnight`                           | `Fungal/SK_FungalMyconidGolem`                                    | **Fungal Queen** — `Fungal/FungalQueen/Fungal_Queen` |
| **Spiders**      | `Spiders/Argiope_v3`                                | `Spiders/Blue_Tarantula_v3`                | `Spiders/Karakurt`                                        | `Spiders/Tarantula`                                               | **Peacock** — `Spiders/Peacock` |
| **Undead**       | `Undead/Zombie`                                     | `Undead/skeleton_v1`                       | `Undead/mummy_v1`                                         | `Undead/litch_v1`                                                 | **Sevarog** — `Undead/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog` |
| **Mutants**      | `Mutants/mutant_worker_v2`                          | `Mutants/Character_001/Meshes/Monster_001` | `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster` | `Mutants/Hollow_Walker/Meshy_AI_The_Hollow_Walker_All_Animations` | —                                                                            |

**Orc Elite** (`elite_orc`): ref [Orc-Elite-v3-closed-mouth.png](./Reference%20Images/Orcs/Orc-Elite-v3-closed-mouth.png); Hi3D `Orc-Elite_v2.glb`; CC `orc-elite_v1.ccProject`.

**Orc Boss** (`Orc_Boss_unreal`): ref [Orc-Boss.png](./Reference%20Images/Orcs/Orc-Boss.png); Hi3D/CC pipeline `imports/hi3d/Orcs/Orc-Boss/`; UE folder `Orcs/OrcBoss/`; showcase idle `Motion/Orc_Boss_unreal_Anim_0_Open_A_UE5` (CC Open A, same pattern as `elite_orc`).

**Undead Worker** (`Zombie`): ref [Zombie.png](./Reference%20Images/Undead/Zombie.png); Hi3D `Zombie.glb`; CC `Zombie.ccProject` (`imports/hi3d/Undead/`).

**Fungal Boss** (`SK_FungalMyconidGolem`): ref [Fungal-Myconid-Golem.png](./Reference%20Images/Fungal/Fungal-Myconid-Golem.png); Hi3D `Fungal-Myconid-Golem.glb`; CC `Fungal-Myconid-Golem.ccProject`.

**Mutants Boss** (Hollow Walker): Fab / Meshy. **Canonical import:** Interchange GLB `01_Animated_GLB/Meshy_AI_The_Hollow_Walker_All_Animations.glb` → skeletal `Meshy_AI_The_Hollow_Walker_All_Animations` + skeleton; **anims:** `...Running`, `...Walking`, `...BeHit_FlyUp`, `...Run_03`, `...Skill_01` (same folder). Reimport: [import_hollow_walker_interchange.py](../DungeonCore/Content/Python/import_hollow_walker_interchange.py) via `UnrealEditor-Cmd -ExecutePythonScript` (MCP `SkeletalMeshTools.import_file` cannot load `.glb`). Stale `SK_Hollow_Walker` orphan removed 2026-11-18. Fab preview: [thumbnail.jpg](../imports/fab/Mutants/Hollow_Walker/glb/thumbnail.jpg).

**Paragon Fey — skin meshes** (no shared `Meshes/Fey`; each skin is its own skeletal mesh under `.../Fey/Skins/<Skin>/Meshes/`):

| Paragon skin id | UE mesh |
| --- | --- |
| **Opaline** (default) | `Fey_Opaline` |
| **AutumnKeeper** | `Fey_AutumnKeeper` |
| **Frostbloom** | `Fey_Frostbloom` |
| **Nightshade** | `Fey_Nightshade` |

**Also in project (not tier-assigned):** `Fungal/Fungal_Elegance_v1` (alternate elite; **not** the Random pick). `Spiders/Spider-Yellow` (alternate spider).

## Not in UE yet

Fab library / pipeline only — **no row** for tiers already listed above.

| Family | Tier | Asset / note |
| --- | --- | --- |
| **Mutants** | Random | Name + asset TBD |
| **Evil Fairies** | Random | **Gameplay** — choose-a-fairy picker not wired; design default mesh **Opaline** ([TODO.md](../TODO.md)) |
| **Undead** | Random | Alternate (not tier pick): **Wraith** — Fab TBD |
| **Fungal** | Elite | Alternates: Mushroom Demon (Fab); `Fungal_Elegance_v1` already in project |
| | Minion | Alternate (rejected for tier): **Cyclostome Agaricus** (Fab) |
| **Undead** | Boss | Alternate: Grave Keeper (Fab) |
| **Evil Fairies** | Boss | Alternate: Harpy Assassin (Fab, review fit) |

Orc worker retexture + **Handheld Torch**; MetaGoblin / catalog **3dGoblin** as minion alternates; Fab **Warrior Orc** as elite alternate (not imported). Evil Fairies minion: rejected Fab **Cutie Goddess**.

Spider tier is by role, not mesh size. Minion source is `imports/blender/Blue Tarantula_v3.fbx`. Source FBX is 25 fps. `attack_1` is 0.96s and `attack_2` is 0.76s, so those two clips miss the 60 fps frame grid and did not import. Every spider mesh has `dead`, `idle`, `idle_2`, `walk`. The original `Blue Tarantula.fbx` did not import — its armature node is named `Armature`, UE drops that node, and `Bone` plus `body` are then two roots. `Blue_Tarantula-v2` is still in the folder; that export holds the last attack frame one extra 25 fps frame, so it has `attack_1` and `attack_2`. Peacock, Spider-Yellow, and Tarantula share one body mesh and differ by diffuse. Karakurt's body is its own. Blue Tarantula is a separate mesh. Fab alternate for the elite: Theraphosa blondi. Diffuses imported beside the meshes (`Argiope_difuse`, `blue_tarantula_difuse`, `karakurt_difuse`, `tarantula_difuse`, `spider_peacock`, `yellow_floral_spider_difuse`). Karakurt has no physics asset (bones too small).

## Intruders (adventurers)

Surface heroes — not dungeon families. Fab / Paragon catalog, plus one custom Firefly party. `—` = not under `/Game/` yet.

**AI, party behavior, trap knowledge, and character-sheet fields:** [Invaders.md](./Invaders.md).

| Role | Primary asset | Named / alternates |
| --- | --- | --- |
| **Wanderer** | Wanderer | — |
| **Assassin** | Free Medieval Assassin Girl | — |
| **Hero** | Heroic Warrior | — |
| **Champion** | Paragon: **Terra** (`/Game/Invaders/ParagonTerra`) | Warrior Queen (rigged) — alternate |
| **Tank** | Paragon: **Greystone** | Paladin RPG Set (knight skins) |
| **Bruiser** | Paragon: **Kwang** (`/Game/Invaders/ParagonKwang`) | Gothic Knight |
| **Mage** | Paragon: **Phase** | Aurora (`/Game/Invaders/ParagonAurora`) - alternate |
| **Archer** | Paragon: **Sparrow** (`/Game/Invaders/ParagonSparrow`) | — |
| **Dwarf Fighter** | Firefly / Hi3D TBD | Intruder party |
| **Elf Bowman** | Firefly / Hi3D TBD | Intruder party |
| **Old Wizard** | Firefly / Hi3D TBD | Intruder party |
| **Female Human Cleric** | Paragon: **Zinx** (`/Game/Invaders/ParagonZinx`) | Firefly / Hi3D TBD - Zinx has a non-sci-fi skin, so she is the Cleric unless the skin does not fit, in which case drop her |

**Intruder party** (custom, not in UE): one shared kit — charcoal wool, ivory linen, dark leather, brushed steel, muted navy trim. Clean stylized realism, white T-pose sheets. Dwarf is the scale fighter: iron cap on his head, shield on the back, axe on the hip.

**Knight / paladin variants (Fab):** **Gothic Knight**, **Paladin RPG Set** — retexture to match art pillar; assign to Hero or Tank spawn tables.

See also [library.md](../library.md) (Fab invaders list), [Future.md](./Future.md) (Warrior Queen, Fantasy RPG Humanoid Pack).
