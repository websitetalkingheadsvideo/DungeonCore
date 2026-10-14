#include "Sim/DungeonSimSubsystem.h"

#include "Data/DungeonContentSubsystem.h"
#include "Save/DungeonRunSaveGame.h"
#include "Sim/Grid/DungeonGridState.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "TimerManager.h"

void UDungeonSimSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    EnsureGridState();
}

void UDungeonSimSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WaveStartTimerHandle);
        World->GetTimerManager().ClearTimer(AdventurerAdvanceTimerHandle);
        World->GetTimerManager().ClearTimer(DefenderSpawnTimerHandle);
    }

    RoomInstances.Reset();
    TrapInstances.Reset();
    ActiveAdventurers.Reset();
    ActiveDefenders.Reset();
    GridState = nullptr;

    Super::Deinitialize();
}

void UDungeonSimSubsystem::StartNewRun(const FRunSetup& InRunSetup)
{
    EnsureGridState();

    RunSetup = InRunSetup;
    ResourceState.CurrentMana = DungeonCore::StartingMana;
    ResourceState.CurrentDungeonPoints = 0;
    ResourceState.BaseManaPerSecond = 0.4f;

    FloorState.FloorIndex = RunSetup.StartingFloorIndex;
    FloorState.OutsideEdgeDirection = EDungeonEdgeDirection::East;
    FloorState.BreachState = EBreachState::Sealed;
    FloorState.CoreBlock = DungeonCore::GetStartingCoreBlockCoord();
    FloorState.SurfaceEntryBlock = FBlockCoord(
        DungeonCore::StartingCoreChamberWidthBlocks - 1,
        FloorState.CoreBlock.Y,
        0
    );

    RoomInstances.Reset();
    TrapInstances.Reset();
    ActiveAdventurers.Reset();
    ActiveDefenders.Reset();
    CurrentWaveNumber = 0;
    CurrentCoreHealth = 100;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WaveStartTimerHandle);
        World->GetTimerManager().ClearTimer(AdventurerAdvanceTimerHandle);
        World->GetTimerManager().ClearTimer(DefenderSpawnTimerHandle);
    }

    SeedFirstFloor();

    OnResourceChanged.Broadcast(ResourceState);
    OnBreachStateChanged.Broadcast(FloorState.BreachState);
    OnWaveChanged.Broadcast(CurrentWaveNumber);
    OnCoreHealthChanged.Broadcast(CurrentCoreHealth);
    OnAdventurersChanged.Broadcast();
}

bool UDungeonSimSubsystem::AddMana(const int32 Amount)
{
    if (Amount <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("AddMana requires a positive amount."));
        return false;
    }

    ResourceState.CurrentMana += Amount;
    OnResourceChanged.Broadcast(ResourceState);
    return true;
}

bool UDungeonSimSubsystem::SpendMana(const int32 Amount)
{
    if (Amount <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SpendMana requires a positive amount."));
        return false;
    }

    if (ResourceState.CurrentMana < Amount)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpendMana failed because the dungeon does not have enough mana."));
        return false;
    }

    ResourceState.CurrentMana -= Amount;
    OnResourceChanged.Broadcast(ResourceState);
    return true;
}

FPlacementResult UDungeonSimSubsystem::DigBlock(const FBlockCoord& BlockCoord)
{
    EnsureGridState();

    const int32 DigManaCost = GetDigManaCost();
    FPlacementResult Result = GridState->ValidateDig(BlockCoord, ResourceState.CurrentMana, DigManaCost);
    if (!Result.bSuccess)
    {
        return Result;
    }

    const bool bSpendSucceeded = SpendMana(DigManaCost);
    if (!bSpendSucceeded)
    {
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    const bool bOpensSurface = BlockCoord == FloorState.SurfaceEntryBlock;
    const bool bApplySucceeded = GridState->ApplyDig(BlockCoord, bOpensSurface);
    if (!bApplySucceeded)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyDig failed for block (%d, %d, %d)."), BlockCoord.X, BlockCoord.Y, BlockCoord.Z);
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::Blocked;
        return Result;
    }

    if (bOpensSurface)
    {
        FloorState.BreachState = EBreachState::Open;
        OnBreachStateChanged.Broadcast(FloorState.BreachState);
        ScheduleNextWave(45.0f);

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AdventurerAdvanceTimerHandle,
                this,
                &UDungeonSimSubsystem::AdvanceAdventurers,
                1.0f,
                true
            );

            World->GetTimerManager().SetTimer(
                DefenderSpawnTimerHandle,
                this,
                &UDungeonSimSubsystem::SpawnDefenders,
                5.0f,
                true
            );
        }
    }

    BroadcastTileAt(BlockCoord);
    return Result;
}

