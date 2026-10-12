#include "UI/DungeonHudWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Player/DungeonPlayerController.h"
#include "Sim/DungeonSimSubsystem.h"

void UDungeonHudWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildHud();

    ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (DungeonPlayerController != nullptr)
    {
        DungeonPlayerController->OnBuildModeChanged.AddDynamic(this, &UDungeonHudWidget::HandleBuildModeChanged);
        DungeonPlayerController->OnPlacementPreviewChanged.AddDynamic(this, &UDungeonHudWidget::HandlePlacementPreviewChanged);
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UDungeonSimSubsystem>()
        : nullptr)
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
        DungeonPlayerController->OnBuildModeChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleBuildModeChanged);
        DungeonPlayerController->OnPlacementPreviewChanged.RemoveDynamic(this, &UDungeonHudWidget::HandlePlacementPreviewChanged);
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UDungeonSimSubsystem>()
        : nullptr)
    {
        DungeonSimSubsystem->OnResourceChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleResourceChanged);
        DungeonSimSubsystem->OnBreachStateChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleBreachStateChanged);
        DungeonSimSubsystem->OnWaveChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleWaveChanged);
        DungeonSimSubsystem->OnCoreHealthChanged.RemoveDynamic(this, &UDungeonHudWidget::HandleCoreHealthChanged);
    }

    if (DigButton != nullptr)
    {
        DigButton->OnClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleDigClicked);
    }

    if (ManaWellButton != nullptr)
    {
        ManaWellButton->OnClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleManaWellClicked);
    }

    if (StarterSpawnerButton != nullptr)
    {
        StarterSpawnerButton->OnClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleStarterSpawnerClicked);
    }

    if (StarterTrapButton != nullptr)
    {
        StarterTrapButton->OnClicked.RemoveDynamic(this, &UDungeonHudWidget::HandleStarterTrapClicked);
    }

    Super::NativeDestruct();
}

void UDungeonHudWidget::HandleResourceChanged(const FResourceState ResourceState)
{
    RefreshManaText(ResourceState);
}

void UDungeonHudWidget::HandleBreachStateChanged(const EBreachState BreachState)
{
    RefreshBreachText(BreachState);
}

void UDungeonHudWidget::HandleBuildModeChanged(const EDungeonBuildMode BuildMode)
{
    RefreshModeText(BuildMode);
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

void UDungeonHudWidget::HandleDigClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->SetBuildModeFromUI(EDungeonBuildMode::Dig);
    }
}

void UDungeonHudWidget::HandleManaWellClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->SetBuildModeFromUI(EDungeonBuildMode::ManaWell);
    }
}

void UDungeonHudWidget::HandleStarterSpawnerClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->SetBuildModeFromUI(EDungeonBuildMode::StarterSpawner);
    }
}

