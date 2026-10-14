#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/DungeonTypes.h"

#include "DungeonHudWidget.generated.h"

class ADungeonPlayerController;
class UCanvasPanel;
class UDungeonContentSubsystem;
class UDungeonShopItemButton;
class UDungeonSimSubsystem;
class UTextBlock;
class UVerticalBox;
struct FDungeonBuildableRow;

/** One generated shop row: the button plus the label we rewrite when affordability or ownership changes. */
USTRUCT()
struct FDungeonShopButtonEntry
{
    GENERATED_BODY()

    UPROPERTY()
    FName BuildableId;

    UPROPERTY()
    TObjectPtr<UDungeonShopItemButton> Button;

    UPROPERTY()
    TObjectPtr<UTextBlock> Label;
};

UCLASS()
class DUNGEONCORE_API UDungeonHudWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Widget tree must exist before Slate builds it on AddToViewport, so the HUD is assembled here, not in NativeConstruct. */
    virtual void NativeOnInitialized() override;

    virtual void NativeConstruct() override;

    virtual void NativeDestruct() override;

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
    void HandleShopItemClicked(FName BuildableId);

    void BuildHud();

    /** Generates one heading + button per catalog group/item available to this run. */
    void BuildShopPanel(UVerticalBox* ShopBox);

    void RefreshAllText();

    void RefreshManaText(const FResourceState& ResourceState);

    void RefreshBreachText(const EBreachState BreachState);

    void RefreshSelectedText(const FName BuildableId);

    void RefreshWaveText(const int32 CurrentWaveNumber);

    void RefreshCoreHealthText(const int32 CurrentCoreHealth);

    void RefreshPlacementPreviewText(const FPlacementResult& PlacementResult);

    /** Rewrites labels/enabled state: hotkey, cost, owned/locked suffix, selected highlight. */
    void RefreshShopButtons();

    FText MakeShopButtonLabel(const FDungeonBuildableRow& Row, const int32 HotkeyIndex) const;

    FString DescribePlacementFailure(const EPlacementFailureReason FailureReason) const;

    ADungeonPlayerController* GetDungeonPlayerController() const;

    UDungeonSimSubsystem* GetDungeonSimSubsystem() const;

    UDungeonContentSubsystem* GetDungeonContentSubsystem() const;

    UTextBlock* CreateInfoTextBlock(const FName WidgetName);

    bool bHudBuilt = false;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UTextBlock> ManaTextBlock;

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
    TArray<FDungeonShopButtonEntry> ShopButtons;
};
