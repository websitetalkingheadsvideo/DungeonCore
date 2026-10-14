#include "UI/DungeonRunSetupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Data/DungeonContentSubsystem.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Player/DungeonPlayerController.h"

void UDungeonRunSetupWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BuildWidget();
    RefreshSelectionText();
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
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleThemeGoblinClicked()
{
    SelectedThemeId = EDungeonThemeId::GoblinTunnels;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleThemeFungalClicked()
{
    SelectedThemeId = EDungeonThemeId::FungalWarren;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleThemeSunkenClicked()
{
    SelectedThemeId = EDungeonThemeId::SunkenRuins;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleFamilyUndeadClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Undead;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleFamilyOrcsClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Orcs;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleFamilySpidersClicked()
{
    SelectedFamilyId = ECreatureFamilyId::Spiders;
    RefreshSelectionText();
}

void UDungeonRunSetupWidget::HandleFamilyFairiesClicked()
{
    SelectedFamilyId = ECreatureFamilyId::EvilFairies;
    RefreshSelectionText();
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
    Backdrop->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.08f, 0.94f));
    UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(Backdrop);
    BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    BackdropSlot->SetOffsets(FMargin(0.0f));

    UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
    UCanvasPanelSlot* RootBoxSlot = RootCanvas->AddChildToCanvas(RootBox);
    RootBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    RootBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    RootBoxSlot->SetAutoSize(true);
    RootBoxSlot->SetZOrder(1);

    UTextBlock* TitleText = CreateTextBlock(TEXT("TitleText"));
    TitleText->SetText(FText::FromString(TEXT("Dungeon Core — Run Setup")));
    FSlateFontInfo TitleFont = TitleText->GetFont();
    TitleFont.Size = 32;
    TitleText->SetFont(TitleFont);
    RootBox->AddChildToVerticalBox(TitleText);

    SelectedThemeTextBlock = CreateTextBlock(TEXT("SelectedThemeText"));
    SelectedFamilyTextBlock = CreateTextBlock(TEXT("SelectedFamilyText"));
    RootBox->AddChildToVerticalBox(SelectedThemeTextBlock);
    RootBox->AddChildToVerticalBox(SelectedFamilyTextBlock);

    UTextBlock* ThemeLabel = CreateTextBlock(TEXT("ThemeLabel"));
    ThemeLabel->SetText(FText::FromString(TEXT("Choose Theme")));
    RootBox->AddChildToVerticalBox(ThemeLabel);

    UHorizontalBox* ThemeButtons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ThemeButtons"));
    RootBox->AddChildToVerticalBox(ThemeButtons);

    UButton* GothicButton = CreateActionButton(GetThemeDisplayText(EDungeonThemeId::Gothic), TEXT("ThemeGothicButton"));
    UButton* GoblinButton = CreateActionButton(GetThemeDisplayText(EDungeonThemeId::GoblinTunnels), TEXT("ThemeGoblinButton"));
    UButton* FungalButton = CreateActionButton(GetThemeDisplayText(EDungeonThemeId::FungalWarren), TEXT("ThemeFungalButton"));
    UButton* SunkenButton = CreateActionButton(GetThemeDisplayText(EDungeonThemeId::SunkenRuins), TEXT("ThemeSunkenButton"));

    ThemeButtons->AddChildToHorizontalBox(GothicButton);
    ThemeButtons->AddChildToHorizontalBox(GoblinButton);
    ThemeButtons->AddChildToHorizontalBox(FungalButton);
    ThemeButtons->AddChildToHorizontalBox(SunkenButton);

    GothicButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeGothicClicked);
    GoblinButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeGoblinClicked);
    FungalButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeFungalClicked);
    SunkenButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleThemeSunkenClicked);

    UTextBlock* FamilyLabel = CreateTextBlock(TEXT("FamilyLabel"));
    FamilyLabel->SetText(FText::FromString(TEXT("Choose Creature Family")));
    RootBox->AddChildToVerticalBox(FamilyLabel);

    UHorizontalBox* FamilyButtons = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FamilyButtons"));
    RootBox->AddChildToVerticalBox(FamilyButtons);

    UButton* UndeadButton = CreateActionButton(GetFamilyDisplayText(ECreatureFamilyId::Undead), TEXT("FamilyUndeadButton"));
    UButton* OrcsButton = CreateActionButton(GetFamilyDisplayText(ECreatureFamilyId::Orcs), TEXT("FamilyOrcsButton"));
    UButton* SpidersButton = CreateActionButton(GetFamilyDisplayText(ECreatureFamilyId::Spiders), TEXT("FamilySpidersButton"));
    UButton* FairiesButton = CreateActionButton(GetFamilyDisplayText(ECreatureFamilyId::EvilFairies), TEXT("FamilyFairiesButton"));

    FamilyButtons->AddChildToHorizontalBox(UndeadButton);
    FamilyButtons->AddChildToHorizontalBox(OrcsButton);
    FamilyButtons->AddChildToHorizontalBox(SpidersButton);
    FamilyButtons->AddChildToHorizontalBox(FairiesButton);

    UndeadButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyUndeadClicked);
    OrcsButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyOrcsClicked);
    SpidersButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilySpidersClicked);
    FairiesButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleFamilyFairiesClicked);

    StartRunButton = CreateActionButton(FText::FromString(TEXT("Start Run")), TEXT("StartRunButton"));
    RootBox->AddChildToVerticalBox(StartRunButton);
    StartRunButton->OnClicked.AddDynamic(this, &UDungeonRunSetupWidget::HandleStartRunClicked);

    bWidgetBuilt = true;
}

void UDungeonRunSetupWidget::RefreshSelectionText()
{
    if (SelectedThemeTextBlock != nullptr)
    {
        SelectedThemeTextBlock->SetText(FText::Format(
            FText::FromString(TEXT("Theme: {0}")),
            GetThemeDisplayText(SelectedThemeId)
        ));
    }

    if (SelectedFamilyTextBlock != nullptr)
    {
        SelectedFamilyTextBlock->SetText(FText::Format(
            FText::FromString(TEXT("Family: {0}")),
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

UButton* UDungeonRunSetupWidget::CreateActionButton(const FText& LabelText, const FName WidgetName)
{
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Label->SetText(LabelText);
    Button->AddChild(Label);
    return Button;
}

UTextBlock* UDungeonRunSetupWidget::CreateTextBlock(const FName WidgetName)
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
    TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    return TextBlock;
}
