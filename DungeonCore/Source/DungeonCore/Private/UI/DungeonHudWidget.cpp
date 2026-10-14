#include "UI/DungeonHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/DungeonContentSubsystem.h"
#include "Player/DungeonPlayerController.h"
#include "Sim/DungeonSimSubsystem.h"
#include "UI/DungeonShopItemButton.h"

namespace
{
    FLinearColor ShopLabelColorSelected()
    {
        return FLinearColor(1.0f, 0.85f, 0.3f, 1.0f);
    }

    FLinearColor ShopLabelColorAffordable()
    {
        return FLinearColor::White;
    }

    FLinearColor ShopLabelColorUnaffordable()
    {
        return FLinearColor(0.75f, 0.45f, 0.45f, 1.0f);
    }

    FLinearColor ShopLabelColorOwned()
    {
        return FLinearColor(0.55f, 0.85f, 0.55f, 1.0f);
    }

    FLinearColor ShopHeadingColor()
    {
        return FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    }
}

void UDungeonHudWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildHud();
}

void UDungeonHudWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (DungeonPlayerController != nullptr)
    {
        DungeonPlayerController->OnSelectedBuildableChanged.AddDynamic(this, &UDungeonHudWidget::HandleSelectedBuildableChanged);
        DungeonPlayerController->OnPlacementPreviewChanged.AddDynamic(this, &UDungeonHudWidget::HandlePlacementPreviewChanged);
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        DungeonSimSubsystem->OnResourceChanged.AddDynamic(this, &UDungeonHudWidget::HandleResourceChanged);
        DungeonSimSubsystem->OnBreachStateChanged.AddDynamic(this, &UDungeonHudWidget::HandleBreachStateChanged);
        DungeonSimSubsystem->OnWaveChanged.AddDynamic(this, &UDungeonHudWidget::HandleWaveChanged);
        DungeonSimSubsystem->OnCoreHealthChanged.AddDynamic(this, &UDungeonHudWidget::HandleCoreHealthChanged);
    }

    RefreshAllText();
}

void UDungeonHudWidget::NativeDestruct()
{
    ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (DungeonPlayerController != nullptr)
    {
        DungeonPlayerController->OnSelectedBuildableChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleSelectedBuildableChanged);
        DungeonPlayerController->OnPlacementPreviewChanged.RemoveDynamic(this, &UDungeonHudWidget::HandlePlacementPreviewChanged);
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        DungeonSimSubsystem->OnResourceChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleResourceChanged);
        DungeonSimSubsystem->OnBreachStateChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleBreachStateChanged);
        DungeonSimSubsystem->OnWaveChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleWaveChanged);
        DungeonSimSubsystem->OnCoreHealthChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleCoreHealthChanged);
    }

    for (const FDungeonShopButtonEntry& Entry : ShopButtons)
    {
        if (Entry.Button != nullptr)
        {
            Entry.Button->OnShopItemClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleShopItemClicked);
        }
    }

    Super::NativeDestruct();
}

void UDungeonHudWidget::HandleResourceChanged(const FResourceState ResourceState)
{
    RefreshManaText(ResourceState);
    RefreshShopButtons();
}

void UDungeonHudWidget::HandleBreachStateChanged(const EBreachState BreachState)
{
    RefreshBreachText(BreachState);
}

void UDungeonHudWidget::HandleSelectedBuildableChanged(const FName BuildableId)
{
    RefreshSelectedText(BuildableId);
    RefreshShopButtons();
}

void UDungeonHudWidget::HandleWaveChanged(const int32 CurrentWaveNumber)
{
    RefreshWaveText(CurrentWaveNumber);
}

void UDungeonHudWidget::HandleCoreHealthChanged(const int32 CurrentCoreHealth)
{
    RefreshCoreHealthText(CurrentCoreHealth);
}

void UDungeonHudWidget::HandlePlacementPreviewChanged(const FPlacementResult PlacementResult)
{
    RefreshPlacementPreviewText(PlacementResult);
}

void UDungeonHudWidget::HandleShopItemClicked(const FName BuildableId)
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->ActivateShopItem(BuildableId);
    }
}

