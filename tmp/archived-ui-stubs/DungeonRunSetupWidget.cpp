#include "UI/DungeonRunSetupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/DungeonContentSubsystem.h"
#include "Player/DungeonPlayerController.h"
#include "UI/DungeonStoreStyle.h"

namespace DungeonSetupLayout
{
    constexpr float TileWidth = 196.0f;
    constexpr float TileHeight = 60.0f;
    constexpr float TileGap = 12.0f;
    constexpr float TileRadius = 4.0f;
    constexpr float StartButtonHeight = 66.0f;
    constexpr float CardRadius = 8.0f;

    inline FLinearColor Backdrop()
    {
        return FLinearColor(0.012f, 0.008f, 0.024f, 1.0f);
    }

    inline FLinearColor GoldBright()
    {
        return FLinearColor(1.0f, 0.87f, 0.56f, 1.0f);
    }

    inline FLinearColor GoldPressed()
    {
        return FLinearColor(0.78f, 0.6f, 0.3f, 1.0f);
    }

    inline FLinearColor GoldEdge()
    {
        return FLinearColor(0.58f, 0.44f, 0.2f, 1.0f);
    }

    inline FButtonStyle TileStyle(const bool bSelected)
    {
        if (bSelected)
        {
            return DungeonStoreStyle::MakeButtonStyle(
                DungeonStoreStyle::CardSelected(),
                DungeonStoreStyle::CardSelected(),
                DungeonStoreStyle::CardSelected(),
                DungeonStoreStyle::Gold(),
                TileRadius,
                2.0f
            );
        }

        return DungeonStoreStyle::MakeButtonStyle(
            DungeonStoreStyle::CardFill(),
            DungeonStoreStyle::CardHover(),
            DungeonStoreStyle::CardSelected(),
            DungeonStoreStyle::GoldDim(),
            TileRadius,
            1.0f
        );
    }

    inline FButtonStyle StartButtonStyle()
    {
        return DungeonStoreStyle::MakeButtonStyle(
            DungeonStoreStyle::Gold(),
            GoldBright(),
            GoldPressed(),
            GoldEdge(),
            TileRadius,
            1.0f
        );
    }
}

void UDungeonRunSetupWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildWidget();
}

void UDungeonRunSetupWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RefreshSelection();
}

void UDungeonRunSetupWidget::NativeDestruct()
{
    if (StartRunButton != nullptr)
    {
        StartRunButton->OnClicked.RemoveDynamic(this, &UDungeonRunSetupWidget::HandleStartRunClicked);
    }

    Super::NativeDestruct();
}

