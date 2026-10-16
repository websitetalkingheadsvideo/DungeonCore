#include "UI/DungeonStoreWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/DungeonBuildableRow.h"
#include "Data/DungeonContentSubsystem.h"
#include "Player/DungeonPlayerController.h"
#include "Sim/DungeonSimSubsystem.h"
#include "UI/DungeonShopItemButton.h"
#include "UI/DungeonStoreStyle.h"

namespace
{
    struct FStoreItemStatus
    {
        bool bOwned = false;
        bool bLocked = false;
        bool bAffordable = false;
        bool bEquipped = false;
        FString RequirementName;
    };

    struct FStoreActionView
    {
        FString Label;
        bool bEnabled = false;
        bool bClosesStore = false;
    };

    void StyleText(UTextBlock* Text, const FName Typeface, const int32 Size, const int32 LetterSpacing, const FLinearColor& Color)
    {
        Text->SetFont(DungeonStoreStyle::MakeFont(Typeface, Size, LetterSpacing));
        Text->SetColorAndOpacity(FSlateColor(Color));
        Text->SetShadowOffset(FVector2D(0.0f, 1.0f));
        Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f));
    }

    void PassClicksToButton(UWidget* Widget)
    {
        Widget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    FString MakeMonogram(const FDungeonBuildableRow& Row)
    {
        if (Row.BuildableId == FName(TEXT("core_attunement_1")))
        {
            return TEXT("I");
        }
        if (Row.BuildableId == FName(TEXT("core_attunement_2")))
        {
            return TEXT("II");
        }
        if (Row.BuildableId == FName(TEXT("core_attunement_3")))
        {
            return TEXT("III");
        }

        const FString Name = Row.DisplayName.ToString();
        FString Letters;
        bool bTakeNext = true;
        for (const TCHAR Character : Name)
        {
            if (FChar::IsWhitespace(Character))
            {
                bTakeNext = true;
                continue;
            }
            if (bTakeNext && FChar::IsAlnum(Character))
            {
                Letters.AppendChar(FChar::ToUpper(Character));
                bTakeNext = false;
            }
        }

        if (Letters.Len() == 1 && Name.Len() > 1)
        {
            Letters.AppendChar(FChar::ToUpper(Name[1]));
        }
        if (Letters.Len() > 2)
        {
            Letters = Letters.Left(2);
        }
        if (Letters.IsEmpty())
        {
            return TEXT("--");
        }
        return Letters;
    }

    const TCHAR* CategoryKicker(const EBuildableCategory Category)
    {
        switch (Category)
        {
        case EBuildableCategory::Dig:
            return TEXT("EXCAVATION");
        case EBuildableCategory::Room:
            return TEXT("CHAMBER");
        case EBuildableCategory::Trap:
            return TEXT("TRAP");
        case EBuildableCategory::Upgrade:
            return TEXT("ATTUNEMENT");
        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled buildable category in the vault."));
            return TEXT("");
        }
    }

    FLinearColor CategoryAccent(const EBuildableCategory Category)
    {
        switch (Category)
        {
        case EBuildableCategory::Dig:
            return DungeonStoreStyle::Ivory();
        case EBuildableCategory::Room:
            return DungeonStoreStyle::Violet();
        case EBuildableCategory::Trap:
            return DungeonStoreStyle::Danger();
        case EBuildableCategory::Upgrade:
            return DungeonStoreStyle::Gold();
        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled buildable category accent in the vault."));
            return DungeonStoreStyle::Ivory();
        }
    }

    FString IncomeLine(const FDungeonBuildableRow& Row)
    {
        FString Line;
        if (Row.ManaPerSecond > 0.0f)
        {
            Line = FString::Printf(TEXT("+%.1f mana / sec"), Row.ManaPerSecond);
        }
        if (Row.ManaPerAdventurerPerSecond > 0.0f)
        {
            if (!Line.IsEmpty())
            {
                Line += TEXT("\n");
            }
            Line += FString::Printf(TEXT("+%.2f / sec for each intruder inside"), Row.ManaPerAdventurerPerSecond);
        }
        return Line;
    }

    FString FootprintLine(const FDungeonBuildableRow& Row)
    {
        switch (Row.Category)
        {
        case EBuildableCategory::Upgrade:
            return TEXT("No tile. The core keeps this.");
        case EBuildableCategory::Room:
            return FString::Printf(TEXT("Footprint  %d × %d"), Row.Footprint.Width, Row.Footprint.Height);
        case EBuildableCategory::Dig:
        case EBuildableCategory::Trap:
            return TEXT("Footprint  1 × 1");
        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled footprint category in the vault."));
            return TEXT("");
        }
    }

    FStoreItemStatus ReadStatus(
        const FDungeonBuildableRow& Row,
        const UDungeonSimSubsystem& Sim,
        const UDungeonContentSubsystem& Content,
        const FName EquippedId
    )
    {
        FStoreItemStatus Status;
        Status.bAffordable = Sim.GetResourceState().CurrentMana >= Row.ManaCost;
        Status.bEquipped = Row.Category != EBuildableCategory::Upgrade && Row.BuildableId == EquippedId;

        if (Row.Category == EBuildableCategory::Upgrade)
        {
            const FPlacementResult Preview = Sim.PreviewUpgradePurchase(Row.BuildableId);
            Status.bOwned = Preview.FailureReason == EPlacementFailureReason::AlreadyOwned;
            Status.bLocked = Preview.FailureReason == EPlacementFailureReason::MissingPrerequisite;
            if (Status.bLocked && !Row.RequiresBuildableId.IsNone())
            {
                const FDungeonBuildableRow* Required = Content.FindBuildable(Row.RequiresBuildableId);
                Status.RequirementName = Required != nullptr ? Required->DisplayName.ToString() : Row.RequiresBuildableId.ToString();
            }
        }
        return Status;
    }

    FStoreActionView ResolveAction(const FDungeonBuildableRow& Row, const FStoreItemStatus& Status)
    {
        FStoreActionView Action;
        if (Row.Category == EBuildableCategory::Upgrade)
        {
            if (Status.bOwned)
            {
                Action.Label = TEXT("ATTUNED");
                return Action;
            }
            if (Status.bLocked)
            {
                Action.Label = TEXT("LOCKED");
                return Action;
            }
            if (!Status.bAffordable)
            {
                Action.Label = TEXT("NOT ENOUGH MANA");
                return Action;
            }
            Action.Label = TEXT("ATTUNE");
            Action.bEnabled = true;
            return Action;
        }

        Action.Label = Status.bEquipped ? TEXT("RETURN TO THE GRID") : TEXT("TAKE TO THE GRID");
        Action.bEnabled = true;
        Action.bClosesStore = true;
        return Action;
    }

    FString RowSubtitle(const FDungeonBuildableRow& Row, const FStoreItemStatus& Status, const int32 HotkeyIndex)
    {
        FString Line;
        if (HotkeyIndex != INDEX_NONE)
        {
            Line = FString::Printf(TEXT("%d   "), HotkeyIndex + 1);
        }
        if (Status.bOwned)
        {
            return Line + TEXT("Attuned");
        }
        if (Status.bLocked)
        {
            return Line + FString::Printf(TEXT("Requires %s"), *Status.RequirementName);
        }
        if (Status.bEquipped)
        {
            Line += TEXT("Equipped   ");
        }

        const FString Income = IncomeLine(Row);
        if (!Income.IsEmpty())
        {
            FString FirstLine;
            Income.Split(TEXT("\n"), &FirstLine, nullptr);
            return Line + (FirstLine.IsEmpty() ? Income : FirstLine);
        }
        return Line + CategoryKicker(Row.Category);
    }
}

void UDungeonStoreWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildStore();
}

void UDungeonStoreWidget::NativeDestruct()
{
    for (const FDungeonStoreTabWidgets& Tab : Tabs)
    {
        if (Tab.Button != nullptr)
        {
            Tab.Button->OnShopItemClicked.RemoveDynamic(this, &UDungeonStoreWidget::HandleTabClicked);
        }
    }
    for (const FDungeonStoreCardWidgets& Card : Cards)
    {
        if (Card.Button != nullptr)
        {
            Card.Button->OnShopItemClicked.RemoveDynamic(this, &UDungeonStoreWidget::HandleCardClicked);
        }
    }
    Super::NativeDestruct();
}

void UDungeonStoreWidget::HandleScrimClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetOwningPlayer<ADungeonPlayerController>())
    {
        DungeonPlayerController->SetDungeonStoreOpen(false);
    }
}

void UDungeonStoreWidget::HandleCloseClicked()
{
    HandleScrimClicked();
}

void UDungeonStoreWidget::HandleActionClicked()
{
    ADungeonPlayerController* DungeonPlayerController = GetOwningPlayer<ADungeonPlayerController>();
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (DungeonPlayerController == nullptr || ContentSubsystem == nullptr || InspectedBuildableId.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Vault action ignored; player or catalog is missing."));
        return;
    }

    const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(InspectedBuildableId);
    if (Row == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Vault action ignored; '%s' is not in the catalog."), *InspectedBuildableId.ToString());
        return;
    }

    const UDungeonSimSubsystem* Sim = GetDungeonSimSubsystem();
    if (Sim == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Vault action ignored; sim subsystem is missing."));
        return;
    }

    const FStoreItemStatus Status = ReadStatus(*Row, *Sim, *ContentSubsystem, DungeonPlayerController->GetSelectedBuildableId());
    const FStoreActionView Action = ResolveAction(*Row, Status);
    if (!Action.bEnabled)
    {
        return;
    }

    DungeonPlayerController->ActivateShopItem(InspectedBuildableId);
    if (Action.bClosesStore)
    {
        DungeonPlayerController->SetDungeonStoreOpen(false);
    }
}

