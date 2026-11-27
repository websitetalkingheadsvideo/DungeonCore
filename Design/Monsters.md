# Monsters — Asset Mapping

Creature picks by family. **Worker** = digs and builds (non-combat or barely), **Minion** = cheap fodder spawned in numbers, **Elite** = tougher and fewer, holds chokepoints, **Boss** = one per chamber, the big threat. **Hero** = a family-specific special that can replace any planned spawn (see below). Any family can be paired with any biome ([Creatures and style.md](./Creatures%20and%20style.md)).

**Tier rule:** Every family has **Worker, Minion, Elite, Boss** plus one **Hero** column in the UE grid. **Not in UE yet** only documents gaps and alternates — not duplicate tier rows.

## Hero tier (special spawn)

Each family has **at most one** named special in the **Hero** column (not the same as [Random Events](./Random%20Events.md)).

**When it rolls:** On every creature spawn, **before** resolving the intended family/tier, roll once for the family's Hero special. If it succeeds, spawn the **Hero** creature **instead of** whatever was scheduled — same family as the spawn context.

**Chance** — controlled by the **Randomness** setting on the New Game card (saved with the run). Default **Medium**.

| Randomness | Chance any Hero special replaces the spawn |
| --- | ---: |
| Low | 0.1% |
| Medium (default) | 1% |
| High | 5% |

**Spawn logic** (roll + replace) is **not built in UE yet** — independent of [Random Events](./Random%20Events.md) and [New Game](./New%20Game.md) **Randomness**. **Meshes** below are in the project unless noted.

| Family           | Hero name                   | In UE (mesh) | Role (draft) |
| ---------------- | ----------------------------- | ------------ | ------------ |
| **Evil Fairies** | **Fey** (Paragon) | Yes — full `ParagonFey` pack; **default skin Opaline** (`Fey_Opaline`) | Choose-a-fairy encounter ([TODO.md](../TODO.md)) |
| **Undead**       | **Sevarog** (Paragon) | Yes — `ParagonSevarog/.../Meshes/Sevarog` | — |
| **Orcs**         | **Narbash** (Paragon) | Yes — `ParagonNarbash/.../Meshes/Narbash` | Orc **Hero** only; war drum on mesh — keep / hide? ([TODO.md](../TODO.md)) |
| **Fungal**       | **Fungal Queen** | Yes — `FungalQueen/Fungal_Queen` | — |
| **Spiders**      | **Peacock** | Yes — `Spiders/Peacock` (+ anims) | — |
| **Mutants**      | **Rampage** (Paragon) | Yes — `/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage` (Epic mount; not under `Monsters/Mutants/`) | Brute Hero; full Paragon anim set — no CC skeleton ([Invaders-Paragons.md](./Invaders-Paragons.md)) |

## Art direction

- **No stylized cartoon assets.** Grounded, dramatic fidelity to match the skins.
- **Hi3D humanoids** = stylized realism. **Undead with visible faces:** Hi3D → **3D Wrap** (head) → CC → AccuRIG → UE. Other humanoids: Hi3D (Advanced face ~300k+) → CC (no HD Rewrap if Wrap done) → AccuRIG → UE.
- Custom designs from **Hi3D**; Fab fills gaps. Stylized Fab only if **retextured** to match.
- Canonical 2D refs: `Design/Reference Images/`. `imports/` paths are local-only (gitignored).

## In UE (`/Game/Monsters/`)

Skeletal mesh or primary rig per tier. `—` = tier not imported yet.

| Family           | Worker                                              | Minion                                     | Elite                                                     | Boss                                                              | Hero                                                                         |
| ---------------- | --------------------------------------------------- | ------------------------------------------ | --------------------------------------------------------- | ----------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| **Orcs**         | `Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion` | `Orcs/Minion/Orc_Minion_Unreal`            | `Orcs/elite_orc`                                          | `Orcs/OrcBoss/Orc_Boss_unreal`                                    | **Narbash** — `Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash` |
| **Evil Fairies** | `EvilFairies/dark_fairy_worker_1`                   | `EvilFairies/dark_fairy_Minion_v1`         | `EvilFairies/dark_fairy_elite_v1`                         | `EvilFairies/dark_fairy_boss_v1`                                  | **Fey (Opaline)** — `EvilFairies/ParagonFey/Characters/Heroes/Fey/Skins/Opaline/Meshes/Fey_Opaline` |
| **Fungal**       | `Fungal/Fungal_Worker_v2`                           | `Fungal/Fungal_Minion`                     | `Fungal/SK_FungiInfestedKnight`                           | `Fungal/SK_FungalMyconidGolem`                                    | **Fungal Queen** — `Fungal/FungalQueen/Fungal_Queen` |
| **Spiders**      | `Spiders/Argiope_v3`                                | `Spiders/Blue_Tarantula_v3`                | `Spiders/Karakurt`                                        | `Spiders/Tarantula`                                               | **Peacock** — `Spiders/Peacock` |
| **Undead**       | `Undead/Zombie`                                     | `Undead/skeleton_v1`                       | `Undead/mummy_v1`                                         | `Undead/litch_v1`                                                 | **Sevarog** — `Undead/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog` |
| **Mutants**      | `Mutants/mutant_worker_v2`                          | `Mutants/Minion/Mutant_Minion_Unreal`      | `Mutants/mutant_worker_v2`                                | `Mutants/Boss/Mutant_Boss`                                          | **Rampage** — `/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage` |

