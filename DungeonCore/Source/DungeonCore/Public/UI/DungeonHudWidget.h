#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/DungeonTypes.h"

#include "DungeonHudWidget.generated.h"

class ADungeonPlayerController;
class UBorder;
class UButton;
class UCanvasPanel;
class UDungeonContentSubsystem;
class UDungeonSimSubsystem;
class UDungeonStoreWidget;
class UTextBlock;

UCLASS()
class DUNGEONCORE_API UDungeonHudWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Widget tree must exist before Slate builds it on AddToViewport, so the HUD is assembled here, not in NativeConstruct. */
    virtual void NativeOnInitialized() override;

    virtual void NativeConstruct() override;

    virtual void NativeDestruct() override;

    void SetStoreVisible(const bool bVisible);

    /** Click tried to spend mana the core does not have. Mana itself is left unchanged. */
    void ShowOutOfManaNotice(const int32 ManaCost);

    void HideOutOfManaNotice();

private:
    UFUNCTION()
    void HandleResourceChanged(FResourceState ResourceState);

    UFUNCTION()
    void HandleBreachStateChanged(EBreachState BreachState);

    UFUNCTION()
    void HandleSelectedBuildableChanged(FName BuildableId);

    UFUNCTION()
    void HandleWaveChanged(int32 CurrentWaveNumber);

    UFUNCTION()
    void HandleCoreHealthChanged(int32 CurrentCoreHealth);

    UFUNCTION()
    void HandlePlacementPreviewChanged(FPlacementResult PlacementResult);

    UFUNCTION()
    void HandleVaultClicked();

    UFUNCTION()
    void HandleOutOfManaNoticeExpired();

    void BuildHud();

    void RefreshAllText();

    void RefreshManaText(const FResourceState& ResourceState);

    void RefreshBreachText(const EBreachState BreachState);

    void RefreshSelectedText(const FName BuildableId);

    void RefreshWaveText(const int32 CurrentWaveNumber);

    void RefreshCoreHealthText(const int32 CurrentCoreHealth);

    void RefreshPlacementPreviewText(const FPlacementResult& PlacementResult);

    void RefreshStore();

    FString DescribePlacementFailure(const EPlacementFailureReason FailureReason) const;

    ADungeonPlayerController* GetDungeonPlayerController() const;

    UDungeonSimSubsystem* GetDungeonSimSubsystem() const;

    UDungeonContentSubsystem* GetDungeonContentSubsystem() const;

    UTextBlock* CreateInfoTextBlock(const FName WidgetName);

    bool bHudBuilt = false;

    bool bOutOfManaNoticeVisible = false;

    FTimerHandle OutOfManaNoticeTimer;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UTextBlock> ManaTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> IncomeTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> BreachTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> SelectedTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> ControlsTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> WaveTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> CoreHealthTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> PlacementPreviewTextBlock;

    UPROPERTY()
    TObjectPtr<UBorder> OutOfManaNotice;

    UPROPERTY()
    TObjectPtr<UTextBlock> OutOfManaText;

    UPROPERTY()
    TObjectPtr<UButton> VaultButton;

    UPROPERTY()
    TObjectPtr<UDungeonStoreWidget> StoreWidget;
};