void UDungeonHudWidget::BuildHud()
{
    if (bHudBuilt || WidgetTree == nullptr)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    UVerticalBox* InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
    UCanvasPanelSlot* InfoBoxSlot = RootCanvas->AddChildToCanvas(InfoBox);
    InfoBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f));
    InfoBoxSlot->SetPosition(FVector2D(24.0f, 24.0f));
    InfoBoxSlot->SetAutoSize(true);

    ManaTextBlock = CreateInfoTextBlock(TEXT("ManaText"));
    BreachTextBlock = CreateInfoTextBlock(TEXT("BreachText"));
    SelectedTextBlock = CreateInfoTextBlock(TEXT("SelectedText"));
    WaveTextBlock = CreateInfoTextBlock(TEXT("WaveText"));
    CoreHealthTextBlock = CreateInfoTextBlock(TEXT("CoreHealthText"));
    PlacementPreviewTextBlock = CreateInfoTextBlock(TEXT("PlacementPreviewText"));
    ControlsTextBlock = CreateInfoTextBlock(TEXT("ControlsText"));
    ControlsTextBlock->SetText(FText::FromString(TEXT("1-9 Select Tool | Left Click Act | LMB Drag Pan | RMB Drag Orbit | Wheel Zoom | R Restart | F5 Save | F9 Load")));

    InfoBox->AddChildToVerticalBox(ManaTextBlock);
    InfoBox->AddChildToVerticalBox(BreachTextBlock);
    InfoBox->AddChildToVerticalBox(SelectedTextBlock);
    InfoBox->AddChildToVerticalBox(WaveTextBlock);
    InfoBox->AddChildToVerticalBox(CoreHealthTextBlock);
    InfoBox->AddChildToVerticalBox(PlacementPreviewTextBlock);
    InfoBox->AddChildToVerticalBox(ControlsTextBlock);

    UVerticalBox* ShopBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ShopBox"));
    UCanvasPanelSlot* ShopBoxSlot = RootCanvas->AddChildToCanvas(ShopBox);
    ShopBoxSlot->SetAnchors(FAnchors(1.0f, 0.0f));
    ShopBoxSlot->SetAlignment(FVector2D(1.0f, 0.0f));
    ShopBoxSlot->SetPosition(FVector2D(-24.0f, 24.0f));
    ShopBoxSlot->SetAutoSize(true);

    BuildShopPanel(ShopBox);

    bHudBuilt = true;
}

void UDungeonHudWidget::BuildShopPanel(UVerticalBox* ShopBox)
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr || ShopBox == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop panel needs the content and sim subsystems."));
        return;
    }

    UTextBlock* ShopTitle = CreateInfoTextBlock(TEXT("ShopTitle"));
    ShopTitle->SetText(FText::FromString(TEXT("Shop (mana)")));
    ShopTitle->SetColorAndOpacity(FSlateColor(ShopHeadingColor()));
    ShopBox->AddChildToVerticalBox(ShopTitle);

    for (const FDungeonShopGroup& Group : ContentSubsystem->GetShopGroups())
    {
        TArray<const FDungeonBuildableRow*> VisibleRows;
        for (const FName& BuildableId : Group.BuildableIds)
        {
            const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
            if (Row == nullptr)
            {
                UE_LOG(LogTemp, Fatal, TEXT("Shop group '%s' references unknown item '%s'."), *Group.GroupId.ToString(), *BuildableId.ToString());
                return;
            }

            if (DungeonSimSubsystem->IsBuildableAvailableForRun(BuildableId))
            {
                VisibleRows.Add(Row);
            }
        }

        if (VisibleRows.Num() == 0)
        {
            continue;
        }

        UTextBlock* Heading = CreateInfoTextBlock(FName(*FString::Printf(TEXT("ShopHeading_%s"), *Group.GroupId.ToString())));
        Heading->SetText(Group.DisplayName);
        Heading->SetColorAndOpacity(FSlateColor(ShopHeadingColor()));
        UVerticalBoxSlot* HeadingSlot = ShopBox->AddChildToVerticalBox(Heading);
        HeadingSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 2.0f));

        for (const FDungeonBuildableRow* Row : VisibleRows)
        {
            const FString ButtonName = FString::Printf(TEXT("ShopButton_%s"), *Row->BuildableId.ToString());
            UDungeonShopItemButton* Button = WidgetTree->ConstructWidget<UDungeonShopItemButton>(
                UDungeonShopItemButton::StaticClass(),
                FName(*ButtonName)
            );
            Button->SetBuildableId(Row->BuildableId);
            Button->SetToolTipText(Row->RoleText);
            Button->OnShopItemClicked.AddDynamic(this, &UDungeonHudWidget::HandleShopItemClicked);

            UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            Button->AddChild(Label);

            UVerticalBoxSlot* ButtonSlot = ShopBox->AddChildToVerticalBox(Button);
            ButtonSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
            ButtonSlot->SetHorizontalAlignment(HAlign_Fill);

            FDungeonShopButtonEntry Entry;
            Entry.BuildableId = Row->BuildableId;
            Entry.Button = Button;
            Entry.Label = Label;
            ShopButtons.Add(Entry);
        }
    }

    RefreshShopButtons();
}