## Monster tier gallery (review map)

**Map:** `/Game/Maps/Dev/L_MonsterTierGallery` — PIE showcase for the **In UE** grid above (6 families × Worker → Hero). Outliner: `Gallery` → `Displays/{Orcs, EvilFairies, Fungal, Undead, Mutants, Spiders}`, plus `Gallery/Environment`, `Gallery/Lighting`. Per-tier foot labels on the displays.

**Paragon on this map (Epic launcher — not duplicated into repo wholesale):**

| Family | Hero column on map | Mount style |
| --- | --- | --- |
| **Orcs** | Narbash — `Monsters/Orcs/ParagonNarbash/.../Meshes/Narbash` | Paragon pack under **Monsters** tree (subset in project) |
| **Evil Fairies** | Fey Opaline — `Monsters/EvilFairies/ParagonFey/.../Fey_Opaline` | Same |
| **Undead** | Sevarog — `Monsters/Undead/ParagonSevarog/.../Meshes/Sevarog` | Same |
| **Mutants** | Rampage — `/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage` | **Default Epic mount** — vault/launcher; not under `Content/Monsters/Mutants/` |

**Custom heroes on map (non-Paragon):** Fungal Queen, Spiders Peacock. **Invader** Firefly / launcher lineup is on **`/Game/Maps/Dev/L_Store`** ([Invaders.md](./Invaders.md#in-ue-gameinvaders)) — not this map unless you merge galleries in-editor.

**Doc sync for agents:** Inventory this level from **Unreal** (MCP or `.umap` `/Game/` string scan), not only `DungeonCore/Content/` on disk.

**Orc Minion** (`Orc_Minion_Unreal`): Hi3D → CC → UE at `Orcs/Minion/Orc_Minion_Unreal`. Ref [Goblin-Minion.png](./Reference%20Images/Orcs/Goblin-Minion.png) — same stocky goblin-orc species as worker, **open brown leather vest** + **matching leather miner hat**, no torch. CC rig under `Orcs/Minion/Post_Process/`. **Supersedes** Fab `Orcs/MetaGoblin/MHC_MetaGoblin` (retain Fab pack until spawn refs retargeted).

**Orc Minion (superseded Fab)** (`MHC_MetaGoblin`): Fab **MetaGoblin** under `Orcs/MetaGoblin/` — not tier pick; was prior grid minion.

**Orc Elite** (`elite_orc`): ref [Orc-Elite-v3-closed-mouth.png](./Reference%20Images/Orcs/Orc-Elite-v3-closed-mouth.png); Hi3D `Orc-Elite_v2.glb`; CC `orc-elite_v1.ccProject`.

**Orc Boss** (`Orc_Boss_unreal`): ref [Orc-Boss.png](./Reference%20Images/Orcs/Orc-Boss.png); Hi3D/CC pipeline `imports/hi3d/Orcs/Orc-Boss/`; UE folder `Orcs/OrcBoss/`; showcase idle `Motion/Orc_Boss_unreal_Anim_0_Open_A_UE5` (CC Open A, same pattern as `elite_orc`).

**Undead Worker** (`Zombie`): ref [Zombie.png](./Reference%20Images/Undead/Zombie.png); Hi3D `Zombie.glb`; CC `Zombie.ccProject` (`imports/hi3d/Undead/`).

**Fungal Boss** (`SK_FungalMyconidGolem`): ref [Fungal-Myconid-Golem.png](./Reference%20Images/Fungal/Fungal-Myconid-Golem.png); Hi3D `Fungal-Myconid-Golem.glb`; CC `Fungal-Myconid-Golem.ccProject`.

**Mutants Worker** (`mutant_worker_v2`): Hi3D → CC → UE raw-flesh digger (no fungal growth, no tooth-maw). Ref [Mutant-Worker_v3.png](./Reference%20Images/Mutants/Mutant-Worker_v3.png); pipeline slug `mutant-worker`. CC post-process at `Mutants/Post_Process/` (family root).

**Mutants Minion** (`Mutant_Minion_Unreal`): Hi3D → CC → `Mutants/Minion/Mutant_Minion_Unreal`. Ref [Mutant Minion.png](./Reference%20Images/Mutants/Mutant%20Minion.png); CC under `Mutants/Minion/Post_Process/`. Same flesh family as worker; tier read = shorter stockier runt (scale in spawn still fine-tune).

**Mutants Elite:** still **`mutant_worker_v2`** at family root until dedicated elite Hi3D import — differentiate via scale / MI at spawn.

**Mutants / Orcs folder layout:** Tier subfolders where imported — e.g. `Mutants/Minion/`, `Mutants/Boss/`, `Orcs/Minion/` (`Post_Process`, `Motion`, `Rigs`). **Worker** meshes may stay at family root until moved.

**Mutants Boss** (`Mutant_Boss`): Hi3D → CC → AccuRIG → UE at `Mutants/Boss/Mutant_Boss`. Hi3D `Mutant-Boss.glb`; CC `imports/hi3d/Mutant/mutant-boss.ccProject`. CC rig blueprints + ABP live under `Mutants/Boss/Post_Process/` (`Mutant_Boss_Blueprint_CC_ControlRig`, `Mutant_Boss_Blueprint_LiveLink`, etc.). Idle pattern: CC Open A → `Mutant_Boss_Anim_0_Open_A_UE5` (same as other humanoids). Shares flesh-mutant art line with worker; boss read = bulk / maw from Hi3D boss gen (Hollow Walker = alternate).

**Paragon Fey — skin meshes** (no shared `Meshes/Fey`; each skin is its own skeletal mesh under `.../Fey/Skins/<Skin>/Meshes/`):

| Paragon skin id | UE mesh |
| --- | --- |
| **Opaline** (default) | `Fey_Opaline` |
| **AutumnKeeper** | `Fey_AutumnKeeper` |
| **Frostbloom** | `Fey_Frostbloom` |
| **Nightshade** | `Fey_Nightshade` |

**Also in project (not tier-assigned):** `Orcs/MetaGoblin/MHC_MetaGoblin` (superseded minion — Fab). `Orcs/OrcMinion/SK_OrcMinion` (older Hi3D/CC minion). `Orcs/OrcWorker/SK_OrcWorker` (duplicate of old minion — not Worker tier). `Fungal/Fungal_Elegance_v1` (alternate elite; **not** the Hero pick). `Spiders/Spider-Yellow` (alternate spider). `Mutants/Character_001/Meshes/Monster_001` and `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster` (superseded Fab flesh mutants — tooth-maw style; anim demo packs retained). `Mutants/MutantBoss/` (pre-reorg CC import — superseded by `Mutants/Boss/`). `Mutants/Hollow_Walker/Meshy_AI_The_Hollow_Walker_All_Animations` (Meshy alternate boss; embedded anims, not CC skeleton).

## Not in UE yet

Fab library / pipeline only — **no row** for tiers already listed above.

| Family | Tier | Asset / note |
| --- | --- | --- |
| **Evil Fairies** | Hero | **Gameplay** — choose-a-fairy picker not wired; design default mesh **Opaline** ([TODO.md](../TODO.md)) |
| **Undead** | Hero | Alternate (not tier pick): **Wraith** — Fab TBD |
| **Fungal** | Elite | Alternates: Mushroom Demon (Fab); `Fungal_Elegance_v1` already in project |
| | Minion | Alternate (rejected for tier): **Cyclostome Agaricus** (Fab) |
| **Undead** | Boss | Alternate: Grave Keeper (Fab) |
| **Evil Fairies** | Boss | Alternate: Harpy Assassin (Fab, review fit) |

Orc worker retexture + **Handheld Torch**; Fab **Warrior Orc** as elite alternate (not imported). Evil Fairies minion: rejected Fab **Cutie Goddess**.

Spider tier is by role, not mesh size. Minion source is `imports/blender/Blue Tarantula_v3.fbx`. Source FBX is 25 fps. `attack_1` is 0.96s and `attack_2` is 0.76s, so those two clips miss the 60 fps frame grid and did not import. Every spider mesh has `dead`, `idle`, `idle_2`, `walk`. The original `Blue Tarantula.fbx` did not import — its armature node is named `Armature`, UE drops that node, and `Bone` plus `body` are then two roots. `Blue_Tarantula-v2` is still in the folder; that export holds the last attack frame one extra 25 fps frame, so it has `attack_1` and `attack_2`. Peacock, Spider-Yellow, and Tarantula share one body mesh and differ by diffuse. Karakurt's body is its own. Blue Tarantula is a separate mesh. Fab alternate for the elite: Theraphosa blondi. Diffuses imported beside the meshes (`Argiope_difuse`, `blue_tarantula_difuse`, `karakurt_difuse`, `tarantula_difuse`, `spider_peacock`, `yellow_floral_spider_difuse`). Karakurt has no physics asset (bones too small).

## Intruders (adventurers)

Surface heroes — not dungeon families. **Full Epic Paragon hero catalog owned** (not all migrated); Fab fills gaps; **Firefly party** + knights **in UE** ([Invaders.md — In UE](./Invaders.md#in-ue-gameinvaders)). `—` = not under `/Game/` yet. Owned-but-unassigned heroes: [Invaders-Paragons.md](./Invaders-Paragons.md#owned-catalog-vs-in-project).

**Combat character sheets** (all families + tiers + invaders): [Character-Sheets.md](./Character-Sheets.md). **Invader trap / party AI:** [Invaders.md](./Invaders.md). **Paragon wiki map:** [Invaders-Paragons.md](./Invaders-Paragons.md).

| Role | Primary asset | Named / alternates |
| --- | --- | --- |
| **Wanderer** | Wanderer | — |
| **Thief** | Free Medieval Assassin Girl | Fab — mount TBD; `ArchetypeId = Thief` (**lockpick** + **climb walls** on sheet) |
| **Assassin** | — | Paragon: **Countess**, **Kallari**, … ([Invaders-Paragons.md](./Invaders-Paragons.md)); burst combat, not the Thief expedition kit |
| **Hero** | `Invaders/HeroicWarrior/SM_HeroicWarrior` | Static mesh only — skeletal TBD |
| **Champion** | Paragon: **Terra** (`/Game/Invaders/ParagonTerra`) | Warrior Queen (rigged) — alternate |
| **Tank** | Paragon: **Greystone** (`/Game/Invaders/ParagonGreystone`) | Paragon **Rampage** (Tank alt); `Invaders/Paladin/Paladin` (CC); `Invaders/GKnight/` (**Gothic Knight** VA–VF) |
| **Bruiser** | Paragon: **Kwang** (`/Game/Invaders/ParagonKwang`) | `Invaders/GKnight/Meshes/SK_GothicKnight_VA` (Fab modular **Gothic Knight**) |
| **Mage** | Paragon: **Phase** | Aurora (`/Game/Invaders/ParagonAurora`) - alternate |
| **Archer** | Paragon: **Sparrow** (`/Game/Invaders/ParagonSparrow`) | — |
| **Dwarf Fighter** | `Invaders/Dwarf/Dwarf_Fighter` | Firefly party · [Invader-Dward_v2.png](./Reference%20Images/Invaders/Invader-Dward_v2.png) |
| **Elf Bowman** | `Invaders/Elf-Archer/Elf_Archer` | Firefly party · [Invader-Elf.png](./Reference%20Images/Invaders/Invader-Elf.png) |
| **Fighter (Thelma)** | `Invaders/Thelma/Invader_Thelma` | Armored fighter — helm, axe; CC (folder name = character, not role) |
| **Wizard** | `Invaders/Wizard/Invader_Wizard_Unreal` | Old staff-mage; Firefly / CC in UE |
| **Elf Mage** | `Invaders/Elf-Mage/Elf_Mage` | Firefly / CC in UE (separate from bowman) |
| **Female Human Cleric** | Paragon: **Zinx** (`/Game/Invaders/ParagonZinx`) | Zinx non-sci-fi skin = default Cleric unless art review fails |

**Intruder party** (Common line): shared kit — charcoal wool, ivory linen, dark leather, brushed steel, muted navy trim. **In UE:** all under `/Game/Invaders/` — dwarf, elf archer, elf mage, **Thelma** (fighter), **Wizard** (staff mage); lineup on **`/Game/Maps/Dev/L_MonsterTierGallery`** ([Invaders.md](./Invaders.md#in-ue-gameinvaders)). Cleric slot: **Zinx** or future Firefly cleric.

**Knight / paladin (Fab + CC):** `Invaders/GKnight/` (**Gothic Knight** variants VA–VF); `Invaders/Paladin/Paladin` — CC import with art-pillar retexture; assign to Tank / Bruiser spawn templates.

See also [library.md](../library.md) (Fab invaders list), [Future.md](./Future.md) (Warrior Queen, Fantasy RPG Humanoid Pack).