FPlacementResult UDungeonSimSubsystem::PlaceRoom(
    const FBlockCoord& OriginBlock,
    const ERoomTypeId RoomTypeId,
    const FRoomFootprint& Footprint,
    const int32 ManaCost
)
{
    EnsureGridState();

    FPlacementResult Result = GridState->ValidateRoomPlacement(OriginBlock, Footprint, ResourceState.CurrentMana, ManaCost);
    if (!Result.bSuccess)
    {
        return Result;
    }

    const bool bSpendSucceeded = SpendMana(ManaCost);
    if (!bSpendSucceeded)
    {
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    FRoomInstanceData RoomInstanceData;
    RoomInstanceData.RoomInstanceId = FGuid::NewGuid();
    RoomInstanceData.RoomTypeId = RoomTypeId;
    RoomInstanceData.OriginBlock = OriginBlock;
    RoomInstanceData.Footprint = Footprint;
    RoomInstanceData.OwningFamily = RunSetup.CreatureFamilyId;
    RoomInstanceData.HitPoints = 100;

    const bool bApplySucceeded = GridState->ApplyRoomPlacement(RoomInstanceData);
    if (!bApplySucceeded)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyRoomPlacement failed for origin block (%d, %d, %d)."), OriginBlock.X, OriginBlock.Y, OriginBlock.Z);
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::Blocked;
        return Result;
    }

    RoomInstances.Add(RoomInstanceData.RoomInstanceId, RoomInstanceData);
    for (const FBlockCoord& BlockCoord : Result.AffectedBlocks)
    {
        BroadcastTileAt(BlockCoord);
    }

    return Result;
}

FPlacementResult UDungeonSimSubsystem::PlaceTrap(
    const FBlockCoord& OriginBlock,
    const ETrapTypeId TrapTypeId,
    const int32 ManaCost
)
{
    EnsureGridState();

    FPlacementResult Result = GridState->ValidateTrapPlacement(OriginBlock, ResourceState.CurrentMana, ManaCost);
    if (!Result.bSuccess)
    {
        return Result;
    }

    const bool bSpendSucceeded = SpendMana(ManaCost);
    if (!bSpendSucceeded)
    {
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    FTrapInstanceData TrapInstanceData;
    TrapInstanceData.TrapInstanceId = FGuid::NewGuid();
    TrapInstanceData.TrapTypeId = TrapTypeId;
    TrapInstanceData.OriginBlock = OriginBlock;
    TrapInstanceData.CooldownRemaining = 0.0f;
    TrapInstanceData.OwningFamily = RunSetup.CreatureFamilyId;

    const bool bApplySucceeded = GridState->ApplyTrapPlacement(TrapInstanceData);
    if (!bApplySucceeded)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyTrapPlacement failed for block (%d, %d, %d)."), OriginBlock.X, OriginBlock.Y, OriginBlock.Z);
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::Blocked;
        return Result;
    }

    TrapInstances.Add(TrapInstanceData.TrapInstanceId, TrapInstanceData);
    BroadcastTileAt(OriginBlock);
    return Result;
}

bool UDungeonSimSubsystem::SaveCurrentRun(const FString& SlotName)
{
    EnsureGridState();

    UDungeonRunSaveGame* SaveGame = Cast<UDungeonRunSaveGame>(UGameplayStatics::CreateSaveGameObject(UDungeonRunSaveGame::StaticClass()));
    if (SaveGame == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create dungeon save game object."));
        return false;
    }

    SaveGame->RunSetup = RunSetup;
    SaveGame->ResourceState = ResourceState;
    SaveGame->FloorState = FloorState;
    SaveGame->Tiles = GridState->GetAllTiles();
    SaveGame->CurrentWaveNumber = CurrentWaveNumber;
    SaveGame->CurrentCoreHealth = CurrentCoreHealth;
    SaveGame->ActiveAdventurers = ActiveAdventurers;
    SaveGame->ActiveDefenders = ActiveDefenders;

    SaveGame->Rooms.Reserve(RoomInstances.Num());
    for (const TPair<FGuid, FRoomInstanceData>& RoomEntry : RoomInstances)
    {
        SaveGame->Rooms.Add(RoomEntry.Value);
    }

    SaveGame->Traps.Reserve(TrapInstances.Num());
    for (const TPair<FGuid, FTrapInstanceData>& TrapEntry : TrapInstances)
    {
        SaveGame->Traps.Add(TrapEntry.Value);
    }

    return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
}

