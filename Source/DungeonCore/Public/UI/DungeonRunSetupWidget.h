#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sim/DungeonTypes.h"

#include "DungeonRunSetupWidget.generated.h"

class ADungeonPlayerController;
class UButton;
class UCanvasPanel;
class UTextBlock;

UCLASS()
class DUNGEONCORE_API UDungeonRunSetupWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    virtual void NativeDestruct() override;

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

    void RefreshSelectionText();

    FText GetThemeDisplayText(const EDungeonThemeId ThemeId) const;

    FText GetFamilyDisplayText(const ECreatureFamilyId FamilyId) const;

    ADungeonPlayerController* GetDungeonPlayerController() const;

    UButton* CreateActionButton(const FText& LabelText, const FName WidgetName);

    UTextBlock* CreateTextBlock(const FName WidgetName);

    bool bWidgetBuilt = false;

    UPROPERTY()
    EDungeonThemeId SelectedThemeId = EDungeonThemeId::Gothic;

    UPROPERTY()
    ECreatureFamilyId SelectedFamilyId = ECreatureFamilyId::Undead;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UTextBlock> SelectedThemeTextBlock;

    UPROPERTY()
    TObjectPtr<UTextBlock> SelectedFamilyTextBlock;

    UPROPERTY()
    TObjectPtr<UButton> StartRunButton;
};