void UDungeonStoreWidget::HandleTabClicked(const FName GroupId)
{
    ShowGroup(GroupId);
    RefreshStore();
}

void UDungeonStoreWidget::HandleCardClicked(const FName BuildableId)
{
    InspectedBuildableId = BuildableId;
    RefreshStore();
}

void UDungeonStoreWidget::ShowGroup(const FName GroupId)
{
    ActiveGroupId = GroupId;
    bool bInspectedInGroup = false;
    for (const FDungeonStoreCardWidgets& Card : Cards)
    {
        if (Card.BuildableId == InspectedBuildableId && Card.GroupId == GroupId)
        {
            bInspectedInGroup = true;
            break;
        }
    }
    if (!bInspectedInGroup)
    {
        InspectedBuildableId = FirstCardInGroup(GroupId);
    }
}

FName UDungeonStoreWidget::FirstCardInGroup(const FName GroupId) const
{
    for (const FDungeonStoreCardWidgets& Card : Cards)
    {
        if (Card.GroupId == GroupId)
        {
            return Card.BuildableId;
        }
    }
    return NAME_None;
}

void UDungeonStoreWidget::BuildStore()
{
    if (bStoreBuilt || WidgetTree == nullptr)
    {
        return;
    }

    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Vault needs the content and sim subsystems."));
        return;
    }

    UCanvasPanel* StoreCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("StoreCanvas"));
    WidgetTree->RootWidget = StoreCanvas;

    UButton* Scrim = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreScrim"));
    Scrim->SetStyle(DungeonStoreStyle::MakeButtonStyle(
        DungeonStoreStyle::ScrimTint(),
        DungeonStoreStyle::ScrimTint(),
        DungeonStoreStyle::ScrimTint(),
        FLinearColor::Transparent,
        0.0f,
        0.0f
    ));
    Scrim->OnClicked.AddDynamic(this, &UDungeonStoreWidget::HandleScrimClicked);
    UCanvasPanelSlot* ScrimSlot = StoreCanvas->AddChildToCanvas(Scrim);
    ScrimSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    ScrimSlot->SetOffsets(FMargin(0.0f));

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StorePanel"));
    Panel->SetPadding(FMargin(28.0f, 22.0f, 28.0f, 18.0f));
    Panel->SetBrush(DungeonStoreStyle::MakeRoundedBrush(DungeonStoreStyle::Ink(), DungeonStoreStyle::GoldDim(), 18.0f, 1.25f));
    Panel->SetClipping(EWidgetClipping::ClipToBounds);
    UCanvasPanelSlot* PanelSlot = StoreCanvas->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.07f, 0.08f, 0.93f, 0.92f));
    PanelSlot->SetOffsets(FMargin(0.0f));
    PanelSlot->SetZOrder(2);

    UVerticalBox* PanelColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreColumn"));
    Panel->AddChild(PanelColumn);

    UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StoreHeader"));
    UVerticalBoxSlot* HeaderSlot = PanelColumn->AddChildToVerticalBox(Header);
    HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));

    UVerticalBox* Wordmark = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreWordmark"));
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreTitle"));
    Title->SetText(FText::FromString(TEXT("CORE VAULT")));
    StyleText(Title, TEXT("Bold"), 26, 420, DungeonStoreStyle::Ivory());
    UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreSubtitle"));
    Subtitle->SetText(FText::FromString(TEXT("Mana buys stone, snares, and a louder core.")));
    StyleText(Subtitle, TEXT("Regular"), 13, 40, DungeonStoreStyle::Muted());
    Wordmark->AddChildToVerticalBox(Title);
    Wordmark->AddChildToVerticalBox(Subtitle);
    UHorizontalBoxSlot* WordmarkSlot = Header->AddChildToHorizontalBox(Wordmark);
    WordmarkSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    WordmarkSlot->SetVerticalAlignment(VAlign_Center);

    UVerticalBox* Balance = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreBalance"));
    UTextBlock* BalanceKicker = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreBalanceKicker"));
    BalanceKicker->SetText(FText::FromString(TEXT("MANA")));
    StyleText(BalanceKicker, TEXT("Bold"), 11, 280, DungeonStoreStyle::Gold());
    BalanceKicker->SetJustification(ETextJustify::Right);
    HeaderManaText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreMana"));
    StyleText(HeaderManaText, TEXT("Bold"), 36, 0, DungeonStoreStyle::Gold());
    HeaderManaText->SetJustification(ETextJustify::Right);
    HeaderIncomeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreIncome"));
    StyleText(HeaderIncomeText, TEXT("Regular"), 13, 40, DungeonStoreStyle::Violet());
    HeaderIncomeText->SetJustification(ETextJustify::Right);
    Balance->AddChildToVerticalBox(BalanceKicker);
    Balance->AddChildToVerticalBox(HeaderManaText);
    Balance->AddChildToVerticalBox(HeaderIncomeText);
    UHorizontalBoxSlot* BalanceSlot = Header->AddChildToHorizontalBox(Balance);
    BalanceSlot->SetVerticalAlignment(VAlign_Center);
    BalanceSlot->SetPadding(FMargin(24.0f, 0.0f, 18.0f, 0.0f));

    UButton* CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreClose"));
    CloseButton->SetStyle(DungeonStoreStyle::MakeButtonStyle(
        DungeonStoreStyle::CardFill(),
        DungeonStoreStyle::CardHover(),
        DungeonStoreStyle::CardSelected(),
        DungeonStoreStyle::GoldDim(),
        10.0f,
        1.0f
    ));
    CloseButton->OnClicked.AddDynamic(this, &UDungeonStoreWidget::HandleCloseClicked);
    UTextBlock* CloseLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreCloseLabel"));
    CloseLabel->SetText(FText::FromString(TEXT("CLOSE")));
    StyleText(CloseLabel, TEXT("Bold"), 12, 220, DungeonStoreStyle::Ivory());
    CloseLabel->SetJustification(ETextJustify::Center);
    PassClicksToButton(CloseLabel);
    CloseButton->AddChild(CloseLabel);
    USizeBox* CloseSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StoreCloseSize"));
    CloseSize->SetWidthOverride(108.0f);
    CloseSize->SetHeightOverride(44.0f);
    CloseSize->AddChild(CloseButton);
    UHorizontalBoxSlot* CloseSlot = Header->AddChildToHorizontalBox(CloseSize);
    CloseSlot->SetVerticalAlignment(VAlign_Center);

    UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StoreBody"));
    UVerticalBoxSlot* BodySlot = PanelColumn->AddChildToVerticalBox(Body);
    BodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UVerticalBox* TabColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreTabs"));
    USizeBox* TabSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StoreTabSize"));
    TabSize->SetWidthOverride(200.0f);
    TabSize->AddChild(TabColumn);
    UHorizontalBoxSlot* TabSlot = Body->AddChildToHorizontalBox(TabSize);
    TabSlot->SetVerticalAlignment(VAlign_Fill);
    TabSlot->SetPadding(FMargin(0.0f, 8.0f, 18.0f, 0.0f));

    UVerticalBox* ListColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreListColumn"));
    ListHeadingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreListHeading"));
    StyleText(ListHeadingText, TEXT("Bold"), 20, 80, DungeonStoreStyle::Ivory());
    OfferingCountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreOfferingCount"));
    StyleText(OfferingCountText, TEXT("Bold"), 11, 240, DungeonStoreStyle::Muted());
    ListColumn->AddChildToVerticalBox(ListHeadingText);
    UVerticalBoxSlot* CountSlot = ListColumn->AddChildToVerticalBox(OfferingCountText);
    CountSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 12.0f));

    UScrollBox* ListScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StoreList"));
    ListScroll->SetScrollBarVisibility(ESlateVisibility::Collapsed);
    ListScroll->SetAnimateWheelScrolling(true);
    ListScroll->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
    UVerticalBoxSlot* ScrollSlot = ListColumn->AddChildToVerticalBox(ListScroll);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UHorizontalBoxSlot* ListSlot = Body->AddChildToHorizontalBox(ListColumn);
    ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ListSlot->SetVerticalAlignment(VAlign_Fill);
    ListSlot->SetPadding(FMargin(0.0f, 8.0f, 18.0f, 0.0f));

    UBorder* Dossier = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StoreDossier"));
    Dossier->SetPadding(FMargin(20.0f, 18.0f, 20.0f, 18.0f));
    Dossier->SetBrush(DungeonStoreStyle::MakeRoundedBrush(DungeonStoreStyle::InkRaised(), DungeonStoreStyle::GoldDim(), 14.0f, 1.0f));
    UVerticalBox* DossierColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StoreDossierColumn"));
    Dossier->AddChild(DossierColumn);

    DetailKickerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreDetailKicker"));
    StyleText(DetailKickerText, TEXT("Bold"), 11, 280, DungeonStoreStyle::Violet());
    DetailTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreDetailTitle"));
    StyleText(DetailTitleText, TEXT("Bold"), 28, 0, DungeonStoreStyle::Ivory());
    DetailTitleText->SetAutoWrapText(true);
    DetailRoleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreDetailRole"));
    StyleText(DetailRoleText, TEXT("Regular"), 14, 10, DungeonStoreStyle::Muted());
    DetailRoleText->SetAutoWrapText(true);
    DetailStatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreDetailStats"));
    StyleText(DetailStatsText, TEXT("Regular"), 14, 10, DungeonStoreStyle::Ivory());
    DetailStatsText->SetAutoWrapText(true);
    DetailCostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreDetailCost"));
    StyleText(DetailCostText, TEXT("Bold"), 40, 0, DungeonStoreStyle::Gold());

    DossierColumn->AddChildToVerticalBox(DetailKickerText);
    UVerticalBoxSlot* TitleSlot = DossierColumn->AddChildToVerticalBox(DetailTitleText);
    TitleSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 10.0f));
    UVerticalBoxSlot* RoleSlot = DossierColumn->AddChildToVerticalBox(DetailRoleText);
    RoleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 16.0f));
    DossierColumn->AddChildToVerticalBox(DetailStatsText);

    USpacer* DossierPush = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("StoreDossierPush"));
    UVerticalBoxSlot* PushSlot = DossierColumn->AddChildToVerticalBox(DossierPush);
    PushSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

    UTextBlock* CostKicker = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreCostKicker"));
    CostKicker->SetText(FText::FromString(TEXT("COST")));
    StyleText(CostKicker, TEXT("Bold"), 11, 280, DungeonStoreStyle::Muted());
    DossierColumn->AddChildToVerticalBox(CostKicker);
    DossierColumn->AddChildToVerticalBox(DetailCostText);

    ActionButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StoreAction"));
    ActionButton->OnClicked.AddDynamic(this, &UDungeonStoreWidget::HandleActionClicked);
    ActionLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreActionLabel"));
    ActionLabel->SetJustification(ETextJustify::Center);
    PassClicksToButton(ActionLabel);
    ActionButton->AddChild(ActionLabel);
    USizeBox* ActionSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StoreActionSize"));
    ActionSize->SetHeightOverride(52.0f);
    ActionSize->AddChild(ActionButton);
    UVerticalBoxSlot* ActionSlot = DossierColumn->AddChildToVerticalBox(ActionSize);
    ActionSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    USizeBox* DossierSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StoreDossierSize"));
    DossierSize->SetWidthOverride(320.0f);
    DossierSize->AddChild(Dossier);
    UHorizontalBoxSlot* DossierSlot = Body->AddChildToHorizontalBox(DossierSize);
    DossierSlot->SetVerticalAlignment(VAlign_Fill);
    DossierSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));

    UTextBlock* Footer = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StoreFooter"));
    Footer->SetText(FText::FromString(TEXT("B  CLOSE      ATTUNE SPENDS NOW      CHAMBERS AND TRAPS ARM A TOOL")));
    StyleText(Footer, TEXT("Bold"), 10, 180, DungeonStoreStyle::Muted());
    UVerticalBoxSlot* FooterSlot = PanelColumn->AddChildToVerticalBox(Footer);
    FooterSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 0.0f));

    for (const FDungeonShopGroup& Group : ContentSubsystem->GetShopGroups())
    {
        TArray<const FDungeonBuildableRow*> VisibleRows;
        for (const FName& BuildableId : Group.BuildableIds)
        {
            const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
            if (Row == nullptr)
            {
                UE_LOG(LogTemp, Fatal, TEXT("Vault group '%s' references unknown item '%s'."), *Group.GroupId.ToString(), *BuildableId.ToString());
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

        const FString TabName = FString::Printf(TEXT("StoreTab_%s"), *Group.GroupId.ToString());
        UDungeonShopItemButton* TabButton = WidgetTree->ConstructWidget<UDungeonShopItemButton>(UDungeonShopItemButton::StaticClass(), FName(*TabName));
        TabButton->SetBuildableId(Group.GroupId);
        TabButton->OnShopItemClicked.AddDynamic(this, &UDungeonStoreWidget::HandleTabClicked);

        UHorizontalBox* TabBody = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        PassClicksToButton(TabBody);
        UBorder* Accent = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        Accent->SetBrush(DungeonStoreStyle::MakeRoundedBrush(DungeonStoreStyle::Gold(), DungeonStoreStyle::Gold(), 2.0f, 0.0f));
        PassClicksToButton(Accent);
        USizeBox* AccentSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        AccentSize->SetWidthOverride(3.0f);
        AccentSize->SetHeightOverride(18.0f);
        AccentSize->AddChild(Accent);
        PassClicksToButton(AccentSize);
        UHorizontalBoxSlot* AccentSlot = TabBody->AddChildToHorizontalBox(AccentSize);
        AccentSlot->SetVerticalAlignment(VAlign_Center);
        AccentSlot->SetPadding(FMargin(12.0f, 0.0f, 10.0f, 0.0f));

        UTextBlock* TabLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        TabLabel->SetText(Group.DisplayName);
        StyleText(TabLabel, TEXT("Bold"), 13, 160, DungeonStoreStyle::Ivory());
        PassClicksToButton(TabLabel);
        UHorizontalBoxSlot* LabelSlot = TabBody->AddChildToHorizontalBox(TabLabel);
        LabelSlot->SetVerticalAlignment(VAlign_Center);
        TabButton->AddChild(TabBody);

        USizeBox* TabRowSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        TabRowSize->SetHeightOverride(46.0f);
        TabRowSize->AddChild(TabButton);
        UVerticalBoxSlot* TabRowSlot = TabColumn->AddChildToVerticalBox(TabRowSize);
        TabRowSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

        FDungeonStoreTabWidgets Tab;
        Tab.GroupId = Group.GroupId;
        Tab.GroupDisplayName = Group.DisplayName;
        Tab.Button = TabButton;
        Tab.Label = TabLabel;
        Tab.Accent = Accent;
        Tabs.Add(Tab);

        for (const FDungeonBuildableRow* Row : VisibleRows)
        {
            const FString CardName = FString::Printf(TEXT("StoreCard_%s"), *Row->BuildableId.ToString());
            UDungeonShopItemButton* CardButton = WidgetTree->ConstructWidget<UDungeonShopItemButton>(
                UDungeonShopItemButton::StaticClass(),
                FName(*CardName)
            );
            CardButton->SetBuildableId(Row->BuildableId);
            CardButton->OnShopItemClicked.AddDynamic(this, &UDungeonStoreWidget::HandleCardClicked);

            UHorizontalBox* CardBody = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
            PassClicksToButton(CardBody);

            UBorder* MonogramPlate = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
            MonogramPlate->SetPadding(FMargin(0.0f));
            PassClicksToButton(MonogramPlate);
            UTextBlock* MonogramText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            MonogramText->SetText(FText::FromString(MakeMonogram(*Row)));
            MonogramText->SetJustification(ETextJustify::Center);
            StyleText(MonogramText, TEXT("Bold"), 16, 40, DungeonStoreStyle::Ivory());
            PassClicksToButton(MonogramText);
            MonogramPlate->AddChild(MonogramText);
            if (UBorderSlot* MonogramContentSlot = Cast<UBorderSlot>(MonogramText->Slot))
            {
                MonogramContentSlot->SetHorizontalAlignment(HAlign_Center);
                MonogramContentSlot->SetVerticalAlignment(VAlign_Center);
            }
            USizeBox* MonogramSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
            MonogramSize->SetWidthOverride(52.0f);
            MonogramSize->SetHeightOverride(52.0f);
            MonogramSize->AddChild(MonogramPlate);
            PassClicksToButton(MonogramSize);
            UHorizontalBoxSlot* MonogramSlot = CardBody->AddChildToHorizontalBox(MonogramSize);
            MonogramSlot->SetVerticalAlignment(VAlign_Center);
            MonogramSlot->SetPadding(FMargin(12.0f, 0.0f, 12.0f, 0.0f));

            UVerticalBox* CardCopy = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
            PassClicksToButton(CardCopy);
            UTextBlock* NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            NameText->SetText(Row->DisplayName);
            StyleText(NameText, TEXT("Bold"), 16, 20, DungeonStoreStyle::Ivory());
            PassClicksToButton(NameText);
            UTextBlock* DetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            StyleText(DetailText, TEXT("Regular"), 12, 20, DungeonStoreStyle::Muted());
            PassClicksToButton(DetailText);
            CardCopy->AddChildToVerticalBox(NameText);
            CardCopy->AddChildToVerticalBox(DetailText);
            UHorizontalBoxSlot* CopySlot = CardBody->AddChildToHorizontalBox(CardCopy);
            CopySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            CopySlot->SetVerticalAlignment(VAlign_Center);

            UTextBlock* CostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            CostText->SetJustification(ETextJustify::Right);
            StyleText(CostText, TEXT("Bold"), 16, 0, DungeonStoreStyle::Gold());
            PassClicksToButton(CostText);
            USizeBox* CostSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
            CostSize->SetWidthOverride(72.0f);
            CostSize->AddChild(CostText);
            PassClicksToButton(CostSize);
            UHorizontalBoxSlot* CostSlot = CardBody->AddChildToHorizontalBox(CostSize);
            CostSlot->SetVerticalAlignment(VAlign_Center);
            CostSlot->SetPadding(FMargin(8.0f, 0.0f, 14.0f, 0.0f));

            CardButton->AddChild(CardBody);
            USizeBox* CardSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
            CardSize->SetHeightOverride(76.0f);
            CardSize->AddChild(CardButton);
            ListScroll->AddChild(CardSize);
            if (UScrollBoxSlot* CardScrollSlot = Cast<UScrollBoxSlot>(CardSize->Slot))
            {
                CardScrollSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
            }

            FDungeonStoreCardWidgets Card;
            Card.BuildableId = Row->BuildableId;
            Card.GroupId = Group.GroupId;
            Card.Root = CardSize;
            Card.Button = CardButton;
            Card.MonogramPlate = MonogramPlate;
            Card.MonogramText = MonogramText;
            Card.NameText = NameText;
            Card.DetailText = DetailText;
            Card.CostText = CostText;
            Cards.Add(Card);
        }
    }

    if (Tabs.Num() > 0)
    {
        ShowGroup(Tabs[0].GroupId);
    }

    bStoreBuilt = true;
    RefreshStore();
}

void UDungeonStoreWidget::RefreshStore()
{
    if (!bStoreBuilt)
    {
        return;
    }

    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Vault refresh skipped; subsystems are missing."));
        return;
    }

    const ADungeonPlayerController* DungeonPlayerController = GetOwningPlayer<ADungeonPlayerController>();
    const FName EquippedId = DungeonPlayerController != nullptr ? DungeonPlayerController->GetSelectedBuildableId() : NAME_None;
    const TArray<FName> HotkeyIds = DungeonPlayerController != nullptr
        ? DungeonPlayerController->GetShopHotkeyBuildableIds()
        : TArray<FName>();
    const FResourceState Resources = DungeonSimSubsystem->GetResourceState();

    if (HeaderManaText != nullptr)
    {
        HeaderManaText->SetText(FText::AsNumber(Resources.CurrentMana));
    }
    if (HeaderIncomeText != nullptr)
    {
        HeaderIncomeText->SetText(FText::FromString(FString::Printf(TEXT("+%.1f / sec"), Resources.ManaIncomePerSecond)));
    }

    int32 VisibleCount = 0;
    FText ActiveGroupName;
    for (const FDungeonStoreTabWidgets& Tab : Tabs)
    {
        const bool bActive = Tab.GroupId == ActiveGroupId;
        if (bActive)
        {
            ActiveGroupName = Tab.GroupDisplayName;
        }
        if (Tab.Button != nullptr)
        {
            Tab.Button->SetStyle(DungeonStoreStyle::MakeButtonStyle(
                bActive ? DungeonStoreStyle::CardSelected() : FLinearColor(0.0f, 0.0f, 0.0f, 0.0f),
                DungeonStoreStyle::CardHover(),
                DungeonStoreStyle::CardSelected(),
                bActive ? DungeonStoreStyle::GoldDim() : FLinearColor::Transparent,
                10.0f,
                1.0f
            ));
        }
        if (Tab.Label != nullptr)
        {
            Tab.Label->SetColorAndOpacity(FSlateColor(bActive ? DungeonStoreStyle::Gold() : DungeonStoreStyle::Muted()));
        }
        if (Tab.Accent != nullptr)
        {
            const FLinearColor Accent = bActive ? DungeonStoreStyle::Gold() : FLinearColor::Transparent;
            Tab.Accent->SetBrush(DungeonStoreStyle::MakeRoundedBrush(Accent, Accent, 2.0f, 0.0f));
        }
    }

    for (const FDungeonStoreCardWidgets& Card : Cards)
    {
        const bool bInGroup = Card.GroupId == ActiveGroupId;
        if (Card.Root != nullptr)
        {
            Card.Root->SetVisibility(bInGroup ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        }
        if (!bInGroup)
        {
            continue;
        }
        ++VisibleCount;

        const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(Card.BuildableId);
        if (Row == nullptr || Card.Button == nullptr)
        {
            continue;
        }

        const FStoreItemStatus Status = ReadStatus(*Row, *DungeonSimSubsystem, *ContentSubsystem, EquippedId);
        const bool bInspected = Card.BuildableId == InspectedBuildableId;
        const FLinearColor Accent = Status.bOwned ? DungeonStoreStyle::OwnedGreen() : CategoryAccent(Row->Category);
        const FLinearColor Outline = bInspected ? Accent : (Status.bOwned ? DungeonStoreStyle::OwnedGreen() : DungeonStoreStyle::GoldDim());
        const FLinearColor Fill = bInspected ? DungeonStoreStyle::CardSelected() : DungeonStoreStyle::CardFill();

        Card.Button->SetStyle(DungeonStoreStyle::MakeButtonStyle(
            Fill,
            DungeonStoreStyle::CardHover(),
            DungeonStoreStyle::CardSelected(),
            Outline,
            12.0f,
            bInspected ? 1.5f : 1.0f
        ));
        Card.Button->SetRenderOpacity(Status.bLocked ? 0.55f : 1.0f);

        if (Card.MonogramPlate != nullptr)
        {
            const FLinearColor Plate(Accent.R * 0.22f, Accent.G * 0.22f, Accent.B * 0.22f, 1.0f);
            Card.MonogramPlate->SetBrush(DungeonStoreStyle::MakeRoundedBrush(Plate, Accent, 8.0f, 1.0f));
        }
        if (Card.MonogramText != nullptr)
        {
            Card.MonogramText->SetColorAndOpacity(FSlateColor(Accent));
        }
        if (Card.DetailText != nullptr)
        {
            const int32 HotkeyIndex = HotkeyIds.IndexOfByKey(Card.BuildableId);
            Card.DetailText->SetText(FText::FromString(RowSubtitle(*Row, Status, HotkeyIndex)));
        }
        if (Card.CostText != nullptr)
        {
            Card.CostText->SetText(FText::AsNumber(Row->ManaCost));
            const FLinearColor CostColor = Status.bOwned
                ? DungeonStoreStyle::OwnedGreen()
                : (Status.bAffordable ? DungeonStoreStyle::Gold() : DungeonStoreStyle::Danger());
            Card.CostText->SetColorAndOpacity(FSlateColor(CostColor));
        }
    }

    if (ListHeadingText != nullptr)
    {
        ListHeadingText->SetText(ActiveGroupName);
    }
    if (OfferingCountText != nullptr)
    {
        OfferingCountText->SetText(FText::FromString(FString::Printf(TEXT("%d OFFERINGS"), VisibleCount)));
    }

    const FDungeonBuildableRow* Inspected = ContentSubsystem->FindBuildable(InspectedBuildableId);
    if (Inspected == nullptr)
    {
        if (DetailTitleText != nullptr)
        {
            DetailTitleText->SetText(FText::FromString(TEXT("Nothing selected")));
        }
        if (ActionButton != nullptr)
        {
            ActionButton->SetIsEnabled(false);
        }
        return;
    }

    const FStoreItemStatus Status = ReadStatus(*Inspected, *DungeonSimSubsystem, *ContentSubsystem, EquippedId);
    const FStoreActionView Action = ResolveAction(*Inspected, Status);
    const FLinearColor Accent = Status.bOwned ? DungeonStoreStyle::OwnedGreen() : CategoryAccent(Inspected->Category);

    if (DetailKickerText != nullptr)
    {
        DetailKickerText->SetText(FText::FromString(CategoryKicker(Inspected->Category)));
        DetailKickerText->SetColorAndOpacity(FSlateColor(Accent));
    }
    if (DetailTitleText != nullptr)
    {
        DetailTitleText->SetText(Inspected->DisplayName);
    }
    if (DetailRoleText != nullptr)
    {
        DetailRoleText->SetText(Inspected->RoleText);
    }
    if (DetailStatsText != nullptr)
    {
        TArray<FString> Lines;
        Lines.Add(FootprintLine(*Inspected));
        const FString Income = IncomeLine(*Inspected);
        if (!Income.IsEmpty())
        {
            Lines.Add(Income);
        }
        if (Status.bLocked)
        {
            Lines.Add(FString::Printf(TEXT("Requires  %s"), *Status.RequirementName));
        }
        if (Status.bEquipped)
        {
            Lines.Add(TEXT("Armed on the grid."));
        }
        DetailStatsText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
    }
    if (DetailCostText != nullptr)
    {
        DetailCostText->SetText(FText::FromString(FString::Printf(TEXT("%s  mana"), *FText::AsNumber(Inspected->ManaCost).ToString())));
        DetailCostText->SetColorAndOpacity(FSlateColor(
            Status.bOwned ? DungeonStoreStyle::OwnedGreen()
            : (Status.bAffordable ? DungeonStoreStyle::Gold() : DungeonStoreStyle::Danger())
        ));
    }
    if (ActionButton != nullptr && ActionLabel != nullptr)
    {
        ActionLabel->SetText(FText::FromString(Action.Label));
        ActionButton->SetIsEnabled(Action.bEnabled);
        if (Action.bEnabled)
        {
            ActionButton->SetStyle(DungeonStoreStyle::MakeButtonStyle(
                DungeonStoreStyle::Gold(),
                FLinearColor(1.0f, 0.88f, 0.55f, 1.0f),
                FLinearColor(0.78f, 0.6f, 0.28f, 1.0f),
                DungeonStoreStyle::Gold(),
                12.0f,
                0.0f
            ));
            StyleText(ActionLabel, TEXT("Bold"), 14, 220, DungeonStoreStyle::InkText());
        }
        else
        {
            ActionButton->SetStyle(DungeonStoreStyle::MakeButtonStyle(
                DungeonStoreStyle::CardFill(),
                DungeonStoreStyle::CardFill(),
                DungeonStoreStyle::CardFill(),
                DungeonStoreStyle::GoldDim(),
                12.0f,
                1.0f
            ));
            StyleText(ActionLabel, TEXT("Bold"), 14, 220, DungeonStoreStyle::Muted());
        }
    }
}

UDungeonSimSubsystem* UDungeonStoreWidget::GetDungeonSimSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonSimSubsystem>() : nullptr;
}

UDungeonContentSubsystem* UDungeonStoreWidget::GetDungeonContentSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonContentSubsystem>() : nullptr;
}