bool UDungeonSimSubsystem::LoadSavedRun(const FString& SlotName)
{
    EnsureGridState();

    USaveGame* RawSaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
    UDungeonRunSaveGame* SaveGame = Cast<UDungeonRunSaveGame>(RawSaveGame);
    if (SaveGame == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("No dungeon save was found in slot '%s'."), *SlotName);
        return false;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WaveStartTimerHandle);
        World->GetTimerManager().ClearTimer(AdventurerAdvanceTimerHandle);
        World->GetTimerManager().ClearTimer(DefenderSpawnTimerHandle);
    }

    RunSetup = SaveGame->RunSetup;
    ResourceState = SaveGame->ResourceState;
    FloorState = SaveGame->FloorState;
    CurrentWaveNumber = SaveGame->CurrentWaveNumber;
    CurrentCoreHealth = SaveGame->CurrentCoreHealth;
    ActiveAdventurers = SaveGame->ActiveAdventurers;
    ActiveDefenders = SaveGame->ActiveDefenders;

    RoomInstances.Reset();
    for (const FRoomInstanceData& RoomData : SaveGame->Rooms)
    {
        RoomInstances.Add(RoomData.RoomInstanceId, RoomData);
    }

    TrapInstances.Reset();
    for (const FTrapInstanceData& TrapData : SaveGame->Traps)
    {
        TrapInstances.Add(TrapData.TrapInstanceId, TrapData);
    }

    GridState->ResetGrid();
    for (const FTileData& TileData : SaveGame->Tiles)
    {
        GridState->SetTileData(TileData);
    }

    RestoreSimulationTimers();
    BroadcastAllTiles();
    OnResourceChanged.Broadcast(ResourceState);
    OnBreachStateChanged.Broadcast(FloorState.BreachState);
    OnWaveChanged.Broadcast(CurrentWaveNumber);
    OnCoreHealthChanged.Broadcast(CurrentCoreHealth);
    OnAdventurersChanged.Broadcast();
    return true;
}

void UDungeonSimSubsystem::RestartCurrentRun()
{
    StartNewRun(RunSetup);
}

FPlacementResult UDungeonSimSubsystem::PreviewDigAtBlock(const FBlockCoord& BlockCoord) const
{
    if (GridState == nullptr)
    {
        FPlacementResult Result;
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    return GridState->ValidateDig(BlockCoord, ResourceState.CurrentMana, GetDigManaCost());
}

FPlacementResult UDungeonSimSubsystem::PreviewRoomPlacement(
    const FBlockCoord& OriginBlock,
    const FRoomFootprint& Footprint,
    const int32 ManaCost
) const
{
    if (GridState == nullptr)
    {
        FPlacementResult Result;
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    return GridState->ValidateRoomPlacement(OriginBlock, Footprint, ResourceState.CurrentMana, ManaCost);
}

FPlacementResult UDungeonSimSubsystem::PreviewTrapPlacement(const FBlockCoord& OriginBlock, const int32 ManaCost) const
{
    if (GridState == nullptr)
    {
        FPlacementResult Result;
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    return GridState->ValidateTrapPlacement(OriginBlock, ResourceState.CurrentMana, ManaCost);
}

FResourceState UDungeonSimSubsystem::GetResourceState() const
{
    return ResourceState;
}

FFloorState UDungeonSimSubsystem::GetFloorState() const
{
    return FloorState;
}

FRunSetup UDungeonSimSubsystem::GetRunSetup() const
{
    return RunSetup;
}

UDungeonGridState* UDungeonSimSubsystem::GetGridState() const
{
    return GridState;
}

ERoomTypeId UDungeonSimSubsystem::GetActiveStarterSpawnerRoomType() const
{
    return GetStarterSpawnerRoomType(RunSetup.CreatureFamilyId);
}

ETrapTypeId UDungeonSimSubsystem::GetActiveStarterTrapType() const
{
    return GetStarterTrapType(RunSetup.CreatureFamilyId);
}

int32 UDungeonSimSubsystem::GetCurrentWaveNumber() const
{
    return CurrentWaveNumber;
}

int32 UDungeonSimSubsystem::GetCurrentCoreHealth() const
{
    return CurrentCoreHealth;
}

TArray<FUnitInstanceData> UDungeonSimSubsystem::GetActiveAdventurers() const
{
    return ActiveAdventurers;
}

TArray<FUnitInstanceData> UDungeonSimSubsystem::GetActiveDefenders() const
{
    return ActiveDefenders;
}

int32 UDungeonSimSubsystem::GetDigManaCost() const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        return ContentSubsystem->GetDigManaCost();
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for dig cost lookup."));
    return 0;
}

int32 UDungeonSimSubsystem::GetManaWellCost() const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildableByRoomType(ERoomTypeId::ManaWell))
        {
            return Row->ManaCost;
        }

        UE_LOG(LogTemp, Fatal, TEXT("Mana Well buildable row is required."));
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for Mana Well cost lookup."));
    return 0;
}