void UDungeonHudWidget::RefreshAllText()
{
    ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (DungeonPlayerController != nullptr)
    {
        RefreshSelectedText(DungeonPlayerController->GetSelectedBuildableId());
        RefreshPlacementPreviewText(DungeonPlayerController->GetCurrentPlacementPreview());
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        RefreshManaText(DungeonSimSubsystem->GetResourceState());
        RefreshBreachText(DungeonSimSubsystem->GetFloorState().BreachState);
        RefreshWaveText(DungeonSimSubsystem->GetCurrentWaveNumber());
        RefreshCoreHealthText(DungeonSimSubsystem->GetCurrentCoreHealth());
    }

    RefreshShopButtons();
}

void UDungeonHudWidget::RefreshManaText(const FResourceState& ResourceState)
{
    if (ManaTextBlock == nullptr)
    {
        return;
    }

    ManaTextBlock->SetText(FText::FromString(FString::Printf(
        TEXT("Mana: %d  (+%.1f/s)"),
        ResourceState.CurrentMana,
        ResourceState.ManaIncomePerSecond
    )));
}

void UDungeonHudWidget::RefreshBreachText(const EBreachState BreachState)
{
    if (BreachTextBlock == nullptr)
    {
        return;
    }

    const FString BreachLabel = BreachState == EBreachState::Open ? TEXT("Open") : TEXT("Sealed");
    BreachTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Breach: %s"), *BreachLabel)));
}

void UDungeonHudWidget::RefreshSelectedText(const FName BuildableId)
{
    if (SelectedTextBlock == nullptr)
    {
        return;
    }

    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const FDungeonBuildableRow* Row = ContentSubsystem != nullptr ? ContentSubsystem->FindBuildable(BuildableId) : nullptr;
    const FString SelectedLabel = Row != nullptr ? Row->DisplayName.ToString() : TEXT("None");
    SelectedTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Tool: %s"), *SelectedLabel)));
}

void UDungeonHudWidget::RefreshWaveText(const int32 CurrentWaveNumber)
{
    if (WaveTextBlock == nullptr)
    {
        return;
    }

    WaveTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d"), CurrentWaveNumber)));
}

void UDungeonHudWidget::RefreshCoreHealthText(const int32 CurrentCoreHealth)
{
    if (CoreHealthTextBlock == nullptr)
    {
        return;
    }

    CoreHealthTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Core HP: %d"), CurrentCoreHealth)));
}

void UDungeonHudWidget::RefreshPlacementPreviewText(const FPlacementResult& PlacementResult)
{
    if (PlacementPreviewTextBlock == nullptr)
    {
        return;
    }

    if (PlacementResult.bSuccess)
    {
        PlacementPreviewTextBlock->SetText(FText::FromString(TEXT("Placement: Valid")));
        return;
    }

    PlacementPreviewTextBlock->SetText(FText::FromString(FString::Printf(
        TEXT("Placement: %s"),
        *DescribePlacementFailure(PlacementResult.FailureReason)
    )));
}

