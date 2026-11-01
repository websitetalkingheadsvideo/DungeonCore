# Dungeon Core — TODO

- [ ] **Quixel / Megascans — age verification** — Resolve Epic/Quixel account age-verification bug blocking Bridge/Fab Megascans imports (contact Quixel support if needed).
- [ ] **Setup-screen creature art** — No creature meshes exist locally. Install the Fab packs from `Design/Monsters.md` (MetaGoblin, Skeleton Stylized Cartoonish, Spider, Cutie Goddess) into a UE project under `H:\Unreal Assets`, run `tools/asset-library/Sync-PackRoots.ps1`, migrate one SK_ per family, then set `ShowcaseSkeletalMesh` / `ShowcaseHeightUnits` / `ShowcaseFacingYawDegrees` on the family rows in `DungeonContentSubsystem.cpp`. Until then the card shows "Creature art pending — Fab: …".
- [ ] **Git LFS** — `Content/` is now ~814 MB of binary `.uasset` with no LFS tracking on a GitHub remote; set up `git lfs track "*.uasset" "*.umap"` before the next large migration.
- [ ] **Quixel / Megascans — MCP assessment** — With account unblocked, assess Quixel Bridge workflow through Unreal MCP (open Bridge, export a free asset, confirm `/Game` import, select/preview via MCP, then migrate + wire into project as needed).