int32 UDungeonSimSubsystem::GetStarterSpawnerCost() const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildableByRoomType(GetActiveStarterSpawnerRoomType()))
        {
            return Row->ManaCost;
        }

        UE_LOG(LogTemp, Fatal, TEXT("Starter spawner buildable row is required."));
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for starter spawner cost lookup."));
    return 0;
}

int32 UDungeonSimSubsystem::GetStarterTrapCost() const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildableByTrapType(GetActiveStarterTrapType()))
        {
            return Row->ManaCost;
        }

        UE_LOG(LogTemp, Fatal, TEXT("Starter trap buildable row is required."));
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for starter trap cost lookup."));
    return 0;
}

void UDungeonSimSubsystem::EnsureGridState()
{
    if (GridState == nullptr)
    {
        GridState = NewObject<UDungeonGridState>(this);
        GridState->InitializeGrid(DungeonCore::DefaultChunkSizeInBlocks);
    }
}

void UDungeonSimSubsystem::SeedFirstFloor()
{
    GridState->ResetGrid();

    const int32 ChamberWidth = DungeonCore::StartingCoreChamberWidthBlocks;
    const int32 ChamberHeight = DungeonCore::StartingCoreChamberHeightBlocks;
    const int32 ZLevel = 0;
    const int32 RockMargin = DungeonCore::StartingRockMarginBlocks;
    const int32 MinX = -RockMargin;
    const int32 MaxX = ChamberWidth + RockMargin - 1;
    const int32 MinY = -RockMargin;
    const int32 MaxY = ChamberHeight + RockMargin - 1;

    for (int32 X = MinX; X <= MaxX; ++X)
    {
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            GridState->SetTileData(MakeTileData(FBlockCoord(X, Y, ZLevel), ETileState::SolidRock, false, true));
        }
    }

    FRoomFootprint CoreChamberFootprint;
    CoreChamberFootprint.Width = ChamberWidth;
    CoreChamberFootprint.Height = ChamberHeight;
    CoreChamberFootprint.Depth = 1;
    const FBlockCoord CoreChamberOrigin(0, 0, ZLevel);
    const TArray<FBlockCoord> CoreChamberBlocks = GridState->GetBlocksForFootprint(
        CoreChamberOrigin,
        CoreChamberFootprint
    );

    FRoomInstanceData CoreRoom;
    CoreRoom.RoomInstanceId = FGuid::NewGuid();
    CoreRoom.RoomTypeId = ERoomTypeId::CoreChamber;
    CoreRoom.OriginBlock = CoreChamberOrigin;
    CoreRoom.Footprint = CoreChamberFootprint;
    CoreRoom.OwningFamily = RunSetup.CreatureFamilyId;
    CoreRoom.HitPoints = 500;

    const bool bCorePlaced = GridState->ApplyRoomPlacement(CoreRoom);
    if (!bCorePlaced)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Failed to place the starting core chamber."));
    }

    RoomInstances.Add(CoreRoom.RoomInstanceId, CoreRoom);

    if (FTileData* SurfaceEntryTile = GridState->FindMutableTileData(FloorState.SurfaceEntryBlock))
    {
        SurfaceEntryTile->TileState = ETileState::SurfaceEntry;
        SurfaceEntryTile->TrapTypeId = ETrapTypeId::None;
        SurfaceEntryTile->bWalkable = true;
        SurfaceEntryTile->bDiggable = false;
    }

    for (const FBlockCoord& BlockCoord : CoreChamberBlocks)
    {
        BroadcastTileAt(BlockCoord);
    }

    BroadcastTileAt(FloorState.SurfaceEntryBlock);
}

FTileData UDungeonSimSubsystem::MakeTileData(
    const FBlockCoord& BlockCoord,
    const ETileState TileState,
    const bool bWalkable,
    const bool bDiggable
) const
{
    FTileData TileData;
    TileData.BlockCoord = BlockCoord;
    TileData.TileState = TileState;
    TileData.RoomTypeId = ERoomTypeId::None;
    TileData.TrapTypeId = ETrapTypeId::None;
    TileData.bWalkable = bWalkable;
    TileData.bDiggable = bDiggable;
    TileData.OwningChunk = MakeChunkCoordForBlock(BlockCoord);
    return TileData;
}

