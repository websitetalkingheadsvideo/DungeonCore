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
        World->GetTimerManager().ClearTimer(ManaTickTimerHandle);
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
    ResourceState.BaseManaPerSecond = DungeonCore::SealedManaPerSecond;
    ManaFractionCarry = 0.0f;
    AdventurerDecisionStream.Initialize(RunSetup.Seed);

    FloorState.FloorIndex = RunSetup.StartingFloorIndex;
    FloorState.BreachState = EBreachState::Sealed;
    FloorState.CoreBlock = DungeonCore::GetStartingCoreBlockCoord();
    // No predetermined entrance: the chamber starts sealed and the dungeon opens only when the player's
    // tunnels reach the edge of the mountain, on whichever side(s) they choose.
    FloorState.SurfaceEntryBlocks.Reset();

    RoomInstances.Reset();
    TrapInstances.Reset();
    ActiveAdventurers.Reset();
    ActiveDefenders.Reset();
    PurchasedUpgradeIds.Reset();
    CurrentWaveNumber = 0;
    CurrentCoreHealth = 100;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WaveStartTimerHandle);
        World->GetTimerManager().ClearTimer(AdventurerAdvanceTimerHandle);
        World->GetTimerManager().ClearTimer(DefenderSpawnTimerHandle);
    }

    SeedFirstFloor();
    StartManaTickTimer();
    ResourceState.ManaIncomePerSecond = ComputeManaIncomePerSecond();

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

    // Tunnels that reach the edge of the mountain become entrances. The first one opens the dungeon to
    // adventurers; later ones simply add more ways in.
    const bool bReachesSurface = DungeonCore::IsSurfaceEdgeBlock(BlockCoord);
    const bool bApplySucceeded = GridState->ApplyDig(BlockCoord, bReachesSurface);
    if (!bApplySucceeded)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyDig failed for block (%d, %d, %d)."), BlockCoord.X, BlockCoord.Y, BlockCoord.Z);
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::Blocked;
        return Result;
    }

    if (bReachesSurface)
    {
        FloorState.SurfaceEntryBlocks.AddUnique(BlockCoord);

        if (FloorState.BreachState == EBreachState::Sealed)
        {
            FloorState.BreachState = EBreachState::Open;
            ResourceState.BaseManaPerSecond = DungeonCore::OpenManaPerSecond;
            OnBreachStateChanged.Broadcast(FloorState.BreachState);
            StartInvasionTimers();
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

    // Income rooms change the rate immediately; the HUD should not wait for the next tick to show it.
    ResourceState.ManaIncomePerSecond = ComputeManaIncomePerSecond();
    OnResourceChanged.Broadcast(ResourceState);

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

FPlacementResult UDungeonSimSubsystem::PurchaseUpgrade(const FName BuildableId)
{
    FPlacementResult Result = ValidateUpgradePurchase(BuildableId);
    if (!Result.bSuccess)
    {
        return Result;
    }

    const bool bSpendSucceeded = SpendMana(Result.ManaCost);
    if (!bSpendSucceeded)
    {
        Result.bSuccess = false;
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    PurchasedUpgradeIds.Add(BuildableId);
    ResourceState.ManaIncomePerSecond = ComputeManaIncomePerSecond();
    OnResourceChanged.Broadcast(ResourceState);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Purchased upgrade '%s' for %d mana; income now %.2f/s."),
        *BuildableId.ToString(),
        Result.ManaCost,
        ResourceState.ManaIncomePerSecond
    );
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
    SaveGame->PurchasedUpgradeIds = PurchasedUpgradeIds;

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
    AdventurerDecisionStream.Initialize(RunSetup.Seed);
    CurrentWaveNumber = SaveGame->CurrentWaveNumber;
    CurrentCoreHealth = SaveGame->CurrentCoreHealth;
    ActiveAdventurers = SaveGame->ActiveAdventurers;
    ActiveDefenders = SaveGame->ActiveDefenders;
    PurchasedUpgradeIds = SaveGame->PurchasedUpgradeIds;

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
    StartManaTickTimer();
    ResourceState.ManaIncomePerSecond = ComputeManaIncomePerSecond();
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

FPlacementResult UDungeonSimSubsystem::PreviewUpgradePurchase(const FName BuildableId) const
{
    return ValidateUpgradePurchase(BuildableId);
}

bool UDungeonSimSubsystem::IsUpgradeOwned(const FName BuildableId) const
{
    return PurchasedUpgradeIds.Contains(BuildableId);
}

bool UDungeonSimSubsystem::IsBuildableAvailableForRun(const FName BuildableId) const
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (ContentSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required to check buildable availability."));
        return false;
    }

    const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
    if (Row == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("IsBuildableAvailableForRun: unknown buildable '%s'."), *BuildableId.ToString());
        return false;
    }

    return Row->AllowedFamilies.Num() == 0 || Row->AllowedFamilies.Contains(RunSetup.CreatureFamilyId);
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

    for (const FBlockCoord& BlockCoord : CoreChamberBlocks)
    {
        BroadcastTileAt(BlockCoord);
    }
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

    StartInvasionTimers();
}

void UDungeonSimSubsystem::StartInvasionTimers()
{
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

void UDungeonSimSubsystem::StartManaTickTimer()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot start the mana tick without a world."));
        return;
    }

    World->GetTimerManager().SetTimer(
        ManaTickTimerHandle,
        this,
        &UDungeonSimSubsystem::TickManaIncome,
        DungeonCore::ManaTickIntervalSeconds,
        true
    );
}

