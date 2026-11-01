#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/DungeonTypes.h"

#include "DungeonRunSetupWidget.generated.h"

class ADungeonPlayerController;
class UButton;
class UCanvasPanel;
class UTextBlock;
class UWidget;

/** One selectable tile on the setup screen: the button plus the label whose colour flips when selected. */
USTRUCT()
struct FDungeonSetupOptionWidgets
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UButton> Button;

    UPROPERTY()
    TObjectPtr<UTextBlock> Label;
};

UCLASS()
class DUNGEONCORE_API UDungeonRunSetupWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    virtual void NativeConstruct() override;

    virtual void NativeDestruct() override;

    /** Same effect as clicking the theme tile; used by the DungeonPreviewTheme console command. */
    void SelectTheme(const EDungeonThemeId ThemeId);

private:
    UFUNCTION()
    void HandleThemeGothicClicked();

    UFUNCTION()
    void HandleThemeGoblinClicked();

    UFUNCTION()
    void HandleThemeFungalClicked();

    UFUNCTION()
    void HandleThemeSunkenClicked();

    UFUNCTION()
    void HandleFamilyUndeadClicked();

    UFUNCTION()
    void HandleFamilyOrcsClicked();

    UFUNCTION()
    void HandleFamilySpidersClicked();

    UFUNCTION()
    void HandleFamilyFairiesClicked();

    UFUNCTION()
    void HandleStartRunClicked();

    void BuildWidget();

    /** Repaints the selected tiles and the summary line from SelectedThemeId / SelectedFamilyId. */
    void RefreshSelection();

    FText GetThemeDisplayText(const EDungeonThemeId ThemeId) const;

    FText GetFamilyDisplayText(const ECreatureFamilyId FamilyId) const;

    FText GetFamilyArtSourceText(const ECreatureFamilyId FamilyId) const;

    ADungeonPlayerController* GetDungeonPlayerController() const;

    FDungeonSetupOptionWidgets CreateOptionTile(const FText& LabelText, const FName WidgetName);

    void ApplyOptionTileStyle(const FDungeonSetupOptionWidgets& Option, const bool bSelected);

    UTextBlock* CreateTextBlock(const FName WidgetName, const FName Typeface, const int32 Size, const int32 LetterSpacing, const FLinearColor& Color);

    UTextBlock* CreateSectionHeader(const FText& HeaderText, const FName WidgetName);

    UWidget* CreateDivider(const FName WidgetName);

    bool bWidgetBuilt = false;

    UPROPERTY()
    EDungeonThemeId SelectedThemeId = EDungeonThemeId::Gothic;

    UPROPERTY()
    ECreatureFamilyId SelectedFamilyId = ECreatureFamilyId::Orcs;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TMap<EDungeonThemeId, FDungeonSetupOptionWidgets> ThemeOptions;

    UPROPERTY()
    TMap<ECreatureFamilyId, FDungeonSetupOptionWidgets> FamilyOptions;

    UPROPERTY()
    TObjectPtr<UTextBlock> SelectionSummaryTextBlock;

    /** Shown under the summary while the selected family's creature art is still only on Fab. */
    UPROPERTY()
    TObjectPtr<UTextBlock> PendingArtTextBlock;

    UPROPERTY()
    TObjectPtr<UButton> StartRunButton;
};