FChunkCoord UDungeonSimSubsystem::MakeChunkCoordForBlock(const FBlockCoord& BlockCoord) const
{
    return FChunkCoord(
        FMath::FloorToInt(static_cast<float>(BlockCoord.X) / static_cast<float>(DungeonCore::DefaultChunkSizeInBlocks)),
        FMath::FloorToInt(static_cast<float>(BlockCoord.Y) / static_cast<float>(DungeonCore::DefaultChunkSizeInBlocks)),
        FMath::FloorToInt(static_cast<float>(BlockCoord.Z) / static_cast<float>(DungeonCore::DefaultChunkSizeInBlocks))
    );
}

void UDungeonSimSubsystem::BroadcastTileAt(const FBlockCoord& BlockCoord)
{
    const FTileData* TileData = GridState != nullptr ? GridState->FindTileData(BlockCoord) : nullptr;
    if (TileData != nullptr)
    {
        OnTileChanged.Broadcast(*TileData);
    }
}

void UDungeonSimSubsystem::BroadcastAllTiles()
{
    if (GridState == nullptr)
    {
        return;
    }

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    for (const FTileData& TileData : Tiles)
    {
        OnTileChanged.Broadcast(TileData);
    }
}

void UDungeonSimSubsystem::RestoreSimulationTimers()
{
    if (FloorState.BreachState != EBreachState::Open)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AdventurerAdvanceTimerHandle,
            this,
            &UDungeonSimSubsystem::AdvanceAdventurers,
            1.0f,
            true
        );

        World->GetTimerManager().SetTimer(
            DefenderSpawnTimerHandle,
            this,
            &UDungeonSimSubsystem::SpawnDefenders,
            5.0f,
            true
        );
    }

    ScheduleNextWave(45.0f);
}

void UDungeonSimSubsystem::ScheduleNextWave(const float DelaySeconds)
{
    if (FloorState.BreachState != EBreachState::Open)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            WaveStartTimerHandle,
            this,
            &UDungeonSimSubsystem::StartNextWave,
            DelaySeconds,
            false
        );
    }
}

void UDungeonSimSubsystem::StartNextWave()
{
    if (FloorState.BreachState != EBreachState::Open)
    {
        return;
    }

    CurrentWaveNumber += 1;
    OnWaveChanged.Broadcast(CurrentWaveNumber);

    const int32 WaveSize = 3 + FMath::Max(0, CurrentWaveNumber - 1);
    for (int32 UnitIndex = 0; UnitIndex < WaveSize; ++UnitIndex)
    {
        FUnitInstanceData AdventurerData;
        AdventurerData.UnitInstanceId = FGuid::NewGuid();
        AdventurerData.UnitTypeId = UnitIndex % 3 == 2 ? EUnitTypeId::AdventurerRanged : EUnitTypeId::AdventurerMelee;
        AdventurerData.CurrentBlock = FloorState.SurfaceEntryBlock;
        AdventurerData.CurrentHealth = 10;
        AdventurerData.bIsAdventurer = true;
        AdventurerData.Damage = AdventurerData.UnitTypeId == EUnitTypeId::AdventurerRanged ? 8 : 10;
        AdventurerData.PathStepIndex = 0;
        AdventurerData.CurrentPath = BuildPathToCore(AdventurerData.CurrentBlock);

        ActiveAdventurers.Add(AdventurerData);
    }

    OnAdventurersChanged.Broadcast();
    ScheduleNextWave(45.0f);
}