void UDungeonSimSubsystem::TickManaIncome()
{
    const float IncomePerSecond = ComputeManaIncomePerSecond();
    ResourceState.ManaIncomePerSecond = IncomePerSecond;

    ManaFractionCarry += IncomePerSecond * DungeonCore::ManaTickIntervalSeconds;
    const int32 WholeMana = FMath::FloorToInt32(ManaFractionCarry);
    ManaFractionCarry -= static_cast<float>(WholeMana);

    if (WholeMana > 0)
    {
        ResourceState.CurrentMana += WholeMana;
    }
    OnResourceChanged.Broadcast(ResourceState);
}

float UDungeonSimSubsystem::ComputeManaIncomePerSecond() const
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (ContentSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required to compute mana income."));
        return 0.0f;
    }

    const int32 AdventurersCounted = FMath::Min(ActiveAdventurers.Num(), DungeonCore::MaxAdventurersCountedForMana);
    const float AdventurerFactor = static_cast<float>(AdventurersCounted);

    float Income = ResourceState.BaseManaPerSecond + AdventurerFactor * DungeonCore::ManaPerAdventurerInsidePerSecond;

    for (const TPair<FGuid, FRoomInstanceData>& RoomEntry : RoomInstances)
    {
        const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildableByRoomType(RoomEntry.Value.RoomTypeId);
        if (Row == nullptr)
        {
            // Core Chamber and corridors are not shop items and contribute nothing.
            continue;
        }
        Income += Row->ManaPerSecond + Row->ManaPerAdventurerPerSecond * AdventurerFactor;
    }

    for (const FName& UpgradeId : PurchasedUpgradeIds)
    {
        const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(UpgradeId);
        if (Row == nullptr)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Owned upgrade '%s' is missing from the shop catalog."), *UpgradeId.ToString());
            return Income;
        }
        Income += Row->ManaPerSecond + Row->ManaPerAdventurerPerSecond * AdventurerFactor;
    }

    return Income;
}

