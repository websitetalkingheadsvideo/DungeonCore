---
name: fab-asset-migration
description: Migrate meshes, materials and textures from the locally installed Fab packs under H:\Unreal Assets into the Dungeon Core project Content folder with all dependencies, then wire them into C++ via ConstructorHelpers. Use when replacing placeholder primitives with library assets, when library.md is referenced, or when a /Game/<Pack>/... asset is needed in code.
---

# Fab Asset Migration (Dungeon Core)

Packs live under `H:\Unreal Assets\<PackId>\` but the `.uproject` is often **nested** and its name
**does not match** `<PackId>` (inventory zip names are a fourth alias). Use `tools/asset-library/pack-roots.json`
(`contentPath`, `contentMounts`) — refresh with `Sync-PackRoots.ps1` after new installs.

`library.md` (repo root) and `H:\Unreal Assets\asset-pack-inventory.md` describe packs by **catalog id**
(top-level folder). Project content is git-tracked, so only migrate what is actually used.

## Why copy instead of reference

`.uasset` files embed `/Game/...` import paths. Copying a package *and* every dependency to the same
`/Game`-relative location inside `DungeonCore/Content/` makes those paths resolve with no redirectors
or fix-ups. Packs are UE 5.2–5.5 and load forward into 5.8.

## Workflow

```
- [ ] 1. Pick assets via `asset-library-lookup` (`search_library.py`) or library.md picks (prefer MI_* and SM_*; never maps)
- [ ] 2. Dry-run the dependency crawl and check size + unresolved refs
- [ ] 3. Copy
- [ ] 4. Wire in C++ with ConstructorHelpers::FObjectFinder (editor closed, then Build.bat)
- [ ] 5. Launch -game, grep the log for load failures, screenshot, look at it
```

**Step 2–3: crawl and copy.** Run the script in-session with `&` (not `powershell -File`; that mangles
the `-Seeds` array). Pipe to `Out-Null` or a file if you want to suppress the listing; a trailing
`Select-Object -First 1` aborts the pipeline before the copy happens.

```powershell
# Resolve contentPath + print migrate line (or run it):
& "tools/asset-library/Get-MigratePackCommand.ps1" -PackId Crystal_Cave `
    -Seeds @("/Game/Crystal_Cave/Meshes/SM_Crystal_D_01", "/Game/Crystal_Cave/Materials/MI_GlowCrystalls") `
    -Execute -DryRun

# Manual (contentPath from pack-roots.json, not always <PackId>\Content):
& ".cursor/skills/fab-asset-migration/scripts/migrate-pack-assets.ps1" `
    -PackContentRoot "H:\Unreal Assets\Crystal_Cave\Content" `
    -Seeds @("/Game/Crystal_Cave/Meshes/SM_Crystal_D_01", "/Game/Crystal_Cave/Materials/MI_GlowCrystalls") `
    -DestContentRoot "E:\Dungeon Core\DungeonCore\Content" `
    -DryRun
```

Drop `-DryRun` to copy. Exit codes: `0` ok, `2` destination exists with a different file size (not
overwritten — inspect before forcing). Unresolved refs are listed; `/Game/...` refs that are only
default texture parameters on a master material (for example `T_Fill_*`) are harmless when the copied
instance overrides them. Anything else unresolved must be seeded from its own pack.

**Step 4: wire it.** Paths are `/Game/<Pack>/<Folder>/<Asset>.<Asset>`. Use the existing helpers in
`DungeonChunkActor.cpp` (`AssignLibraryMesh`, `AssignLibraryMaterial`); they log an `Error` if the
finder fails instead of falling back to a primitive. Adding `UPROPERTY` components needs a cold build
(`Build.bat DungeonCoreEditor Win64 Development -Project=... -WaitMutex -NoHotReload`), which fails
while the editor has Live Coding active.

**Finding material parameter names** without opening the editor: read the `.uasset` as ASCII and grep
for identifiers. Example from `MI_GlowCrystalls`: `Out_Color`, `Inner_Color`, `Deep_Paralax_Color`,
`Inner_Glow`, `Power`. Drive them with a `UMaterialInstanceDynamic` created from the MI.

```powershell
$s = [Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes("...\MI_GlowCrystalls.uasset"))
[regex]::Matches($s, '[A-Za-z_][A-Za-z0-9_]{3,40}') | % Value | Sort-Object -Unique
```

**Step 5: verify.** First launch after migration compiles shaders and builds textures (minutes). Then:

```powershell
Select-String "DungeonCore\Saved\Logs\DungeonCore.log" -Pattern "library (mesh|material) missing|Failed to find|Failed to load"
```

Take an in-engine `HighResShot` via `-ExecCmds` and read the image; a claimed fix that is not visible
in PIE does not count. Delete `Saved\Screenshots\WindowsEditor\HighresScreenshot*.png` afterwards.

## Gotchas

- Engine cube UVs are 0–1 per face: a tiled material shows one tile per 300 uu block, and stretched on
  non-square walls. Scale UVs in the MI or use a pack mesh instead of the engine cube if it looks wrong.
- Pack meshes have arbitrary pivots/orientation (`SM_Crystal_D_01` is authored point-down). Lay out
  from `UStaticMesh::GetBounds()` and rotate in local space; do not hard-code offsets.
- Substrate is on in this project; Leartes masters compile fine but the first load is slow.
- Never copy `/Game/Developers`, `/Game/Collections`, `__ExternalActors__`, or `.umap` files.
