#include "UI/DungeonHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "TimerManager.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/DungeonContentSubsystem.h"
#include "Player/DungeonPlayerController.h"
#include "Sim/DungeonSimSubsystem.h"
#include "UI/DungeonStoreStyle.h"
#include "UI/DungeonStoreWidget.h"

namespace
{
    void StyleHudText(UTextBlock* Text, const FName Typeface, const int32 Size, const int32 LetterSpacing, const FLinearColor& Color)
    {
        Text->SetFont(DungeonStoreStyle::MakeFont(Typeface, Size, LetterSpacing));
        Text->SetColorAndOpacity(FSlateColor(Color));
        Text->SetShadowOffset(FVector2D(0.0f, 1.0f));
        Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f));
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

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(OutOfManaNoticeTimer);
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        DungeonSimSubsystem->OnResourceChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleResourceChanged);
        DungeonSimSubsystem->OnBreachStateChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleBreachStateChanged);
        DungeonSimSubsystem->OnWaveChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleWaveChanged);
        DungeonSimSubsystem->OnCoreHealthChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleCoreHealthChanged);
    }

    if (VaultButton != nullptr)
    {
        VaultButton->OnClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleVaultClicked);
    }

    Super::NativeDestruct();
}

void UDungeonHudWidget::SetStoreVisible(const bool bVisible)
{
    if (StoreWidget == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Core Vault widget is missing."));
        return;
    }

    StoreWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (bVisible)
    {
        StoreWidget->RefreshStore();
    }
}

void UDungeonHudWidget::HandleResourceChanged(const FResourceState ResourceState)
{
    RefreshManaText(ResourceState);
    RefreshStore();
}

void UDungeonHudWidget::HandleBreachStateChanged(const EBreachState BreachState)
{
    RefreshBreachText(BreachState);
}

void UDungeonHudWidget::HandleSelectedBuildableChanged(const FName BuildableId)
{
    RefreshSelectedText(BuildableId);
    RefreshStore();
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

void UDungeonHudWidget::HandleVaultClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->SetDungeonStoreOpen(true);
    }
}

