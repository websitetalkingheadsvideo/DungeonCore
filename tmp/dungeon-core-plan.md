# Dungeon Core — implementation index

Authoritative design spec lives in the Cursor plan `dungeon_core_plan_a06e33c3.plan.md` (do not edit that file from automation).

This repo implements the plan in `DungeonCore/`:

| Plan area | Location |
|-----------|----------|
| Sim / grid | `Source/DungeonCore/Public/Sim/`, `Private/Sim/` |
| Content rows | `Source/DungeonCore/Public/Data/` |
| World visuals | `Source/DungeonCore/Public/World/` |
| UI | `Source/DungeonCore/Public/UI/` |
| Scale | `DungeonTypes.h` — 1 block = 300 uu (10 ft cube) |
| Build | `Rebuild.bat`, `PackageWin64.bat` |
| Rules | `../.cursorrules` |

Phases 1–6.5: prototype loop (setup → dig/breach → waves → core health). Phase 8: Win64 packaging stub; Steam deferred.
