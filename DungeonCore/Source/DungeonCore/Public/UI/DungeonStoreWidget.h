#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "DungeonStoreWidget.generated.h"

class UBorder;
class UButton;
class UDungeonContentSubsystem;
class UDungeonShopItemButton;
class UDungeonSimSubsystem;
class UScrollBox;
class UTextBlock;
struct FDungeonBuildableRow;

USTRUCT()
struct FDungeonStoreTabWidgets
{
    GENERATED_BODY()

    UPROPERTY()
    FName GroupId;

    UPROPERTY()
    FText GroupDisplayName;

    UPROPERTY()
    TObjectPtr<UDungeonShopItemButton> Button;

    UPROPERTY()
    TObjectPtr<UTextBlock> Label;

    UPROPERTY()
    TObjectPtr<UBorder> Accent;
};

USTRUCT()
struct FDungeonStoreCardWidgets
{
    GENERATED_BODY()

    UPROPERTY()
    FName BuildableId;

    UPROPERTY()
    FName GroupId;

    UPROPERTY()
    TObjectPtr<UWidget> Root;

    UPROPERTY()
    TObjectPtr<UDungeonShopItemButton> Button;

    UPROPERTY()
    TObjectPtr<UBorder> MonogramPlate;

    UPROPERTY()
    TObjectPtr<UTextBlock> MonogramText;

    UPROPERTY()
    TObjectPtr<UTextBlock> NameText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailText;

    UPROPERTY()
    TObjectPtr<UTextBlock> CostText;
};

/** Full-screen Core Vault. Built in C++ from the shop catalog; B or the scrim closes it. */
UCLASS()
class DUNGEONCORE_API UDungeonStoreWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    virtual void NativeDestruct() override;

    void RefreshStore();

private:
    UFUNCTION()
    void HandleScrimClicked();

    UFUNCTION()
    void HandleCloseClicked();

    UFUNCTION()
    void HandleActionClicked();

    UFUNCTION()
    void HandleTabClicked(FName GroupId);

    UFUNCTION()
    void HandleCardClicked(FName BuildableId);

    void BuildStore();

    void ShowGroup(const FName GroupId);

    FName FirstCardInGroup(const FName GroupId) const;

    UDungeonSimSubsystem* GetDungeonSimSubsystem() const;

    UDungeonContentSubsystem* GetDungeonContentSubsystem() const;

    bool bStoreBuilt = false;

    FName ActiveGroupId;

    FName InspectedBuildableId;

    UPROPERTY()
    TObjectPtr<UTextBlock> HeaderManaText;

    UPROPERTY()
    TObjectPtr<UTextBlock> HeaderIncomeText;

    UPROPERTY()
    TObjectPtr<UTextBlock> ListHeadingText;

    UPROPERTY()
    TObjectPtr<UTextBlock> OfferingCountText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailKickerText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailTitleText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailRoleText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailStatsText;

    UPROPERTY()
    TObjectPtr<UTextBlock> DetailCostText;

    UPROPERTY()
    TObjectPtr<UButton> ActionButton;

    UPROPERTY()
    TObjectPtr<UTextBlock> ActionLabel;

    UPROPERTY()
    TArray<FDungeonStoreTabWidgets> Tabs;

    UPROPERTY()
    TArray<FDungeonStoreCardWidgets> Cards;
};