void UDungeonSimSubsystem::AdvanceAdventurers()
{
    if (FloorState.BreachState != EBreachState::Open || GridState == nullptr)
    {
        return;
    }

    bool bAnyStateChanged = false;

    for (int32 DefenderIndex = ActiveDefenders.Num() - 1; DefenderIndex >= 0; --DefenderIndex)
    {
        FUnitInstanceData& DefenderData = ActiveDefenders[DefenderIndex];

        int32 ClosestAdventurerIndex = INDEX_NONE;
        TArray<FBlockCoord> ClosestPath;

        for (int32 AdventurerIndex = 0; AdventurerIndex < ActiveAdventurers.Num(); ++AdventurerIndex)
        {
            const TArray<FBlockCoord> CandidatePath = BuildPathToBlock(DefenderData.CurrentBlock, ActiveAdventurers[AdventurerIndex].CurrentBlock);
            if (CandidatePath.Num() == 0)
            {
                continue;
            }

            if (ClosestAdventurerIndex == INDEX_NONE || CandidatePath.Num() < ClosestPath.Num())
            {
                ClosestAdventurerIndex = AdventurerIndex;
                ClosestPath = CandidatePath;
            }
        }

        if (ClosestAdventurerIndex == INDEX_NONE)
        {
            continue;
        }

        if (ClosestPath.Num() <= 2)
        {
            ActiveAdventurers[ClosestAdventurerIndex].CurrentHealth -= 10;
            DefenderData.CurrentHealth -= 4;
            bAnyStateChanged = true;

            if (ActiveAdventurers[ClosestAdventurerIndex].CurrentHealth <= 0)
            {
                ActiveAdventurers.RemoveAt(ClosestAdventurerIndex);
                AddMana(8);
            }

            if (DefenderData.CurrentHealth <= 0)
            {
                ActiveDefenders.RemoveAt(DefenderIndex);
            }

            continue;
        }

        DefenderData.CurrentBlock = ClosestPath[1];
        DefenderData.CurrentPath = ClosestPath;
        DefenderData.PathStepIndex = 1;
        bAnyStateChanged = true;
    }

    for (int32 UnitIndex = ActiveAdventurers.Num() - 1; UnitIndex >= 0; --UnitIndex)
    {
        FUnitInstanceData& AdventurerData = ActiveAdventurers[UnitIndex];

        if (AdventurerData.StatusTags.Contains(TEXT("Slowed")))
        {
            AdventurerData.StatusTags.RemoveSingle(TEXT("Slowed"));
            bAnyStateChanged = true;
            continue;
        }

        AdventurerData.CurrentPath = BuildPathToCore(AdventurerData.CurrentBlock);

        if (AdventurerData.CurrentPath.Num() == 0)
        {
            continue;
        }

        if (AdventurerData.CurrentPath.Num() == 1)
        {
            const bool bRemoveAtCurrentBlock = HandleAdventurerTileEffects(AdventurerData);
            if (bRemoveAtCurrentBlock)
            {
                ActiveAdventurers.RemoveAt(UnitIndex);
                bAnyStateChanged = true;
            }

            continue;
        }

        AdventurerData.PathStepIndex = 1;
        AdventurerData.CurrentBlock = AdventurerData.CurrentPath[AdventurerData.PathStepIndex];
        bAnyStateChanged = true;

        const bool bRemoveAfterMove = HandleAdventurerTileEffects(AdventurerData);
        if (bRemoveAfterMove)
        {
            ActiveAdventurers.RemoveAt(UnitIndex);
        }
    }

    if (bAnyStateChanged)
    {
        OnAdventurersChanged.Broadcast();
    }
}

void UDungeonSimSubsystem::SpawnDefenders()
{
    if (FloorState.BreachState != EBreachState::Open)
    {
        return;
    }

    const ERoomTypeId StarterSpawnerType = GetStarterSpawnerRoomType(RunSetup.CreatureFamilyId);
    const EUnitTypeId StarterDefenderType = GetStarterDefenderType(RunSetup.CreatureFamilyId);
    bool bSpawnedAnyDefender = false;

    for (const TPair<FGuid, FRoomInstanceData>& RoomEntry : RoomInstances)
    {
        const FRoomInstanceData& RoomInstanceData = RoomEntry.Value;
        if (RoomInstanceData.RoomTypeId != StarterSpawnerType)
        {
            continue;
        }

        int32 ExistingDefenderCountForSpawner = 0;
        for (const FUnitInstanceData& DefenderData : ActiveDefenders)
        {
            if (DefenderData.HomeBlock == RoomInstanceData.OriginBlock)
            {
                ExistingDefenderCountForSpawner += 1;
            }
        }

        if (ExistingDefenderCountForSpawner >= 1)
        {
            continue;
        }

        FUnitInstanceData DefenderData;
        DefenderData.UnitInstanceId = FGuid::NewGuid();
        DefenderData.UnitTypeId = StarterDefenderType;
        DefenderData.CurrentBlock = RoomInstanceData.OriginBlock;
        DefenderData.HomeBlock = RoomInstanceData.OriginBlock;
        DefenderData.CurrentHealth = GetDefenderMaxHealth(StarterDefenderType);
        DefenderData.bIsAdventurer = false;
        DefenderData.Damage = GetDefenderDamage(StarterDefenderType);
        DefenderData.PathStepIndex = 0;

        ActiveDefenders.Add(DefenderData);
        bSpawnedAnyDefender = true;
    }

    if (bSpawnedAnyDefender)
    {
        OnAdventurersChanged.Broadcast();
    }
}

