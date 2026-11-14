# Dungeon Core — TODO

- [ ] **Quixel / Megascans — age verification** — Megascans are available through Fab. Quixel Bridge is still blocked by the Epic/Quixel age-verification bug (contact Quixel support only if Bridge is needed).
- [ ] **Hollow Walker boss cleanup** — Boss mesh + 5 anims in `/Game/Monsters/Mutants/Hollow_Walker/` (Interchange GLB). Delete stale `SK_Hollow_Walker`; optional rename to `SK_Hollow_Walker`. Wire ABP / spawn when gameplay needs it.
- [ ] **Setup-screen creature art** — Wire `ShowcaseSkeletalMesh` per family in `DungeonContentSubsystem.cpp` where missing. Spiders/New Game showcase may still need hookup.
- [ ] **Git LFS** — `Content/` is now ~814 MB of binary `.uasset` with no LFS tracking on a GitHub remote; set up `git lfs track "*.uasset" "*.umap"` before the next large migration.
- [ ] **Quixel / Megascans — MCP assessment** — With account unblocked, assess Quixel Bridge workflow through Unreal MCP (open Bridge, export a free asset, confirm `/Game` import, select/preview via MCP, then migrate + wire into project as needed).
- [ ] **Character powers** — Design abilities for every creature family (Worker / Minion / Elite / Boss tiers in `Design/Monsters.md`) and every invader role (Wanderer, Assassin, Hero, plus Paragon additions). Cover role, active and passive powers, and counters (which traps and monsters beat them).
- [ ] **Worker picks** — Choose a Worker asset for Evil Fairies in `Design/Monsters.md` (Orcs, Fungal, Spiders, Undead, Mutants workers are assigned in the UE grid).
- [ ] **Heroic Warrior (invader)** - Imported as a static mesh at `/Game/Invaders/HeroicWarrior/SM_HeroicWarrior` (55K tris, 2 m tall) from `F:\Epic\Heroic_Warrior-7d3733ab`. No skeleton: rig it in AccuRig, reimport as a skeletal mesh, and hook up its separate texture (`fbx\textures\heroic_fantasy_brythu_0926193638_refine.png`).