FPlacementResult UDungeonSimSubsystem::ValidateUpgradePurchase(const FName BuildableId) const
{
    FPlacementResult Result;

    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (ContentSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required to validate upgrade purchases."));
        return Result;
    }

    const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
    if (Row == nullptr || Row->Category != EBuildableCategory::Upgrade)
    {
        Result.FailureReason = EPlacementFailureReason::UnknownBuildable;
        return Result;
    }

    Result.ManaCost = Row->ManaCost;

    if (!IsBuildableAvailableForRun(BuildableId))
    {
        Result.FailureReason = EPlacementFailureReason::FamilyLocked;
        return Result;
    }

    if (PurchasedUpgradeIds.Contains(BuildableId))
    {
        Result.FailureReason = EPlacementFailureReason::AlreadyOwned;
        return Result;
    }

    if (!Row->RequiresBuildableId.IsNone() && !PurchasedUpgradeIds.Contains(Row->RequiresBuildableId))
    {
        Result.FailureReason = EPlacementFailureReason::MissingPrerequisite;
        return Result;
    }

    if (ResourceState.CurrentMana < Row->ManaCost)
    {
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    Result.bSuccess = true;
    return Result;
}

FBlockCoord UDungeonSimSubsystem::FindNearestSurfaceEntry(const FBlockCoord& FromBlock) const
{
    if (FloorState.SurfaceEntryBlocks.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FindNearestSurfaceEntry called while the dungeon has no entrances."));
        return FloorState.CoreBlock;
    }

    FBlockCoord Nearest = FloorState.SurfaceEntryBlocks[0];
    int32 NearestDistance = TNumericLimits<int32>::Max();
    for (const FBlockCoord& Entry : FloorState.SurfaceEntryBlocks)
    {
        const int32 Distance = FMath::Abs(Entry.X - FromBlock.X) + FMath::Abs(Entry.Y - FromBlock.Y);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            Nearest = Entry;
        }
    }
    return Nearest;
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

    if (FloorState.SurfaceEntryBlocks.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("StartNextWave: breach is Open but there are no surface entries."));
        return;
    }

    CurrentWaveNumber += 1;
    OnWaveChanged.Broadcast(CurrentWaveNumber);

    // Spread the wave across every entrance the player has opened.
    const int32 WaveSize = 3 + FMath::Max(0, CurrentWaveNumber - 1);
    for (int32 UnitIndex = 0; UnitIndex < WaveSize; ++UnitIndex)
    {
        FUnitInstanceData AdventurerData;
        AdventurerData.UnitInstanceId = FGuid::NewGuid();
        AdventurerData.UnitTypeId = UnitIndex % 3 == 2 ? EUnitTypeId::AdventurerRanged : EUnitTypeId::AdventurerMelee;
        AdventurerData.CurrentBlock = FloorState.SurfaceEntryBlocks[UnitIndex % FloorState.SurfaceEntryBlocks.Num()];
        AdventurerData.CurrentHealth = 10;
        AdventurerData.bIsAdventurer = true;
        AdventurerData.Damage = AdventurerData.UnitTypeId == EUnitTypeId::AdventurerRanged ? 8 : 10;
        AdventurerData.PathStepIndex = 0;
        RevealSurroundings(AdventurerData);
        AdventurerData.CurrentPath = BuildAdventurerPath(AdventurerData);

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

        RevealSurroundings(AdventurerData);
        AdventurerData.CurrentPath = BuildAdventurerPath(AdventurerData);

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
        RevealSurroundings(AdventurerData);
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

void UDungeonSimSubsystem::RevealSurroundings(FUnitInstanceData& AdventurerData) const
{
    if (GridState == nullptr)
    {
        return;
    }

    AdventurerData.KnownBlocks.AddUnique(AdventurerData.CurrentBlock);

    const TArray<FBlockCoord> Directions = {
        FBlockCoord(1, 0, 0),
        FBlockCoord(-1, 0, 0),
        FBlockCoord(0, 1, 0),
        FBlockCoord(0, -1, 0)
    };

    for (const FBlockCoord& Direction : Directions)
    {
        FBlockCoord Probe = AdventurerData.CurrentBlock;
        for (int32 Step = 0; Step < DungeonCore::AdventurerSightBlocks; ++Step)
        {
            Probe = Probe + Direction;
            const FTileData* ProbeTile = GridState->FindTileData(Probe);
            if (ProbeTile == nullptr || !ProbeTile->bWalkable)
            {
                break;
            }
            AdventurerData.KnownBlocks.AddUnique(Probe);
        }
    }
}

TArray<FBlockCoord> UDungeonSimSubsystem::BuildAdventurerPath(const FUnitInstanceData& AdventurerData) const
{
    TArray<FBlockCoord> EmptyPath;
    if (GridState == nullptr)
    {
        return EmptyPath;
    }

    const FBlockCoord StartBlock = AdventurerData.CurrentBlock;
    const FTileData* StartTile = GridState->FindTileData(StartBlock);
    if (StartTile == nullptr || !StartTile->bWalkable)
    {
        return EmptyPath;
    }

    TSet<FBlockCoord> Known;
    Known.Append(AdventurerData.KnownBlocks);
    Known.Add(StartBlock);

    bool bCoreKnown = false;
    for (const FBlockCoord& KnownBlock : Known)
    {
        const FTileData* KnownTile = GridState->FindTileData(KnownBlock);
        if (KnownTile != nullptr && KnownTile->RoomTypeId == ERoomTypeId::CoreChamber)
        {
            bCoreKnown = true;
            break;
        }
    }

    // Breadth-first over known tiles only. Unknown walkable tiles are accepted as goals (the step into
    // the dark) but never expanded through, so the adventurer cannot "see" beyond what it has explored.
    // The known graph is small, so the search runs to completion and the goal is chosen afterwards.
    TArray<FBlockCoord> Frontier;
    Frontier.Add(StartBlock);

    TMap<FBlockCoord, FBlockCoord> CameFrom;
    CameFrom.Add(StartBlock, StartBlock);
    TMap<FBlockCoord, int32> Distance;
    Distance.Add(StartBlock, 0);

    TArray<FBlockCoord> CoreGoals;
    TArray<FBlockCoord> UnknownGoals;

    const TArray<FBlockCoord> NeighborOffsets = {
        FBlockCoord(1, 0, 0),
        FBlockCoord(-1, 0, 0),
        FBlockCoord(0, 1, 0),
        FBlockCoord(0, -1, 0)
    };

    int32 FrontierIndex = 0;
    while (FrontierIndex < Frontier.Num())
    {
        const FBlockCoord CurrentBlock = Frontier[FrontierIndex];
        FrontierIndex += 1;

        const FTileData* CurrentTile = GridState->FindTileData(CurrentBlock);
        if (CurrentTile != nullptr && CurrentTile->RoomTypeId == ERoomTypeId::CoreChamber)
        {
            CoreGoals.Add(CurrentBlock);
        }

        if (!Known.Contains(CurrentBlock))
        {
            UnknownGoals.Add(CurrentBlock);
            continue;
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

            CameFrom.Add(NeighborBlock, CurrentBlock);
            Distance.Add(NeighborBlock, Distance[CurrentBlock] + 1);
            Frontier.Add(NeighborBlock);
        }
    }

    const TArray<FBlockCoord>& Goals = bCoreKnown ? CoreGoals : UnknownGoals;
    if (Goals.Num() == 0)
    {
        return EmptyPath;
    }

    // Among the nearest goals pick one at random so a party splits up at junctions instead of marching
    // in lockstep, and so no compass direction is systematically favoured.
    int32 NearestDistance = TNumericLimits<int32>::Max();
    for (const FBlockCoord& Goal : Goals)
    {
        NearestDistance = FMath::Min(NearestDistance, Distance[Goal]);
    }
    TArray<FBlockCoord> NearestGoals;
    for (const FBlockCoord& Goal : Goals)
    {
        if (Distance[Goal] == NearestDistance)
        {
            NearestGoals.Add(Goal);
        }
    }
    const FBlockCoord GoalBlock = NearestGoals[AdventurerDecisionStream.RandRange(0, NearestGoals.Num() - 1)];

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
            AdventurerData.CurrentBlock = FindNearestSurfaceEntry(AdventurerData.CurrentBlock);
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