void UDungeonRunSetupWidget::HandleThemeGothicClicked()
{
    SelectedThemeId = EDungeonThemeId::Gothic;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleThemeGoblinClicked()
{
    SelectedThemeId = EDungeonThemeId::GoblinTunnels;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleThemeFungalClicked()
{
    SelectedThemeId = EDungeonThemeId::FungalWarren;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleThemeSunkenClicked()
{
    SelectedThemeId = EDungeonThemeId::SunkenRuins;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleFamilyUndeadClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Undead;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleFamilyOrcsClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Orcs;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleFamilySpidersClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Spiders;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleFamilyFairiesClicked()
{
    SelectedFamilyId = ECreatureFamilyId::EvilFairies;
    RefreshSelection();
}

void UDungeonRunSetupWidget::HandleStartRunClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->StartRunFromSetup(SelectedThemeId, SelectedFamilyId);
    }
}

void UDungeonRunSetupWidget::BuildWidget()
{
    if (bWidgetBuilt || WidgetTree == nullptr)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
    Backdrop->SetBrushColor(DungeonSetupLayout::Backdrop());
    UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BackdropSlot->SetOffsets(FMargin(0.0f));

    // Central plate: dark card with a gold rule, like a tome page laid on the table.
    UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Card"));
    Card->SetBrush(DungeonStoreStyle::MakeRoundedBrush(
        DungeonStoreStyle::Ink(),
        DungeonStoreStyle::Gold(),
        DungeonSetupLayout::CardRadius,
        2.0f
    ));
    Card->SetPadding(FMargin(64.0f, 48.0f, 64.0f, 52.0f));
    UCanvasPanelSlot* CardSlot = RootCanvas->AddChildToCanvas(Card);
    CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    CardSlot->SetAutoSize(true);
    CardSlot->SetZOrder(1);

    UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Card->AddChild(Column);

    UTextBlock* TitleText = CreateTextBlock(
        TEXT("TitleText"),
        DungeonStoreStyle::FantasyDisplay(),
        52,
        380,
        DungeonStoreStyle::Gold()
    );
    TitleText->SetText(FText::FromString(TEXT("DUNGEON CORE")));
    TitleText->SetJustification(ETextJustify::Center);
    TitleText->SetShadowOffset(FVector2D(0.0f, 3.0f));
    TitleText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
    UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(TitleText);
    TitleSlot->SetHorizontalAlignment(HAlign_Center);

    UTextBlock* SubtitleText = CreateTextBlock(
        TEXT("SubtitleText"),
        DungeonStoreStyle::FantasyRegular(),
        15,
        260,
        DungeonStoreStyle::Muted()
    );
    SubtitleText->SetText(FText::FromString(TEXT("Shape the lair. Choose what dwells within.")));
    SubtitleText->SetJustification(ETextJustify::Center);
    UVerticalBoxSlot* SubtitleSlot = Column->AddChildToVerticalBox(SubtitleText);
    SubtitleSlot->SetHorizontalAlignment(HAlign_Center);
    SubtitleSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));

    UVerticalBoxSlot* TopRuleSlot = Column->AddChildToVerticalBox(CreateDivider(TEXT("TopRule")));
    TopRuleSlot->SetHorizontalAlignment(HAlign_Fill);
    TopRuleSlot->SetPadding(FMargin(0.0f, 26.0f, 0.0f, 28.0f));

    UVerticalBoxSlot* ThemeHeaderSlot = Column->AddChildToVerticalBox(
        CreateSectionHeader(FText::FromString(TEXT("DUNGEON THEME")), TEXT("ThemeHeader"))
    );
    ThemeHeaderSlot->SetHorizontalAlignment(HAlign_Center);

    UHorizontalBox* ThemeRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ThemeRow"));
    UVerticalBoxSlot* ThemeRowSlot = Column->AddChildToVerticalBox(ThemeRow);
    ThemeRowSlot->SetHorizontalAlignment(HAlign_Center);
    ThemeRowSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    const TArray<EDungeonThemeId> ThemeOrder = {
        EDungeonThemeId::Gothic,
        EDungeonThemeId::GoblinTunnels,
        EDungeonThemeId::FungalWarren,
        EDungeonThemeId::SunkenRuins
    };
    for (int32 Index = 0; Index < ThemeOrder.Num(); ++Index)
    {
        const EDungeonThemeId ThemeId = ThemeOrder[Index];
        const FName WidgetName(*FString::Printf(TEXT("ThemeTile_%d"), Index));
        const FDungeonSetupOptionWidgets Option = CreateOptionTile(GetThemeDisplayText(ThemeId), WidgetName);
        ThemeOptions.Add(ThemeId, Option);
        UHorizontalBoxSlot* TileSlot = ThemeRow->AddChildToHorizontalBox(Option.Button);
        TileSlot->SetPadding(FMargin(Index == 0 ? 0.0f : DungeonSetupLayout::TileGap, 0.0f, 0.0f, 0.0f));
    }

    ThemeOptions[EDungeonThemeId::Gothic].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeGothicClicked);
    ThemeOptions[EDungeonThemeId::GoblinTunnels].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeGoblinClicked);
    ThemeOptions[EDungeonThemeId::FungalWarren].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeFungalClicked);
    ThemeOptions[EDungeonThemeId::SunkenRuins].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeSunkenClicked);

    UVerticalBoxSlot* FamilyHeaderSlot = Column->AddChildToVerticalBox(
        CreateSectionHeader(FText::FromString(TEXT("CREATURE FAMILY")), TEXT("FamilyHeader"))
    );
    FamilyHeaderSlot->SetHorizontalAlignment(HAlign_Center);
    FamilyHeaderSlot->SetPadding(FMargin(0.0f, 30.0f, 0.0f, 0.0f));

    UHorizontalBox* FamilyRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FamilyRow"));
    UVerticalBoxSlot* FamilyRowSlot = Column->AddChildToVerticalBox(FamilyRow);
    FamilyRowSlot->SetHorizontalAlignment(HAlign_Center);
    FamilyRowSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    const TArray<ECreatureFamilyId> FamilyOrder = {
        ECreatureFamilyId::Undead,
        ECreatureFamilyId::Orcs,
        ECreatureFamilyId::Spiders,
        ECreatureFamilyId::EvilFairies
    };
    for (int32 Index = 0; Index < FamilyOrder.Num(); ++Index)
    {
        const ECreatureFamilyId FamilyId = FamilyOrder[Index];
        const FName WidgetName(*FString::Printf(TEXT("FamilyTile_%d"), Index));
        const FDungeonSetupOptionWidgets Option = CreateOptionTile(GetFamilyDisplayText(FamilyId), WidgetName);
        FamilyOptions.Add(FamilyId, Option);
        UHorizontalBoxSlot* TileSlot = FamilyRow->AddChildToHorizontalBox(Option.Button);
        TileSlot->SetPadding(FMargin(Index == 0 ? 0.0f : DungeonSetupLayout::TileGap, 0.0f, 0.0f, 0.0f));
    }

    FamilyOptions[ECreatureFamilyId::Undead].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyUndeadClicked);
    FamilyOptions[ECreatureFamilyId::Orcs].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyOrcsClicked);
    FamilyOptions[ECreatureFamilyId::Spiders].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilySpidersClicked);
    FamilyOptions[ECreatureFamilyId::EvilFairies].Button->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyFairiesClicked);

    UVerticalBoxSlot* BottomRuleSlot = Column->AddChildToVerticalBox(CreateDivider(TEXT("BottomRule")));
    BottomRuleSlot->SetHorizontalAlignment(HAlign_Fill);
    BottomRuleSlot->SetPadding(FMargin(0.0f, 32.0f, 0.0f, 22.0f));

    SelectionSummaryTextBlock = CreateTextBlock(
        TEXT("SelectionSummaryText"),
        DungeonStoreStyle::FantasyRegular(),
        16,
        220,
        DungeonStoreStyle::Ivory()
    );
    SelectionSummaryTextBlock->SetJustification(ETextJustify::Center);
    UVerticalBoxSlot* SummarySlot = Column->AddChildToVerticalBox(SelectionSummaryTextBlock);
    SummarySlot->SetHorizontalAlignment(HAlign_Center);

    StartRunButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartRunButton"));
    StartRunButton->SetStyle(DungeonSetupLayout::StartButtonStyle());
    USizeBox* StartSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StartSizeBox"));
    StartSizeBox->SetHeightOverride(DungeonSetupLayout::StartButtonHeight);
    UTextBlock* StartLabel = CreateTextBlock(
        TEXT("StartRunLabel"),
        DungeonStoreStyle::FantasyBold(),
        20,
        460,
        DungeonStoreStyle::InkText()
    );
    StartLabel->SetText(FText::FromString(TEXT("START RUN")));
    StartLabel->SetJustification(ETextJustify::Center);
    USizeBoxSlot* StartLabelSlot = Cast<USizeBoxSlot>(StartSizeBox->AddChild(StartLabel));
    StartLabelSlot->SetHorizontalAlignment(HAlign_Center);
    StartLabelSlot->SetVerticalAlignment(VAlign_Center);
    StartRunButton->AddChild(StartSizeBox);
    UVerticalBoxSlot* StartSlot = Column->AddChildToVerticalBox(StartRunButton);
    StartSlot->SetHorizontalAlignment(HAlign_Fill);
    StartSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 0.0f));
    StartRunButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleStartRunClicked);

    bWidgetBuilt = true;
}

