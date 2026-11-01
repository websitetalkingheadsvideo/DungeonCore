# Setup screen showcase — plan

## Current state
- `UDungeonRunSetupWidget`: opaque backdrop + centred card. Sim not seeded until Start Run, so nothing is behind it.
- `ADungeonChunkActor` renders the chamber from sim data only; themes are never read. Before the run it shows a stray cube + crystal at origin (hidden by the backdrop).
- `FDungeonThemeRow` / `FDungeonCreatureFamilyRow` have no usable visual data.
- Library: environment assets for all four themes exist on `H:`; **no creature meshes anywhere locally**. `Design/Monsters.md` names Fab packs that are not downloaded.

## Final state
- Behind the setup card: the real 3×3 core chamber at the real world location, themed per selected theme, core crystal centred, one creature standing beside it. Clicking a theme/family tile re-skins in place.
- Card floats on the left third (per `Design/Start Screen.md`); camera at a 3/4 hero angle; Start Run restores the build camera and hands off to the chunk actor at the same spot.
- Theme/family rows carry the visual data so the same rows can later drive biome breaches in the chunk renderer.
- Creature slot is data-driven; empty until Fab packs are installed, with the gap shown on screen and in the log (no primitive fallback).

## Files
- `Public/Data/DungeonThemeRow.h` — add `FloorMaterial`, `WallMaterial`, `AccentMesh`, `AccentHeightUnits`, `AccentLightColor`.
- `Public/Data/DungeonCreatureFamilyRow.h` — add `ShowcaseSkeletalMesh`, `ShowcaseStaticMesh`, `ShowcaseHeightUnits`, `ShowcaseArtSource`.
- `Private/Data/DungeonContentSubsystem.cpp` — fill rows with `/Game/...` paths + Fab source notes.
- `Public|Private/World/DungeonShowcaseActor.{h,cpp}` — new. Floor/wall ISMs (1 instance per block), 4 corner accents, crystal + glow + spin, creature pivot, key/fill lights. `ApplyTheme(row)`, `ApplyFamily(row)`.
- `Public|Private/Player/DungeonCameraPawn.{h,cpp}` — `FrameForShowcase()`, `ResetToBuildView()`.
- `Public|Private/Player/DungeonPlayerController.{h,cpp}` — spawn/destroy showcase, hide/show chunk actor, `PreviewSetupSelection()`.
- `Public|Private/UI/DungeonRunSetupWidget.{h,cpp}` — transparent backdrop, card left-anchored, default family Orcs, preview call on tile click, pending-art note.
- `Public/Sim/DungeonTypes.h` — showcase camera constants.
- `Content/` — migrated assets below.

## Asset migration (dry-run first)
| Theme | Floor MI | Wall MI | Accent SM |
| --- | --- | --- | --- |
| Gothic | `/Game/GothicCathedral/Materials/MI_Floor_Tile_B` | `/Game/GothicCathedral/Materials/MI_Stone_Tile_A` | `/Game/GothicCathedral/Meshes/SM_Pillar_A` |
| Goblin Tunnels | `/Game/FantasyDungeon/Materials/MI_Dirt_Floor_v01` (in project) | `/Game/GreenCave/Materials/Instances/MI_LargeWall_b` (Lost_Cave) | `/Game/GreenCave/Meshes/SM_SupportBeam` |
| Fungal Warren | `/Game/Crystal_Cave/Materials/LeartesMaterials/Materials/MI_WateryGround` | `/Game/GreenCave/Materials/Instances/MI_LargeWall_b` | `/Game/Crystal_Cave/Meshes/SM_CrystalCluster_D_01` |
| Sunken Ruins | `/Game/JungleRuins/Materials/MI_SandstoneWithMoss` | `/Game/Mayan_Cave/Materials/MI_OrnWall_01` | `/Game/JungleRuins/Meshes/SM_Pillar01` |

Creatures (Fab, user installs): Orcs → MetaGoblin, Undead → Skeleton Stylized Cartoonish, Spiders → Spider, Fairies → Cutie Goddess.

## Tasks
- [ ] Extend theme/family rows + seed data
- [ ] Migrate theme assets
- [ ] Showcase actor
- [ ] Camera framing helpers
- [ ] Controller wiring (spawn / preview / teardown)
- [ ] Widget layout + preview calls + pending note
- [ ] Build, launch, screenshot each theme, grep log
- [ ] TODO.md entry for Fab creature installs