void UDungeonHudWidget::BuildHud()
{
    if (bHudBuilt || WidgetTree == nullptr)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    WidgetTree->RootWidget = RootCanvas;

    UBorder* StatusPlate = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StatusPlate"));
    StatusPlate->SetPadding(FMargin(18.0f, 14.0f, 20.0f, 14.0f));
    StatusPlate->SetBrush(DungeonStoreStyle::MakeRoundedBrush(
        DungeonStoreStyle::InkRaised(),
        DungeonStoreStyle::GoldDim(),
        16.0f,
        1.0f
    ));

    UVerticalBox* InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
    StatusPlate->AddChild(InfoBox);

    UTextBlock* ManaKicker = CreateInfoTextBlock(TEXT("ManaKicker"));
    ManaKicker->SetText(FText::FromString(TEXT("MANA")));
    StyleHudText(ManaKicker, TEXT("Bold"), 11, 280, DungeonStoreStyle::Gold());

    ManaTextBlock = CreateInfoTextBlock(TEXT("ManaText"));
    StyleHudText(ManaTextBlock, TEXT("Bold"), 32, 0, DungeonStoreStyle::Gold());

    IncomeTextBlock = CreateInfoTextBlock(TEXT("IncomeText"));
    StyleHudText(IncomeTextBlock, TEXT("Regular"), 14, 30, DungeonStoreStyle::Violet());

    BreachTextBlock = CreateInfoTextBlock(TEXT("BreachText"));
    SelectedTextBlock = CreateInfoTextBlock(TEXT("SelectedText"));
    WaveTextBlock = CreateInfoTextBlock(TEXT("WaveText"));
    CoreHealthTextBlock = CreateInfoTextBlock(TEXT("CoreHealthText"));
    PlacementPreviewTextBlock = CreateInfoTextBlock(TEXT("PlacementPreviewText"));
    StyleHudText(BreachTextBlock, TEXT("Regular"), 14, 10, DungeonStoreStyle::Ivory());
    StyleHudText(SelectedTextBlock, TEXT("Regular"), 14, 10, DungeonStoreStyle::Ivory());
    StyleHudText(WaveTextBlock, TEXT("Regular"), 14, 10, DungeonStoreStyle::Ivory());
    StyleHudText(CoreHealthTextBlock, TEXT("Regular"), 14, 10, DungeonStoreStyle::Ivory());
    StyleHudText(PlacementPreviewTextBlock, TEXT("Regular"), 14, 10, DungeonStoreStyle::Muted());

    ControlsTextBlock = CreateInfoTextBlock(TEXT("ControlsText"));
    ControlsTextBlock->SetAutoWrapText(true);
    ControlsTextBlock->SetText(FText::FromString(TEXT("B  Vault     1-9  Tool     Click  Act     Drag  Pan     RMB  Orbit     Wheel  Zoom     R  Restart     F5 Save     F9 Load")));
    StyleHudText(ControlsTextBlock, TEXT("Regular"), 12, 10, DungeonStoreStyle::Muted());

    InfoBox->AddChildToVerticalBox(ManaKicker);
    InfoBox->AddChildToVerticalBox(ManaTextBlock);
    InfoBox->AddChildToVerticalBox(IncomeTextBlock);
    UVerticalBoxSlot* BreachSlot = InfoBox->AddChildToVerticalBox(BreachTextBlock);
    BreachSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
    InfoBox->AddChildToVerticalBox(SelectedTextBlock);
    InfoBox->AddChildToVerticalBox(WaveTextBlock);
    InfoBox->AddChildToVerticalBox(CoreHealthTextBlock);
    InfoBox->AddChildToVerticalBox(PlacementPreviewTextBlock);
    UVerticalBoxSlot* ControlsSlot = InfoBox->AddChildToVerticalBox(ControlsTextBlock);
    ControlsSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));

    USizeBox* PlateSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StatusPlateSize"));
    PlateSize->SetWidthOverride(460.0f);
    PlateSize->AddChild(StatusPlate);
    UCanvasPanelSlot* PlateSlot = RootCanvas->AddChildToCanvas(PlateSize);
    PlateSlot->SetAnchors(FAnchors(0.0f, 0.0f));
    PlateSlot->SetPosition(FVector2D(28.0f, 28.0f));
    PlateSlot->SetAutoSize(true);

    VaultButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("VaultButton"));
    VaultButton->SetStyle(DungeonStoreStyle::MakeButtonStyle(
        DungeonStoreStyle::Ink(),
        DungeonStoreStyle::CardHover(),
        DungeonStoreStyle::CardSelected(),
        DungeonStoreStyle::Gold(),
        12.0f,
        1.25f
    ));
    VaultButton->OnClicked.AddDynamic(this, &UDungeonHudWidget::HandleVaultClicked);
    UTextBlock* VaultLabel = CreateInfoTextBlock(TEXT("VaultLabel"));
    VaultLabel->SetText(FText::FromString(TEXT("VAULT    B")));
    VaultLabel->SetJustification(ETextJustify::Center);
    VaultLabel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    StyleHudText(VaultLabel, TEXT("Bold"), 14, 280, DungeonStoreStyle::Gold());
    VaultButton->AddChild(VaultLabel);
    USizeBox* VaultSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("VaultButtonSize"));
    VaultSize->SetWidthOverride(168.0f);
    VaultSize->SetHeightOverride(48.0f);
    VaultSize->AddChild(VaultButton);
    UCanvasPanelSlot* VaultSlot = RootCanvas->AddChildToCanvas(VaultSize);
    VaultSlot->SetAnchors(FAnchors(1.0f, 0.0f));
    VaultSlot->SetAlignment(FVector2D(1.0f, 0.0f));
    VaultSlot->SetPosition(FVector2D(-28.0f, 28.0f));
    VaultSlot->SetAutoSize(true);

    StoreWidget = WidgetTree->ConstructWidget<UDungeonStoreWidget>(UDungeonStoreWidget::StaticClass(), TEXT("Store"));
    UCanvasPanelSlot* StoreSlot = RootCanvas->AddChildToCanvas(StoreWidget);
    StoreSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    StoreSlot->SetOffsets(FMargin(0.0f));
    StoreSlot->SetZOrder(10);
    StoreWidget->SetVisibility(ESlateVisibility::Collapsed);

    OutOfManaNotice = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OutOfManaNotice"));
    OutOfManaNotice->SetPadding(FMargin(22.0f, 12.0f));
    OutOfManaNotice->SetBrush(DungeonStoreStyle::MakeRoundedBrush(
        DungeonStoreStyle::Ink(),
        DungeonStoreStyle::Danger(),
        14.0f,
        1.5f
    ));
    OutOfManaNotice->SetVisibility(ESlateVisibility::Collapsed);
    OutOfManaText = CreateInfoTextBlock(TEXT("OutOfManaText"));
    OutOfManaText->SetJustification(ETextJustify::Center);
    OutOfManaText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    StyleHudText(OutOfManaText, TEXT("Bold"), 16, 180, DungeonStoreStyle::Danger());
    OutOfManaNotice->AddChild(OutOfManaText);
    UCanvasPanelSlot* NoticeSlot = RootCanvas->AddChildToCanvas(OutOfManaNotice);
    NoticeSlot->SetAnchors(FAnchors(0.5f, 0.0f));
    NoticeSlot->SetAlignment(FVector2D(0.5f, 0.0f));
    NoticeSlot->SetPosition(FVector2D(0.0f, 28.0f));
    NoticeSlot->SetAutoSize(true);
    NoticeSlot->SetZOrder(5);

    bHudBuilt = true;
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

    RefreshStore();
}

