# Dungeon Core — TODO

- [ ] **Quixel / Megascans — age verification** — Megascans are available through Fab. Quixel Bridge is still blocked by the Epic/Quixel age-verification bug (contact Quixel support only if Bridge is needed).
- [ ] **Hollow Walker boss cleanup** — Boss mesh + 5 anims in `/Game/Monsters/Mutants/Hollow_Walker/` (Interchange GLB). Delete stale `SK_Hollow_Walker`; optional rename to `SK_Hollow_Walker`. Wire ABP / spawn when gameplay needs it.
- [ ] **Setup-screen creature art** — Wire `ShowcaseSkeletalMesh` per family in `DungeonContentSubsystem.cpp` where missing. Spiders/New Game showcase may still need hookup.
- [ ] **Git LFS** — `Content/` is now ~814 MB of binary `.uasset` with no LFS tracking on a GitHub remote; set up `git lfs track "*.uasset" "*.umap"` before the next large migration.
- [ ] **Quixel / Megascans — MCP assessment** — With account unblocked, assess Quixel Bridge workflow through Unreal MCP (open Bridge, export a free asset, confirm `/Game` import, select/preview via MCP, then migrate + wire into project as needed).
- [ ] **Character powers** — Design abilities for every creature family (Worker / Minion / Elite / Boss tiers in `Design/Monsters.md`) and every invader role (Wanderer, Assassin, Hero, plus Paragon additions). Cover role, active and passive powers, and counters (which traps and monsters beat them).
- [ ] **Worker picks** — Choose a Worker asset for Evil Fairies in `Design/Monsters.md` (Orcs, Fungal, Spiders, Undead, Mutants workers are assigned in the UE grid).
- [ ] **Heroic Warrior (invader)** - Imported as a static mesh at `/Game/Invaders/HeroicWarrior/SM_HeroicWarrior` (55K tris, 2 m tall) from `F:\Epic\Heroic_Warrior-7d3733ab`. No skeleton: rig it in AccuRig, reimport as a skeletal mesh, and hook up its separate texture (`fbx\textures\heroic_fantasy_brythu_0926193638_refine.png`).
- [ ] **Convert Fab creatures to the CC skeleton** (Character Creator / AccuRig, same as `orc_minion.fbx`) so every humanoid shares one animation set. Export FBX to `D:\DungeonCore\` and import. Still to convert:
  - Orc Worker — `Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion`
  - Orc Elite — `Orcs/elite_orc`
  - Fungal Boss — `Fungal/SK_FungalMyconidGolem`
  - Mutant Minion — `Mutants/Character_001/Meshes/Monster_001`
  - Mutant Elite — `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster`
  - Skip: Spiders (8 legs), Narbash (full Paragon anim set). Done: Orc Minion (`Orcs/OrcMinion/SK_OrcMinion`), Mutant Boss (`Mutants/MutantBoss/SK_MutantBoss`).
- [ ] **Orc Boss drum** — Paragon Narbash wears a war drum (bones `drum_main`, `drum_side_l/r`, material slot `M03_Drums`). Decide: keep, hide the drum, or pick a different Orc Boss.
- [ ] **3D models for store items** — Placed items are still placeholder cubes (`M_ItemPlaceholder` in `BP_DungeonGrid` Items ISM) with a text label. Check the asset library first, then source or make a model for each: Monster Generator, Mana Generator, Prism Beam Trap, Resonance Plate Trap, Gravity Well Trap, Runaway Cart Trap, Loot Bait, Unshackled Golem, Shard Sentinel (see `Design/Store.md`).
