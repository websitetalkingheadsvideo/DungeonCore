#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sim/DungeonTypes.h"

#include "DungeonSimSubsystem.generated.h"

class UDungeonGridState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonResourceChangedSignature, FResourceState, ResourceState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonTileChangedSignature, FTileData, TileData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonBreachStateChangedSignature, EBreachState, BreachState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDungeonAdventurersChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonWaveChangedSignature, int32, CurrentWaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonCoreHealthChangedSignature, int32, CurrentCoreHealth);

UCLASS()
class DUNGEONCORE_API UDungeonSimSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void StartNewRun(const FRunSetup& InRunSetup);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    bool AddMana(const int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    bool SpendMana(const int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    FPlacementResult DigBlock(const FBlockCoord& BlockCoord);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    FPlacementResult PlaceRoom(
        const FBlockCoord& OriginBlock,
        const ERoomTypeId RoomTypeId,
        const FRoomFootprint& Footprint,
        const int32 ManaCost
    );

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    FPlacementResult PlaceTrap(const FBlockCoord& OriginBlock, const ETrapTypeId TrapTypeId, const int32 ManaCost);

    /** Buys a non-placed catalog upgrade (e.g. Core Attunement). Spends mana and recomputes income on success. */
    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    FPlacementResult PurchaseUpgrade(const FName BuildableId);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    bool SaveCurrentRun(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    bool LoadSavedRun(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void RestartCurrentRun();

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult PreviewDigAtBlock(const FBlockCoord& BlockCoord) const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult PreviewRoomPlacement(
        const FBlockCoord& OriginBlock,
        const FRoomFootprint& Footprint,
        const int32 ManaCost
    ) const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult PreviewTrapPlacement(const FBlockCoord& OriginBlock, const int32 ManaCost) const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult PreviewUpgradePurchase(const FName BuildableId) const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    bool IsUpgradeOwned(const FName BuildableId) const;

    /** False for catalog items locked to a creature family other than this run's. */
    UFUNCTION(BlueprintPure, Category = "Dungeon")
    bool IsBuildableAvailableForRun(const FName BuildableId) const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FResourceState GetResourceState() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FFloorState GetFloorState() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FRunSetup GetRunSetup() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    UDungeonGridState* GetGridState() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    int32 GetCurrentWaveNumber() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    int32 GetCurrentCoreHealth() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    TArray<FUnitInstanceData> GetActiveAdventurers() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    TArray<FUnitInstanceData> GetActiveDefenders() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    int32 GetDigManaCost() const;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonResourceChangedSignature OnResourceChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonTileChangedSignature OnTileChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonBreachStateChangedSignature OnBreachStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonAdventurersChangedSignature OnAdventurersChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonWaveChangedSignature OnWaveChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonCoreHealthChangedSignature OnCoreHealthChanged;

private:
    void EnsureGridState();

    void SeedFirstFloor();

    FTileData MakeTileData(const FBlockCoord& BlockCoord, const ETileState TileState, const bool bWalkable, const bool bDiggable) const;

    FChunkCoord MakeChunkCoordForBlock(const FBlockCoord& BlockCoord) const;

    void BroadcastTileAt(const FBlockCoord& BlockCoord);

    void BroadcastAllTiles();

    void RestoreSimulationTimers();

    /** Starts the invasion timers (waves, adventurer movement, defender spawns). Call once the breach is Open. */
    void StartInvasionTimers();

    void StartManaTickTimer();

    void TickManaIncome();

    /** Base trickle + every placed room's catalog income + every owned upgrade's catalog income. */
    float ComputeManaIncomePerSecond() const;

    /** Validation shared by preview and purchase; does not spend anything. */
    FPlacementResult ValidateUpgradePurchase(const FName BuildableId) const;

    FBlockCoord FindNearestSurfaceEntry(const FBlockCoord& FromBlock) const;

    void ScheduleNextWave(const float DelaySeconds);

    void StartNextWave();

    void AdvanceAdventurers();

    void SpawnDefenders();

    /** Adds the adventurer's block and every walkable tile visible down the four straight corridors to KnownBlocks. */
    void RevealSurroundings(FUnitInstanceData& AdventurerData) const;

    /**
     * Fog-of-war pathing. Traverses only tiles the adventurer knows. Goal is a known Core Chamber tile if one
     * has been seen, otherwise the nearest unknown walkable tile. Empty when nothing is reachable.
     */
    TArray<FBlockCoord> BuildAdventurerPath(const FUnitInstanceData& AdventurerData) const;

    TArray<FBlockCoord> BuildPathToBlock(const FBlockCoord& StartBlock, const FBlockCoord& GoalBlock) const;

    bool HandleAdventurerTileEffects(FUnitInstanceData& AdventurerData);

    EUnitTypeId GetStarterDefenderType(const ECreatureFamilyId CreatureFamilyId) const;

    int32 GetDefenderMaxHealth(const EUnitTypeId UnitTypeId) const;

    int32 GetDefenderDamage(const EUnitTypeId UnitTypeId) const;

    class UDungeonContentSubsystem* GetDungeonContentSubsystem() const;

    ERoomTypeId GetStarterSpawnerRoomType(const ECreatureFamilyId CreatureFamilyId) const;

    ETrapTypeId GetStarterTrapType(const ECreatureFamilyId CreatureFamilyId) const;

    UPROPERTY()
    TObjectPtr<UDungeonGridState> GridState;

    UPROPERTY()
    TMap<FGuid, FRoomInstanceData> RoomInstances;

    UPROPERTY()
    TMap<FGuid, FTrapInstanceData> TrapInstances;

    UPROPERTY()
    FResourceState ResourceState;

    UPROPERTY()
    FRunSetup RunSetup;

    UPROPERTY()
    FFloorState FloorState;

    UPROPERTY()
    TArray<FUnitInstanceData> ActiveAdventurers;

    UPROPERTY()
    TArray<FUnitInstanceData> ActiveDefenders;

    /** Catalog ids of non-placed upgrades bought this run. */
    UPROPERTY()
    TArray<FName> PurchasedUpgradeIds;

    int32 CurrentWaveNumber = 0;

    int32 CurrentCoreHealth = 0;

    FTimerHandle WaveStartTimerHandle;

    FTimerHandle AdventurerAdvanceTimerHandle;

    FTimerHandle DefenderSpawnTimerHandle;

    FTimerHandle ManaTickTimerHandle;

    /** Sub-integer mana carried between ticks so fractional income is not lost. */
    float ManaFractionCarry = 0.0f;

    /** Seeded from FRunSetup::Seed; drives adventurer tie-breaks so runs are reproducible. */
    FRandomStream AdventurerDecisionStream;
};