void UDungeonHudWidget::ShowOutOfManaNotice(const int32 ManaCost)
{
    if (OutOfManaNotice == nullptr || OutOfManaText == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Out-of-mana notice is missing from the HUD."));
        return;
    }

    const FString Label = ManaCost > 0
        ? FString::Printf(TEXT("NOT ENOUGH MANA     NEED %d"), ManaCost)
        : FString(TEXT("NOT ENOUGH MANA"));
    OutOfManaText->SetText(FText::FromString(Label));
    OutOfManaNotice->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    bOutOfManaNoticeVisible = true;
    if (ManaTextBlock != nullptr)
    {
        ManaTextBlock->SetColorAndOpacity(FSlateColor(DungeonStoreStyle::Danger()));
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            OutOfManaNoticeTimer,
            this,
            &UDungeonHudWidget::HandleOutOfManaNoticeExpired,
            2.5f,
            false
        );
    }
}

void UDungeonHudWidget::HideOutOfManaNotice()
{
    bOutOfManaNoticeVisible = false;
    if (OutOfManaNotice != nullptr)
    {
        OutOfManaNotice->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (ManaTextBlock != nullptr)
    {
        ManaTextBlock->SetColorAndOpacity(FSlateColor(DungeonStoreStyle::Gold()));
    }
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(OutOfManaNoticeTimer);
    }
}

void UDungeonHudWidget::HandleOutOfManaNoticeExpired()
{
    HideOutOfManaNotice();
}

void UDungeonHudWidget::RefreshManaText(const FResourceState& ResourceState)
{
    if (ManaTextBlock != nullptr)
    {
        ManaTextBlock->SetText(FText::AsNumber(FMath::Max(0, ResourceState.CurrentMana)));
        ManaTextBlock->SetColorAndOpacity(FSlateColor(
            bOutOfManaNoticeVisible ? DungeonStoreStyle::Danger() : DungeonStoreStyle::Gold()
        ));
    }
    if (IncomeTextBlock != nullptr)
    {
        IncomeTextBlock->SetText(FText::FromString(FString::Printf(TEXT("+%.1f / sec"), ResourceState.ManaIncomePerSecond)));
    }
}

void UDungeonHudWidget::RefreshBreachText(const EBreachState BreachState)
{
    if (BreachTextBlock == nullptr)
    {
        return;
    }

    const FString BreachLabel = BreachState == EBreachState::Open ? TEXT("Open") : TEXT("Sealed");
    BreachTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Breach   %s"), *BreachLabel)));
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
    SelectedTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Tool   %s"), *SelectedLabel)));
}

void UDungeonHudWidget::RefreshWaveText(const int32 CurrentWaveNumber)
{
    if (WaveTextBlock == nullptr)
    {
        return;
    }

    WaveTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Wave   %d"), CurrentWaveNumber)));
}

void UDungeonHudWidget::RefreshCoreHealthText(const int32 CurrentCoreHealth)
{
    if (CoreHealthTextBlock == nullptr)
    {
        return;
    }

    CoreHealthTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Core   %d"), CurrentCoreHealth)));
}

void UDungeonHudWidget::RefreshPlacementPreviewText(const FPlacementResult& PlacementResult)
{
    if (PlacementPreviewTextBlock == nullptr)
    {
        return;
    }

    const bool bOutOfMana = !PlacementResult.bSuccess
        && PlacementResult.FailureReason == EPlacementFailureReason::InsufficientMana;
    PlacementPreviewTextBlock->SetColorAndOpacity(FSlateColor(
        bOutOfMana ? DungeonStoreStyle::Danger() : DungeonStoreStyle::Muted()
    ));

    if (PlacementResult.bSuccess)
    {
        PlacementPreviewTextBlock->SetText(FText::FromString(TEXT("Placement   Clear")));
        return;
    }

    PlacementPreviewTextBlock->SetText(FText::FromString(FString::Printf(
        TEXT("Placement   %s"),
        *DescribePlacementFailure(PlacementResult.FailureReason)
    )));
}

void UDungeonHudWidget::RefreshStore()
{
    if (StoreWidget != nullptr && StoreWidget->IsVisible())
    {
        StoreWidget->RefreshStore();
    }
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