void UDungeonHudWidget::HandleStarterTrapClicked()
{
    if (ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController())
    {
        DungeonPlayerController->SetBuildModeFromUI(EDungeonBuildMode::StarterTrap);
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
    BuildModeTextBlock = CreateInfoTextBlock(TEXT("BuildModeText"));
    WaveTextBlock = CreateInfoTextBlock(TEXT("WaveText"));
    CoreHealthTextBlock = CreateInfoTextBlock(TEXT("CoreHealthText"));
    PlacementPreviewTextBlock = CreateInfoTextBlock(TEXT("PlacementPreviewText"));
    ControlsTextBlock = CreateInfoTextBlock(TEXT("ControlsText"));
    ControlsTextBlock->SetText(FText::FromString(TEXT("1-4 Build | Left Click Act | WASD/Edge Pan | Wheel Zoom | R Restart | F5 Save | F9 Load")));

    InfoBox->AddChildToVerticalBox(ManaTextBlock);
    InfoBox->AddChildToVerticalBox(BreachTextBlock);
    InfoBox->AddChildToVerticalBox(BuildModeTextBlock);
    InfoBox->AddChildToVerticalBox(WaveTextBlock);
    InfoBox->AddChildToVerticalBox(CoreHealthTextBlock);
    InfoBox->AddChildToVerticalBox(PlacementPreviewTextBlock);
    InfoBox->AddChildToVerticalBox(ControlsTextBlock);

    UHorizontalBox* ActionBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ActionBox"));
    UCanvasPanelSlot* ActionBoxSlot = RootCanvas->AddChildToCanvas(ActionBox);
    ActionBoxSlot->SetAnchors(FAnchors(0.0f, 1.0f));
    ActionBoxSlot->SetPosition(FVector2D(24.0f, -60.0f));
    ActionBoxSlot->SetAutoSize(true);

    DigButton = CreateActionButton(FText::FromString(TEXT("Dig")), TEXT("DigButton"));
    ManaWellButton = CreateActionButton(FText::FromString(TEXT("Mana Well")), TEXT("ManaWellButton"));
    StarterSpawnerButton = CreateActionButton(FText::FromString(TEXT("Spawner")), TEXT("StarterSpawnerButton"));
    StarterTrapButton = CreateActionButton(FText::FromString(TEXT("Trap")), TEXT("StarterTrapButton"));

    ActionBox->AddChildToHorizontalBox(DigButton);
    ActionBox->AddChildToHorizontalBox(ManaWellButton);
    ActionBox->AddChildToHorizontalBox(StarterSpawnerButton);
    ActionBox->AddChildToHorizontalBox(StarterTrapButton);

    DigButton->OnClicked.AddDynamic(this, &UDungeonHudWidget::HandleDigClicked);
    ManaWellButton->OnClicked.AddDynamic(this, &UDungeonHudWidget::HandleManaWellClicked);
    StarterSpawnerButton->OnClicked.AddDynamic(this, &UDungeonHudWidget::HandleStarterSpawnerClicked);
    StarterTrapButton->OnClicked.AddDynamic(this, &UDungeonHudWidget::HandleStarterTrapClicked);

    bHudBuilt = true;
}

void UDungeonHudWidget::RefreshAllText()
{
    ADungeonPlayerController* DungeonPlayerController = GetDungeonPlayerController();
    if (DungeonPlayerController != nullptr)
    {
        RefreshModeText(DungeonPlayerController->GetBuildMode());
        RefreshPlacementPreviewText(DungeonPlayerController->GetCurrentPlacementPreview());
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UDungeonSimSubsystem>()
        : nullptr)
    {
        RefreshManaText(DungeonSimSubsystem->GetResourceState());
        RefreshBreachText(DungeonSimSubsystem->GetFloorState().BreachState);
        RefreshWaveText(DungeonSimSubsystem->GetCurrentWaveNumber());
        RefreshCoreHealthText(DungeonSimSubsystem->GetCurrentCoreHealth());
    }
}

void UDungeonHudWidget::RefreshManaText(const FResourceState& ResourceState)
{
    if (ManaTextBlock == nullptr)
    {
        return;
    }

    ManaTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Mana: %d"), ResourceState.CurrentMana)));
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

void UDungeonHudWidget::RefreshModeText(const EDungeonBuildMode BuildMode)
{
    if (BuildModeTextBlock == nullptr)
    {
        return;
    }

    FString ModeLabel;

    switch (BuildMode)
    {
    case EDungeonBuildMode::Dig:
        ModeLabel = TEXT("Dig");
        break;
    case EDungeonBuildMode::ManaWell:
        ModeLabel = TEXT("Mana Well");
        break;
    case EDungeonBuildMode::StarterSpawner:
        ModeLabel = TEXT("Starter Spawner");
        break;
    case EDungeonBuildMode::StarterTrap:
        ModeLabel = TEXT("Starter Trap");
        break;
    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled build mode while refreshing HUD."));
        break;
    }

    BuildModeTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Mode: %s"), *ModeLabel)));
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

    FString FailureLabel;

    switch (PlacementResult.FailureReason)
    {
    case EPlacementFailureReason::None:
        FailureLabel = TEXT("Waiting for target");
        break;
    case EPlacementFailureReason::OutOfBounds:
        FailureLabel = TEXT("Out of bounds");
        break;
    case EPlacementFailureReason::Blocked:
        FailureLabel = TEXT("Blocked");
        break;
    case EPlacementFailureReason::InsufficientMana:
        FailureLabel = TEXT("Not enough mana");
        break;
    case EPlacementFailureReason::RequiresClearedTile:
        FailureLabel = TEXT("Needs cleared tile");
        break;
    case EPlacementFailureReason::WouldDisconnectPath:
        FailureLabel = TEXT("Must dig from reachable space");
        break;
    case EPlacementFailureReason::InvalidFootprint:
        FailureLabel = TEXT("Invalid footprint");
        break;
    case EPlacementFailureReason::FamilyLocked:
        FailureLabel = TEXT("Family locked");
        break;
    case EPlacementFailureReason::ThemeLocked:
        FailureLabel = TEXT("Theme locked");
        break;
    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled placement failure reason in HUD."));
        return;
    }

    PlacementPreviewTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Placement: %s"), *FailureLabel)));
}

ADungeonPlayerController* UDungeonHudWidget::GetDungeonPlayerController() const
{
    return GetOwningPlayer<ADungeonPlayerController>();
}

UButton* UDungeonHudWidget::CreateActionButton(const FText& LabelText, const FName WidgetName)
{
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), WidgetName);
    UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Label->SetText(LabelText);
    Button->AddChild(Label);
    return Button;
}

UTextBlock* UDungeonHudWidget::CreateInfoTextBlock(const FName WidgetName)
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
    TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    return TextBlock;
}
