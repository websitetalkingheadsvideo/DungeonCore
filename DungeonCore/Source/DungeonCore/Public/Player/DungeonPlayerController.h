#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sim/DungeonTypes.h"

#include "DungeonPlayerController.generated.h"

class ADungeonBuildCursorActor;
class ADungeonChunkActor;
class UDungeonContentSubsystem;
class UDungeonSimSubsystem;
class UDungeonHudWidget;
class UDungeonRunSetupWidget;
struct FDungeonBuildableRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonSelectedBuildableChangedSignature, FName, BuildableId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonPlacementPreviewChangedSignature, FPlacementResult, PlacementResult);

UCLASS()
class DUNGEONCORE_API ADungeonPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ADungeonPlayerController();

    virtual void BeginPlay() override;

    virtual void PlayerTick(const float DeltaSeconds) override;

    virtual void SetupInputComponent() override;

    /** Shop click: upgrades are bought immediately, everything else becomes the active placement tool. */
    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void ActivateShopItem(const FName BuildableId);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void SelectBuildable(const FName BuildableId);

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FName GetSelectedBuildableId() const;

    /** Placeable catalog items available to this run, in catalog order; index N is hotkey N+1 (max 9). */
    UFUNCTION(BlueprintPure, Category = "Dungeon")
    TArray<FName> GetShopHotkeyBuildableIds() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult GetCurrentPlacementPreview() const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void StartRunFromSetup(const EDungeonThemeId ThemeId, const ECreatureFamilyId CreatureFamilyId);

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    bool IsRunActive() const;

    /** Console: dig the floor-1 block at (X, Y) through the normal sim rules. For testing without mouse input. */
    UFUNCTION(Exec)
    void DungeonDig(const int32 BlockX, const int32 BlockY);

    /** Console: log mana, income, breach state, entrances and unit counts; RepeatSeconds > 0 keeps logging on that interval. */
    UFUNCTION(Exec)
    void DungeonStatus(const float RepeatSeconds);

    /** Console cheat: add mana for testing layouts. */
    UFUNCTION(Exec)
    void DungeonGrantMana(const int32 Amount);

    /** Console: buy a catalog upgrade by id (e.g. DungeonBuy core_attunement_1). */
    UFUNCTION(Exec)
    void DungeonBuy(const FString& BuildableId);

    /** Console: select a catalog item by id, then place it at floor-1 block (X, Y) through the normal sim rules. */
    UFUNCTION(Exec)
    void DungeonPlace(const FString& BuildableId, const int32 BlockX, const int32 BlockY);

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonSelectedBuildableChangedSignature OnSelectedBuildableChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonPlacementPreviewChangedSignature OnPlacementPreviewChanged;

private:
    UFUNCTION()
    void HandleTileChanged(FTileData TileData);

    UFUNCTION()
    void HandleBreachStateChanged(EBreachState BreachState);

    UFUNCTION()
    void HandleAdventurersChanged();

    void HandlePrimaryAction();

    void HandleLeftMousePressed();

    void HandleLeftMouseReleased();

    void ApplyClickDragCameraInput();

    void SelectShopHotkey(const int32 HotkeyIndex);

    /** Runs the selected item's action (dig / room / trap) at the given block. Upgrades are never placed. */
    FPlacementResult PlaceSelectedBuildableAt(const FBlockCoord& BlockCoord);

    const FDungeonBuildableRow* GetSelectedBuildableRow() const;

    void UpdateCursorFromMouse();

    bool ProjectMouseToDungeonSurface(FVector& OutWorldPoint) const;

    void UpdatePlacementPreview();

    void ZoomIn();

    void ZoomOut();

    void RecenterCameraOnCore();

    void RestartRun();

    void QuickSaveRun();

    void QuickLoadRun();

    void RefreshChunkVisuals();

    UDungeonSimSubsystem* GetDungeonSimSubsystem() const;

    UDungeonContentSubsystem* GetDungeonContentSubsystem() const;

    ADungeonChunkActor* FindChunkActor() const;

    void EnsureChunkActorExists();

    void EnsureBuildCursor();

    class ADungeonCameraPawn* GetDungeonCameraPawn() const;

    UPROPERTY()
    TObjectPtr<ADungeonBuildCursorActor> BuildCursorActor;

    UPROPERTY()
    TObjectPtr<UDungeonHudWidget> HudWidget;

    UPROPERTY()
    TObjectPtr<UDungeonRunSetupWidget> SetupWidget;

    /** Catalog id of the active placement tool; always a Dig/Room/Trap item, never an Upgrade. */
    UPROPERTY()
    FName SelectedBuildableId;

    UPROPERTY()
    FPlacementResult CurrentPlacementPreview;

    FBlockCoord HoveredBlockCoord;

    bool bHasHoveredBlock = false;

    bool bRunActive = false;

    bool bHasPreviousMousePosition = false;

    float PreviousMouseX = 0.0f;

    float PreviousMouseY = 0.0f;

    float LeftMouseDragDistancePixels = 0.0f;

    FTimerHandle StatusLogTimerHandle;

    static constexpr float ClickDragThresholdPixels = 8.0f;
};
