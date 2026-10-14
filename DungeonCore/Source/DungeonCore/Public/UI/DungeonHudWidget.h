#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/DungeonTypes.h"

#include "DungeonHudWidget.generated.h"

class ADungeonPlayerController;
class UButton;
class UCanvasPanel;
class UTextBlock;

UCLASS()
class DUNGEONCORE_API UDungeonHudWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleResourceChanged(FResourceState ResourceState);

    UFUNCTION()
    void HandleBreachStateChanged(EBreachState BreachState);

    UFUNCTION()
    void HandleBuildModeChanged(EDungeonBuildMode BuildMode);

    UFUNCTION()
    void HandleWaveChanged(int32 CurrentWaveNumber);

    UFUNCTION()
    void HandleCoreHealthChanged(int32 CurrentCoreHealth);

    UFUNCTION()
    void HandlePlacementPreviewChanged(FPlacementResult PlacementResult);

    UFUNCTION()
    void HandleDigClicked();

    UFUNCTION()
    void HandleManaWellClicked();

    UFUNCTION()
    void HandleStarterSpawnerClicked();

    UFUNCTION()
    void HandleStarterTrapClicked();

    void BuildHud();

    void RefreshAllText();

    void RefreshManaText(const FResourceState& ResourceState);

    void RefreshBreachText(const EBreachState BreachState);

    void RefreshModeText(const EDungeonBuildMode BuildMode);

    void RefreshWaveText(const int32 CurrentWaveNumber);

    void RefreshCoreHealthText(const int32 CurrentCoreHealth);

    void RefreshPlacementPreviewText(const FPlacementResult& PlacementResult);

    ADungeonPlayerController* GetDungeonPlayerController() const;

    UButton* CreateActionButton(const FText& LabelText, const FName WidgetName);

    UTextBlock* CreateInfoTextBlock(const FName WidgetName);

    bool bHudBuilt = false;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UTextBlock> ManaTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> BreachTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> BuildModeTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> ControlsTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> WaveTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> CoreHealthTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> PlacementPreviewTextBlock;

    UPROPERTY()
    TObjectPtr<UButton> DigButton;

    UPROPERTY()
    TObjectPtr<UButton> ManaWellButton;

    UPROPERTY()
    TObjectPtr<UButton> StarterSpawnerButton;

    UPROPERTY()
    TObjectPtr<UButton> StarterTrapButton;
};