void UDungeonHudWidget::RefreshShopButtons()
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    const ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr || DungeonPlayerController == nullptr)
    {
        return;
    }

    const TArray<FName> HotkeyIds = DungeonPlayerController->GetShopHotkeyBuildableIds();
    const FName SelectedId = DungeonPlayerController->GetSelectedBuildableId();
    const int32 CurrentMana = DungeonSimSubsystem->GetResourceState().CurrentMana;

    for (const FDungeonShopButtonEntry& Entry : ShopButtons)
    {
        const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(Entry.BuildableId);
        if (Row == nullptr || Entry.Button == nullptr || Entry.Label == nullptr)
        {
            continue;
        }

        const int32 HotkeyIndex = HotkeyIds.IndexOfByKey(Entry.BuildableId);
        Entry.Label->SetText(MakeShopButtonLabel(*Row, HotkeyIndex));

        if (Row->Category == EBuildableCategory::Upgrade)
        {
            const FPlacementResult Preview = DungeonSimSubsystem->PreviewUpgradePurchase(Entry.BuildableId);
            const bool bOwned = Preview.FailureReason == EPlacementFailureReason::AlreadyOwned;
            Entry.Button->SetIsEnabled(Preview.bSuccess);
            Entry.Label->SetColorAndOpacity(FSlateColor(
                bOwned ? ShopLabelColorOwned()
                : Preview.bSuccess ? ShopLabelColorAffordable()
                : ShopLabelColorUnaffordable()
            ));
            continue;
        }

        Entry.Button->SetIsEnabled(true);
        const bool bSelected = Entry.BuildableId == SelectedId;
        const bool bAffordable = CurrentMana >= Row->ManaCost;
        Entry.Label->SetColorAndOpacity(FSlateColor(
            bSelected ? ShopLabelColorSelected()
            : bAffordable ? ShopLabelColorAffordable()
            : ShopLabelColorUnaffordable()
        ));
    }
}

FText UDungeonHudWidget::MakeShopButtonLabel(const FDungeonBuildableRow& Row, const int32 HotkeyIndex) const
{
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();

    FString Prefix;
    if (HotkeyIndex != INDEX_NONE)
    {
        Prefix = FString::Printf(TEXT("[%d] "), HotkeyIndex + 1);
    }

    FString Suffix;
    if (Row.Category == EBuildableCategory::Upgrade && DungeonSimSubsystem != nullptr)
    {
        const FPlacementResult Preview = DungeonSimSubsystem->PreviewUpgradePurchase(Row.BuildableId);
        if (Preview.FailureReason == EPlacementFailureReason::AlreadyOwned)
        {
            Suffix = TEXT("  (owned)");
        }
        else if (Preview.FailureReason == EPlacementFailureReason::MissingPrerequisite)
        {
            Suffix = TEXT("  (locked)");
        }
    }

    FString Income;
    if (Row.ManaPerSecond > 0.0f)
    {
        Income = FString::Printf(TEXT("  +%.1f/s"), Row.ManaPerSecond);
    }
    if (Row.ManaPerAdventurerPerSecond > 0.0f)
    {
        Income += FString::Printf(TEXT("  +%.2f/s per intruder"), Row.ManaPerAdventurerPerSecond);
    }

    return FText::FromString(FString::Printf(
        TEXT("%s%s  %d%s%s"),
        *Prefix,
        *Row.DisplayName.ToString(),
        Row.ManaCost,
        *Income,
        *Suffix
    ));
}

FString UDungeonHudWidget::DescribePlacementFailure(const EPlacementFailureReason FailureReason) const
{
    switch (FailureReason)
    {
    case EPlacementFailureReason::None:
        return TEXT("Waiting for target");
    case EPlacementFailureReason::OutOfBounds:
        return TEXT("Out of bounds");
    case EPlacementFailureReason::Blocked:
        return TEXT("Blocked");
    case EPlacementFailureReason::InsufficientMana:
        return TEXT("Not enough mana");
    case EPlacementFailureReason::RequiresClearedTile:
        return TEXT("Needs cleared tile");
    case EPlacementFailureReason::WouldDisconnectPath:
        return TEXT("Must dig from reachable space");
    case EPlacementFailureReason::InvalidFootprint:
        return TEXT("Invalid footprint");
    case EPlacementFailureReason::FamilyLocked:
        return TEXT("Family locked");
    case EPlacementFailureReason::ThemeLocked:
        return TEXT("Theme locked");
    case EPlacementFailureReason::AlreadyOwned:
        return TEXT("Already owned");
    case EPlacementFailureReason::MissingPrerequisite:
        return TEXT("Requires an earlier upgrade");
    case EPlacementFailureReason::UnknownBuildable:
        return TEXT("No tool selected");
    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled placement failure reason in HUD."));
        return TEXT("");
    }
}

ADungeonPlayerController* UDungeonHudWidget::GetDungeonPlayerController() const
{
    return GetOwningPlayer<ADungeonPlayerController>();
}

UDungeonSimSubsystem* UDungeonHudWidget::GetDungeonSimSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonSimSubsystem>() : nullptr;
}

UDungeonContentSubsystem* UDungeonHudWidget::GetDungeonContentSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonContentSubsystem>() : nullptr;
}

UTextBlock* UDungeonHudWidget::CreateInfoTextBlock(const FName WidgetName)
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
    TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    return TextBlock;
}