TArray<FBlockCoord> UDungeonSimSubsystem::BuildPathToCore(const FBlockCoord& StartBlock) const
{
    TArray<FBlockCoord> EmptyPath;
    if (GridState == nullptr)
    {
        return EmptyPath;
    }

    const FTileData* StartTile = GridState->FindTileData(StartBlock);
    if (StartTile == nullptr || !StartTile->bWalkable)
    {
        return EmptyPath;
    }

    TArray<FBlockCoord> Frontier;
    Frontier.Add(StartBlock);

    TMap<FBlockCoord, FBlockCoord> CameFrom;
    CameFrom.Add(StartBlock, StartBlock);

    int32 FrontierIndex = 0;
    FBlockCoord GoalBlock = StartBlock;
    bool bFoundGoal = false;

    const TArray<FBlockCoord> NeighborOffsets = {
        FBlockCoord(1, 0, 0),
        FBlockCoord(-1, 0, 0),
        FBlockCoord(0, 1, 0),
        FBlockCoord(0, -1, 0)
    };

    while (FrontierIndex < Frontier.Num())
    {
        const FBlockCoord CurrentBlock = Frontier[FrontierIndex];
        FrontierIndex += 1;

        const FTileData* CurrentTile = GridState->FindTileData(CurrentBlock);
        if (CurrentTile != nullptr && CurrentTile->RoomTypeId == ERoomTypeId::CoreChamber)
        {
            GoalBlock = CurrentBlock;
            bFoundGoal = true;
            break;
        }

        for (const FBlockCoord& Offset : NeighborOffsets)
        {
            const FBlockCoord NeighborBlock = CurrentBlock + Offset;
            if (CameFrom.Contains(NeighborBlock))
            {
                continue;
            }

            const FTileData* NeighborTile = GridState->FindTileData(NeighborBlock);
            if (NeighborTile == nullptr || !NeighborTile->bWalkable)
            {
                continue;
            }

            Frontier.Add(NeighborBlock);
            CameFrom.Add(NeighborBlock, CurrentBlock);
        }
    }

    if (!bFoundGoal)
    {
        return EmptyPath;
    }

    TArray<FBlockCoord> ReversePath;
    FBlockCoord CurrentBlock = GoalBlock;
    ReversePath.Add(CurrentBlock);

    while (!(CurrentBlock == StartBlock))
    {
        CurrentBlock = CameFrom[CurrentBlock];
        ReversePath.Add(CurrentBlock);
    }

    TArray<FBlockCoord> Path;
    for (int32 Index = ReversePath.Num() - 1; Index >= 0; --Index)
    {
        Path.Add(ReversePath[Index]);
    }

    return Path;
}

TArray<FBlockCoord> UDungeonSimSubsystem::BuildPathToBlock(
    const FBlockCoord& StartBlock,
    const FBlockCoord& GoalBlock
) const
{
    TArray<FBlockCoord> EmptyPath;
    if (GridState == nullptr)
    {
        return EmptyPath;
    }

    const FTileData* StartTile = GridState->FindTileData(StartBlock);
    const FTileData* GoalTile = GridState->FindTileData(GoalBlock);
    if (StartTile == nullptr || GoalTile == nullptr || !StartTile->bWalkable || !GoalTile->bWalkable)
    {
        return EmptyPath;
    }

    TArray<FBlockCoord> Frontier;
    Frontier.Add(StartBlock);

    TMap<FBlockCoord, FBlockCoord> CameFrom;
    CameFrom.Add(StartBlock, StartBlock);

    int32 FrontierIndex = 0;
    bool bFoundGoal = false;

    const TArray<FBlockCoord> NeighborOffsets = {
        FBlockCoord(1, 0, 0),
        FBlockCoord(-1, 0, 0),
        FBlockCoord(0, 1, 0),
        FBlockCoord(0, -1, 0)
    };

    while (FrontierIndex < Frontier.Num())
    {
        const FBlockCoord CurrentBlock = Frontier[FrontierIndex];
        FrontierIndex += 1;

        if (CurrentBlock == GoalBlock)
        {
            bFoundGoal = true;
            break;
        }

        for (const FBlockCoord& Offset : NeighborOffsets)
        {
            const FBlockCoord NeighborBlock = CurrentBlock + Offset;
            if (CameFrom.Contains(NeighborBlock))
            {
                continue;
            }

            const FTileData* NeighborTile = GridState->FindTileData(NeighborBlock);
            if (NeighborTile == nullptr || !NeighborTile->bWalkable)
            {
                continue;
            }

            Frontier.Add(NeighborBlock);
            CameFrom.Add(NeighborBlock, CurrentBlock);
        }
    }

    if (!bFoundGoal)
    {
        return EmptyPath;
    }

    TArray<FBlockCoord> ReversePath;
    FBlockCoord CurrentBlock = GoalBlock;
    ReversePath.Add(CurrentBlock);

    while (!(CurrentBlock == StartBlock))
    {
        CurrentBlock = CameFrom[CurrentBlock];
        ReversePath.Add(CurrentBlock);
    }

    TArray<FBlockCoord> Path;
    for (int32 Index = ReversePath.Num() - 1; Index >= 0; --Index)
    {
        Path.Add(ReversePath[Index]);
    }

    return Path;
}