void UDungeonRunSetupWidget::RefreshSelection()
{
    for (const TPair<EDungeonThemeId, FDungeonSetupOptionWidgets>& Entry : ThemeOptions)
    {
        ApplyOptionTileStyle(Entry.Value, Entry.Key == SelectedThemeId);
    }

    for (const TPair<ECreatureFamilyId, FDungeonSetupOptionWidgets>& Entry : FamilyOptions)
    {
        ApplyOptionTileStyle(Entry.Value, Entry.Key == SelectedFamilyId);
    }

    if (SelectionSummaryTextBlock != nullptr)
    {
        SelectionSummaryTextBlock->SetText(FText::Format(
            FText::FromString(TEXT("{0}   \u00B7   {1}")),
            GetThemeDisplayText(SelectedThemeId),
            GetFamilyDisplayText(SelectedFamilyId)
        ));
    }
}

FText UDungeonRunSetupWidget::GetThemeDisplayText(const EDungeonThemeId ThemeId) const
{
    if (const UGameInstance* GameInstance = GetGameInstance())
    {
        if (const UDungeonContentSubsystem* ContentSubsystem = GameInstance->GetSubsystem<UDungeonContentSubsystem>())
        {
            if (const FDungeonThemeRow* ThemeRow = ContentSubsystem->FindThemeRow(ThemeId))
            {
                return ThemeRow->DisplayName;
            }
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Theme row lookup failed for setup widget."));
    return FText::GetEmpty();
}

FText UDungeonRunSetupWidget::GetFamilyDisplayText(const ECreatureFamilyId FamilyId) const
{
    if (const UGameInstance* GameInstance = GetGameInstance())
    {
        if (const UDungeonContentSubsystem* ContentSubsystem = GameInstance->GetSubsystem<UDungeonContentSubsystem>())
        {
            if (const FDungeonCreatureFamilyRow* FamilyRow = ContentSubsystem->FindFamilyRow(FamilyId))
            {
                return FamilyRow->DisplayName;
            }
        }
    }

    UE_LOG(LogTemp, Fatal, TEXT("Family row lookup failed for setup widget."));
    return FText::GetEmpty();
}

ADungeonPlayerController* UDungeonRunSetupWidget::GetDungeonPlayerController() const
{
    return GetOwningPlayer<ADungeonPlayerController>();
}

FDungeonSetupOptionWidgets UDungeonRunSetupWidget::CreateOptionTile(const FText& LabelText, const FName WidgetName)
{
    FDungeonSetupOptionWidgets Option;
    Option.Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);

    USizeBox* TileSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    TileSizeBox->SetWidthOverride(DungeonSetupLayout::TileWidth);
    TileSizeBox->SetHeightOverride(DungeonSetupLayout::TileHeight);

    Option.Label = CreateTextBlock(
        NAME_None,
        DungeonStoreStyle::FantasyBold(),
        15,
        140,
        DungeonStoreStyle::Ivory()
    );
    Option.Label->SetText(LabelText);
    Option.Label->SetJustification(ETextJustify::Center);

    USizeBoxSlot* LabelSlot = Cast<USizeBoxSlot>(TileSizeBox->AddChild(Option.Label));
    LabelSlot->SetHorizontalAlignment(HAlign_Center);
    LabelSlot->SetVerticalAlignment(VAlign_Center);
    Option.Button->AddChild(TileSizeBox);

    ApplyOptionTileStyle(Option, false);
    return Option;
}

void UDungeonRunSetupWidget::ApplyOptionTileStyle(const FDungeonSetupOptionWidgets& Option, const bool bSelected)
{
    if (Option.Button == nullptr || Option.Label == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Setup option tile is missing its button or label."));
        return;
    }

    Option.Button->SetStyle(DungeonSetupLayout::TileStyle(bSelected));
    Option.Label->SetColorAndOpacity(FSlateColor(bSelected ? DungeonStoreStyle::Gold() : DungeonStoreStyle::Ivory()));
}

UTextBlock* UDungeonRunSetupWidget::CreateTextBlock(
    const FName WidgetName,
    const FName Typeface,
    const int32 Size,
    const int32 LetterSpacing,
    const FLinearColor& Color
)
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
    TextBlock->SetFont(DungeonStoreStyle::MakeFantasyFont(Typeface, Size, LetterSpacing));
    TextBlock->SetColorAndOpacity(FSlateColor(Color));
    return TextBlock;
}

UTextBlock* UDungeonRunSetupWidget::CreateSectionHeader(const FText& HeaderText, const FName WidgetName)
{
    UTextBlock* Header = CreateTextBlock(WidgetName, DungeonStoreStyle::FantasyBold(), 13, 480, DungeonStoreStyle::Gold());
    Header->SetText(HeaderText);
    Header->SetJustification(ETextJustify::Center);
    return Header;
}

UWidget* UDungeonRunSetupWidget::CreateDivider(const FName WidgetName)
{
    USizeBox* RuleBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), WidgetName);
    RuleBox->SetHeightOverride(1.5f);
    UBorder* Rule = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    Rule->SetBrushColor(DungeonStoreStyle::GoldDim());
    RuleBox->AddChild(Rule);
    return RuleBox;
}
