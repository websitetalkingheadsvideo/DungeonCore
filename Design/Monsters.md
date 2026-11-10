# Monsters — Asset Mapping

Creature picks by family. **Worker** = digs and builds (non-combat or barely), **Minion** = cheap fodder spawned in numbers, **Elite** = tougher and fewer, holds chokepoints, **Boss** = one per chamber, the big threat. Any family can be paired with any biome ([Creatures and style.md](./Creatures%20and%20style.md)).

**Tier rule:** Every family has **exactly four tiers: Worker, Minion, Elite, Boss**. The UE grid always lists all four; **Not in UE yet** only documents gaps and alternates — not extra tier rows.

## Art direction

- **No stylized cartoon assets.** Grounded, dramatic fidelity to match the skins.
- **Hi3D humanoids** = stylized realism. **Undead with visible faces:** Hi3D → **3D Wrap** (head) → CC → AccuRIG → UE. Other humanoids: Hi3D (Advanced face ~300k+) → CC (no HD Rewrap if Wrap done) → AccuRIG → UE.
- Custom designs from **Hi3D**; Fab fills gaps. Stylized Fab only if **retextured** to match.
- Canonical 2D refs: `Design/Reference Images/`. `imports/` paths are local-only (gitignored).

## In UE (`/Game/Monsters/`)

Skeletal mesh or primary rig per tier. `—` = tier not imported yet.

| Family | Worker | Minion | Elite | Boss |
| --- | --- | --- | --- | --- |
| **Orcs** | `Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion` | `Orcs/3dGoblin/SM_3dGoblin` (static) | `Orcs/elite_orc` | `Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash` |
| **Evil Fairies** | `EvilFairies/dark_fairy_worker_1` | `EvilFairies/dark_fairy_Minion_v1` | `EvilFairies/dark_fairy_elite_v1` | `EvilFairies/dark_fairy_boss_v1` |
| **Fungal** | — | — | `Fungal/SK_FungiInfestedKnight` | `Fungal/SK_FungalMyconidGolem` |
| **Spiders** | — | — | — | — |
| **Undead** | — | `Undead/skeleton_v1` | `Undead/mummy_v1` | `Undead/litch_v1` |
| **Mutants** | `Mutants/mutant_worker_v2` | `Mutants/Character_001/Meshes/Monster_001` | `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster` | — |

**Orc Elite** (`elite_orc`): ref [Orc-Elite-v3-closed-mouth.png](./Reference%20Images/Orcs/Orc-Elite-v3-closed-mouth.png); Hi3D `Orc-Elite_v2.glb`; CC `orc-elite_v1.ccProject`.

**Also in project (not tier-assigned):** `Fungal/Fungal_Elegance_v1` (alternate elite).

## Not in UE yet

Fab library / pipeline only — **no row** for tiers already listed above.

| Family | Tier | Asset / note |
| --- | --- | --- |
| **Fungal** | Worker | TBD ref `Fungal-Worker-v1.png` — [Hi3D prompt](../../../.cursor/skills/hi3d-image-to-3d/references/prompts.md#fungal-worker--spore-digger-hi3d-2d-ref) |
| | Minion | **Cyclostome Agaricus** (Fab) or Hi3D TBD |
| | Elite | Alternates: Mushroom Demon (Fab); `Fungal_Elegance_v1` already in project |
| **Spiders** | Worker | TBD |
| | Minion | Spider (Fab) |
| | Elite | Theraphosa blondi (Fab) |
| | Boss | Spider Tarantula (Fab) |
| **Undead** | Worker | Zombie (Fab) |
| | Boss | Alternate: Grave Keeper (Fab) |
| **Mutants** | Boss | **The Hollow Walker** (Fab) |
| **Evil Fairies** | Boss | Alternate: Harpy Assassin (Fab, review fit) |

Orc worker retexture + **Handheld Torch**; MetaGoblin / catalog **3dGoblin** as minion alternates; Fab **Warrior Orc** as elite alternate (not imported). Evil Fairies minion: rejected Fab **Cutie Goddess**.

## Intruders (adventurers)

Surface heroes — not dungeon families. Fab / Paragon catalog; `—` = not under `/Game/` yet.

| Role | Primary asset | Named / alternates |
| --- | --- | --- |
| **Wanderer** | Wanderer | — |
| **Assassin** | Free Medieval Assassin Girl | — |
| **Hero** | Heroic Warrior | — |
| **Champion** | **Warrior Queen** (rigged) | — |
| **Tank** | Paragon: **Greystone** | Paladin RPG Set (knight skins) |
| **Bruiser** | Paragon: **Kwang** | Gothic Knight |
| **Mage** | Paragon: **Aurora** | — |
| **Archer** | **Robin** | — |

**Knight / paladin variants (Fab):** **Gothic Knight**, **Paladin RPG Set** — retexture to match art pillar; assign to Hero or Tank spawn tables.

See also [library.md](../library.md) (Fab invaders list), [Future.md](./Future.md) (Warrior Queen, Fantasy RPG Humanoid Pack).