bool UDungeonSimSubsystem::HandleAdventurerTileEffects(FUnitInstanceData& AdventurerData)
{
    if (GridState == nullptr)
    {
        return false;
    }

    const FTileData* TileData = GridState->FindTileData(AdventurerData.CurrentBlock);
    if (TileData == nullptr)
    {
        return false;
    }

    if (TileData->TileState == ETileState::Trap)
    {
        switch (TileData->TrapTypeId)
        {
        case ETrapTypeId::GraveGrasp:
            AddMana(10);
            return true;

        case ETrapTypeId::SpikeBarricade:
            AdventurerData.CurrentHealth -= 12;
            if (AdventurerData.CurrentHealth <= 0)
            {
                AddMana(8);
                return true;
            }

            return false;

        case ETrapTypeId::WebSnare:
            if (!AdventurerData.StatusTags.Contains(TEXT("Slowed")))
            {
                AdventurerData.StatusTags.Add(TEXT("Slowed"));
            }

            AdventurerData.CurrentHealth -= 4;
            if (AdventurerData.CurrentHealth <= 0)
            {
                AddMana(8);
                return true;
            }

            return false;

        case ETrapTypeId::PixieDustCloud:
            AdventurerData.CurrentBlock = FloorState.SurfaceEntryBlock;
            AdventurerData.CurrentHealth -= 2;
            if (AdventurerData.CurrentHealth <= 0)
            {
                AddMana(8);
                return true;
            }

            return false;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled trap type for adventurer tile effects."));
            return false;
        }
    }

    if (TileData->RoomTypeId == ERoomTypeId::CoreChamber)
    {
        CurrentCoreHealth = FMath::Max(0, CurrentCoreHealth - AdventurerData.Damage);
        OnCoreHealthChanged.Broadcast(CurrentCoreHealth);

        if (CurrentCoreHealth == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("The dungeon core has been destroyed."));
        }

        return true;
    }

    return false;
}

EUnitTypeId UDungeonSimSubsystem::GetStarterDefenderType(const ECreatureFamilyId CreatureFamilyId) const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonCreatureFamilyRow* FamilyRow = ContentSubsystem->FindFamilyRow(CreatureFamilyId))
        {
            if (FamilyRow->AllowedUnitTypes.Num() > 0)
            {
                return FamilyRow->AllowedUnitTypes[0];
            }

            UE_LOG(LogTemp, Fatal, TEXT("Family row must declare at least one allowed unit type."));
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for starter defender lookup."));
    return EUnitTypeId::None;
}

int32 UDungeonSimSubsystem::GetDefenderMaxHealth(const EUnitTypeId UnitTypeId) const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonUnitRow* UnitRow = ContentSubsystem->FindUnitRow(UnitTypeId))
        {
            return UnitRow->MaxHealth;
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for defender health lookup."));
    return 0;
}

int32 UDungeonSimSubsystem::GetDefenderDamage(const EUnitTypeId UnitTypeId) const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonUnitRow* UnitRow = ContentSubsystem->FindUnitRow(UnitTypeId))
        {
            return UnitRow->Damage;
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for defender damage lookup."));
    return 0;
}

ERoomTypeId UDungeonSimSubsystem::GetStarterSpawnerRoomType(const ECreatureFamilyId CreatureFamilyId) const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonCreatureFamilyRow* FamilyRow = ContentSubsystem->FindFamilyRow(CreatureFamilyId))
        {
            return FamilyRow->StarterSpawnerRoom;
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for starter spawner lookup."));
    return ERoomTypeId::None;
}

ETrapTypeId UDungeonSimSubsystem::GetStarterTrapType(const ECreatureFamilyId CreatureFamilyId) const
{
    if (const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem())
    {
        if (const FDungeonCreatureFamilyRow* FamilyRow = ContentSubsystem->FindFamilyRow(CreatureFamilyId))
        {
            return FamilyRow->StarterTrap;
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required for starter trap lookup."));
    return ETrapTypeId::None;
}

UDungeonContentSubsystem* UDungeonSimSubsystem::GetDungeonContentSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonContentSubsystem>() : nullptr;
}
