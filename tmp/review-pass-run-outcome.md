# Review pass — run outcome / packaging work (uncommitted)

## Current state
Previous session implemented the plan todos, Win64 packaging, cook/startup fixes, and a
"Core Destroyed Game Over!" modal. HUD edit postdates the last editor DLL, so it was never compiled.

## Findings
- `DungeonResourceWidgetBridge`: pure pass-through between sim delegates and HUD; unused `Owner`. Dead layer.
- `DungeonAssetRow.h`: unused `using` alias created only to match a plan filename.
- Run-end gating only on `DigBlock` (reports `Blocked`); rooms/traps/upgrades still mutate after defeat.
- Mana tick keeps running after run ends; `LoadSavedRun` restarts timers on finished runs.
- Timer clearing duplicated 3x; `StopInvasionTimers` not reused.
- `SeedStartingClearedTilesAroundCore` clears core-block neighbours: with a 2x2 chamber only 2 rock tiles qualify (plan says 4).
- `DungeonGameMode::BeginPlay` forces windowed 1280x720 every level start (code where config belongs).
- Shop catalog staged twice (`Build.cs` RuntimeDependencies + `DefaultGame.ini` NonUFS dir).
- Duplicate `VerticalBoxSlot.h` include; `RefreshRunOutcomeText` has side effects beyond text.
- `Packaged/` output and `.lnk` not gitignored.

## Final state
- HUD binds directly to sim delegates (incl. `OnRunOutcomeChanged`); bridge + alias header removed.
- `EPlacementFailureReason::RunEnded`; all four sim mutators gated by one helper; HUD text for it.
- `StopRunTimers()` clears all four timers; used by Deinitialize/StartNewRun/LoadSavedRun/SetRunOutcome.
- `RestoreSimulationTimers()` respects run outcome and owns the mana tick restart.
- Starting cleared tiles: one tile touching the middle of each chamber side (4 total).
- Window defaults in `Config/DefaultGameUserSettings.ini`; BeginPlay hack removed.
- Catalog staging via `DefaultGame.ini` only.
- Editor target compiles.

## Files
- Source/DungeonCore/Public|Private/UI/DungeonHudWidget.*
- Source/DungeonCore/Public|Private/UI/DungeonResourceWidgetBridge.* (delete)
- Source/DungeonCore/Public/Data/DungeonAssetRow.h (delete)
- Source/DungeonCore/Public/Sim/DungeonTypes.h, DungeonSimSubsystem.h/.cpp
- Source/DungeonCore/Private/Game/DungeonGameMode.cpp
- Source/DungeonCore/DungeonCore.Build.cs
- Config/DefaultGameUserSettings.ini (new)
- PackageWin64.bat, .gitignore

## Checklist
- [x] HUD direct binding, rename, include cleanup, RunEnded text
- [x] Delete bridge + alias header
- [x] Sim: RunEnded, StopRunTimers, RestoreSimulationTimers, seeding
- [x] GameMode + DefaultGameUserSettings.ini
- [x] Build.cs / .gitignore / bat
- [x] Compile DungeonCoreEditor and DungeonCore (Win64 Development) — both succeed
