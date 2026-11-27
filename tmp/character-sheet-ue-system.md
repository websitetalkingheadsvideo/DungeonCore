# Character sheet — UE three-layer system

**Design:** [Character-Sheets.md](../Design/Character-Sheets.md) · **JSON:** [character-sheets.json](../Data/character-sheets.json)

## Layers

| Layer | C++ | Mutable in play? |
| --- | --- | --- |
| 1 Definition | `UDungeonCharacterSheetData` | No |
| 2 Spawn | `FCharacterSpawnPayload` → `FCharacterCombatSnapshot` | Built at spawn |
| 3 Unit runtime | `FCharacterUnitRuntimeState` on `UCharacterUnitRuntimeComponent` | Yes (HP, CDs) |
| 3 Party runtime | `FInvaderPartyRuntimeState` on `UInvaderPartyStateComponent` | Yes |

## Registry

`UCharacterSheetSubsystem` (GameInstance):

- On init: load `../Data/character-sheets.json` relative to `.uproject` (repo `Data/` when project lives in `DungeonCore/`).
- Fallback: `SeedDefaultSheets()` minimal ids.

## Blueprint use

1. `Get Game Instance` → `Get Subsystem (CharacterSheetSubsystem)` → `GetSheetById`.
2. `BuildSnapshotForSheet` with `SpawnPowerScalar` / `LinePowerScalar`.
3. `CharacterUnitRuntimeComponent::InitializeFromSheet`.
4. Party anchor actor: `InvaderPartyStateComponent::InitializeParty`.

## Files

- `Source/DungeonCore/Public/Character/*`
- `Source/DungeonCore/Private/Character/*`