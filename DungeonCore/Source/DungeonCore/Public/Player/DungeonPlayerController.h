#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sim/DungeonTypes.h"

#include "DungeonPlayerController.generated.h"

class ADungeonBuildCursorActor;
class ADungeonChunkActor;
class UDungeonSimSubsystem;
class UDungeonHudWidget;
class UDungeonRunSetupWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDungeonBuildModeChangedSignature, EDungeonBuildMode, BuildMode);
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

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void SetBuildModeFromUI(const EDungeonBuildMode InBuildMode);

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    EDungeonBuildMode GetBuildMode() const;

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    FPlacementResult GetCurrentPlacementPreview() const;

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void StartRunFromSetup(const EDungeonThemeId ThemeId, const ECreatureFamilyId CreatureFamilyId);

    UFUNCTION(BlueprintPure, Category = "Dungeon")
    bool IsRunActive() const;

    UPROPERTY(BlueprintAssignable, Category = "Dungeon")
    FDungeonBuildModeChangedSignature OnBuildModeChanged;

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

    void SetDigMode();

    void SetManaWellMode();

    void SetStarterSpawnerMode();

    void SetStarterTrapMode();

    void SetBuildMode(const EDungeonBuildMode InBuildMode);

    void UpdateCursorFromMouse();

    void UpdatePlacementPreview();

    void ZoomIn();

    void ZoomOut();

    void RecenterCameraOnCore();

    void RestartRun();

    void QuickSaveRun();

    void QuickLoadRun();

    void RefreshChunkVisuals();

    UDungeonSimSubsystem* GetDungeonSimSubsystem() const;

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

    UPROPERTY()
    EDungeonBuildMode BuildMode = EDungeonBuildMode::Dig;

    UPROPERTY()
    FPlacementResult CurrentPlacementPreview;

    FBlockCoord HoveredBlockCoord;

    bool bHasHoveredBlock = false;

    bool bRunActive = false;
};
