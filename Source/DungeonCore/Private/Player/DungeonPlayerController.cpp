#include "Player/DungeonPlayerController.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Player/DungeonCameraPawn.h"
#include "Sim/DungeonSimSubsystem.h"
#include "UI/DungeonHudWidget.h"
#include "UI/DungeonRunSetupWidget.h"
#include "World/DungeonBuildCursorActor.h"
#include "World/DungeonChunkActor.h"

ADungeonPlayerController::ADungeonPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ADungeonPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Dungeon sim subsystem is required for the player controller."));
    }

    DungeonSimSubsystem->OnTileChanged.AddDynamic(this, &ADungeonPlayerController::HandleTileChanged);
    DungeonSimSubsystem->OnBreachStateChanged.AddDynamic(this, &ADungeonPlayerController::HandleBreachStateChanged);
    DungeonSimSubsystem->OnAdventurersChanged.AddDynamic(this, &ADungeonPlayerController::HandleAdventurersChanged);

    EnsureBuildCursor();
    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetActorHiddenInGame(true);
    }

    SetupWidget = CreateWidget<UDungeonRunSetupWidget>(this, UDungeonRunSetupWidget::StaticClass());
    if (SetupWidget != nullptr)
    {
        SetupWidget->AddToViewport();
    }
}

void ADungeonPlayerController::PlayerTick(const float DeltaSeconds)
{
    Super::PlayerTick(DeltaSeconds);
    UpdateCameraPanFromMouse();
    UpdateCursorFromMouse();
}

void ADungeonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    check(InputComponent != nullptr);

    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ADungeonPlayerController::HandlePrimaryAction);
    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ADungeonPlayerController::SetDigMode);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ADungeonPlayerController::SetManaWellMode);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADungeonPlayerController::SetStarterSpawnerMode);
    InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ADungeonPlayerController::SetStarterTrapMode);
    InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ADungeonPlayerController::PanCameraNorth);
    InputComponent->BindKey(EKeys::W, IE_Repeat, this, &ADungeonPlayerController::PanCameraNorth);
    InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ADungeonPlayerController::PanCameraSouth);
    InputComponent->BindKey(EKeys::S, IE_Repeat, this, &ADungeonPlayerController::PanCameraSouth);
    InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ADungeonPlayerController::PanCameraEast);
    InputComponent->BindKey(EKeys::D, IE_Repeat, this, &ADungeonPlayerController::PanCameraEast);
    InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ADungeonPlayerController::PanCameraWest);
    InputComponent->BindKey(EKeys::A, IE_Repeat, this, &ADungeonPlayerController::PanCameraWest);
    InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ADungeonPlayerController::ZoomIn);
    InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ADungeonPlayerController::ZoomOut);
    InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ADungeonPlayerController::RestartRun);
    InputComponent->BindKey(EKeys::F5, IE_Pressed, this, &ADungeonPlayerController::QuickSaveRun);
    InputComponent->BindKey(EKeys::F9, IE_Pressed, this, &ADungeonPlayerController::QuickLoadRun);
}

void ADungeonPlayerController::SetBuildModeFromUI(const EDungeonBuildMode InBuildMode)
{
    SetBuildMode(InBuildMode);
}

EDungeonBuildMode ADungeonPlayerController::GetBuildMode() const
{
    return BuildMode;
}

FPlacementResult ADungeonPlayerController::GetCurrentPlacementPreview() const
{
    return CurrentPlacementPreview;
}

void ADungeonPlayerController::StartRunFromSetup(const EDungeonThemeId ThemeId, const ECreatureFamilyId CreatureFamilyId)
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        return;
    }

    FRunSetup RunSetup;
    RunSetup.ThemeId = ThemeId;
    RunSetup.CreatureFamilyId = CreatureFamilyId;
    RunSetup.Seed = 1;
    RunSetup.StartingFloorIndex = 1;
    DungeonSimSubsystem->StartNewRun(RunSetup);

    if (SetupWidget != nullptr)
    {
        SetupWidget->RemoveFromParent();
        SetupWidget = nullptr;
    }

    if (HudWidget == nullptr)
    {
        HudWidget = CreateWidget<UDungeonHudWidget>(this, UDungeonHudWidget::StaticClass());
    }

    if (HudWidget != nullptr && !HudWidget->IsInViewport())
    {
        HudWidget->AddToViewport();
    }

    bRunActive = true;
    RefreshChunkVisuals();
    SetBuildMode(EDungeonBuildMode::Dig);
    UpdatePlacementPreview();
}

bool ADungeonPlayerController::IsRunActive() const
{
    return bRunActive;
}

void ADungeonPlayerController::HandleTileChanged(const FTileData TileData)
{
    RefreshChunkVisuals();
}

void ADungeonPlayerController::HandleBreachStateChanged(const EBreachState BreachState)
{
    RefreshChunkVisuals();
}

void ADungeonPlayerController::HandleAdventurersChanged()
{
    RefreshChunkVisuals();
}

void ADungeonPlayerController::HandlePrimaryAction()
{
    if (!bHasHoveredBlock)
    {
        return;
    }

    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        return;
    }

    switch (BuildMode)
    {
    case EDungeonBuildMode::Dig:
        DungeonSimSubsystem->DigBlock(HoveredBlockCoord);
        break;

    case EDungeonBuildMode::ManaWell:
    {
        FRoomFootprint Footprint;
        Footprint.Width = 1;
        Footprint.Height = 1;
        Footprint.Depth = 1;
        DungeonSimSubsystem->PlaceRoom(HoveredBlockCoord, ERoomTypeId::ManaWell, Footprint, DungeonSimSubsystem->GetManaWellCost());
        break;
    }

    case EDungeonBuildMode::StarterSpawner:
    {
        FRoomFootprint Footprint;
        Footprint.Width = 2;
        Footprint.Height = 2;
        Footprint.Depth = 1;
        DungeonSimSubsystem->PlaceRoom(
            HoveredBlockCoord,
            DungeonSimSubsystem->GetActiveStarterSpawnerRoomType(),
            Footprint,
            DungeonSimSubsystem->GetStarterSpawnerCost()
        );
        break;
    }

    case EDungeonBuildMode::StarterTrap:
        DungeonSimSubsystem->PlaceTrap(
            HoveredBlockCoord,
            DungeonSimSubsystem->GetActiveStarterTrapType(),
            DungeonSimSubsystem->GetStarterTrapCost()
        );
        break;

    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled build mode."));
        break;
    }
}

void ADungeonPlayerController::SetDigMode()
{
    SetBuildMode(EDungeonBuildMode::Dig);
}

void ADungeonPlayerController::SetManaWellMode()
{
    SetBuildMode(EDungeonBuildMode::ManaWell);
}

void ADungeonPlayerController::SetStarterSpawnerMode()
{
    SetBuildMode(EDungeonBuildMode::StarterSpawner);
}

void ADungeonPlayerController::SetStarterTrapMode()
{
    SetBuildMode(EDungeonBuildMode::StarterTrap);
}

void ADungeonPlayerController::SetBuildMode(const EDungeonBuildMode InBuildMode)
{
    BuildMode = InBuildMode;
    OnBuildModeChanged.Broadcast(BuildMode);

    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetPreviewMode(BuildMode);
        BuildCursorActor->SetPreviewOriginBlock(HoveredBlockCoord);
    }

    UpdatePlacementPreview();
}

void ADungeonPlayerController::UpdateCursorFromMouse()
{
    if (BuildCursorActor == nullptr || !bRunActive)
    {
        if (BuildCursorActor != nullptr)
        {
            BuildCursorActor->SetActorHiddenInGame(true);
        }
        return;
    }

    FHitResult HitResult;
    const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    if (!bHit)
    {
        bHasHoveredBlock = false;
        BuildCursorActor->SetActorHiddenInGame(true);
        UpdatePlacementPreview();
        return;
    }

    HoveredBlockCoord = DungeonCore::WorldToBlock(HitResult.ImpactPoint);
    HoveredBlockCoord.Z = 0;
    bHasHoveredBlock = true;

    BuildCursorActor->SetActorHiddenInGame(false);
    BuildCursorActor->SetPreviewOriginBlock(HoveredBlockCoord);
    UpdatePlacementPreview();
}

void ADungeonPlayerController::UpdatePlacementPreview()
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr || !bHasHoveredBlock)
    {
        CurrentPlacementPreview = FPlacementResult();
        CurrentPlacementPreview.FailureReason = EPlacementFailureReason::OutOfBounds;
        OnPlacementPreviewChanged.Broadcast(CurrentPlacementPreview);
        if (BuildCursorActor != nullptr)
        {
            BuildCursorActor->SetPreviewValidity(false);
        }
        return;
    }

    switch (BuildMode)
    {
    case EDungeonBuildMode::Dig:
        CurrentPlacementPreview = DungeonSimSubsystem->PreviewDigAtBlock(HoveredBlockCoord);
        break;

    case EDungeonBuildMode::ManaWell:
    {
        FRoomFootprint Footprint;
        Footprint.Width = 1;
        Footprint.Height = 1;
        Footprint.Depth = 1;
        CurrentPlacementPreview = DungeonSimSubsystem->PreviewRoomPlacement(HoveredBlockCoord, Footprint, DungeonSimSubsystem->GetManaWellCost());
        break;
    }

    case EDungeonBuildMode::StarterSpawner:
    {
        FRoomFootprint Footprint;
        Footprint.Width = 2;
        Footprint.Height = 2;
        Footprint.Depth = 1;
        CurrentPlacementPreview = DungeonSimSubsystem->PreviewRoomPlacement(HoveredBlockCoord, Footprint, DungeonSimSubsystem->GetStarterSpawnerCost());
        break;
    }

    case EDungeonBuildMode::StarterTrap:
        CurrentPlacementPreview = DungeonSimSubsystem->PreviewTrapPlacement(HoveredBlockCoord, DungeonSimSubsystem->GetStarterTrapCost());
        break;

    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled build mode while updating placement preview."));
        break;
    }

    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetPreviewValidity(CurrentPlacementPreview.bSuccess);
    }

    OnPlacementPreviewChanged.Broadcast(CurrentPlacementPreview);
}

void ADungeonPlayerController::UpdateCameraPanFromMouse()
{
    int32 ViewportX = 0;
    int32 ViewportY = 0;
    GetViewportSize(ViewportX, ViewportY);

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    if (!GetMousePosition(MouseX, MouseY))
    {
        return;
    }

    const float EdgeThreshold = 16.0f;
    if (MouseY <= EdgeThreshold)
    {
        PanCameraNorth();
    }
    else if (MouseY >= static_cast<float>(ViewportY) - EdgeThreshold)
    {
        PanCameraSouth();
    }

    if (MouseX <= EdgeThreshold)
    {
        PanCameraWest();
    }
    else if (MouseX >= static_cast<float>(ViewportX) - EdgeThreshold)
    {
        PanCameraEast();
    }
}

void ADungeonPlayerController::PanCameraNorth()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->PanInWorldSpace(FVector(0.0f, -60.0f, 0.0f));
    }
}

void ADungeonPlayerController::PanCameraSouth()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->PanInWorldSpace(FVector(0.0f, 60.0f, 0.0f));
    }
}

void ADungeonPlayerController::PanCameraEast()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->PanInWorldSpace(FVector(60.0f, 0.0f, 0.0f));
    }
}

void ADungeonPlayerController::PanCameraWest()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->PanInWorldSpace(FVector(-60.0f, 0.0f, 0.0f));
    }
}

void ADungeonPlayerController::ZoomIn()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->AdjustZoom(-150.0f);
    }
}

void ADungeonPlayerController::ZoomOut()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->AdjustZoom(150.0f);
    }
}

void ADungeonPlayerController::RestartRun()
{
    if (!bRunActive)
    {
        return;
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        DungeonSimSubsystem->RestartCurrentRun();
        UpdatePlacementPreview();
    }
}

void ADungeonPlayerController::QuickSaveRun()
{
    if (!bRunActive)
    {
        return;
    }

    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        DungeonSimSubsystem->SaveCurrentRun(TEXT("quicksave"));
    }
}

void ADungeonPlayerController::QuickLoadRun()
{
    if (UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem())
    {
        const bool bLoaded = DungeonSimSubsystem->LoadSavedRun(TEXT("quicksave"));
        if (bLoaded)
        {
            if (SetupWidget != nullptr)
            {
                SetupWidget->RemoveFromParent();
                SetupWidget = nullptr;
            }

            if (HudWidget == nullptr)
            {
                HudWidget = CreateWidget<UDungeonHudWidget>(this, UDungeonHudWidget::StaticClass());
            }

            if (HudWidget != nullptr && !HudWidget->IsInViewport())
            {
                HudWidget->AddToViewport();
            }

            bRunActive = true;
            UpdatePlacementPreview();
        }
    }
}

void ADungeonPlayerController::RefreshChunkVisuals()
{
    ADungeonChunkActor* ChunkActor = FindChunkActor();
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ChunkActor == nullptr || DungeonSimSubsystem == nullptr)
    {
        return;
    }

    ChunkActor->RefreshFromSimulation(
        DungeonSimSubsystem->GetGridState(),
        DungeonSimSubsystem->GetActiveAdventurers(),
        DungeonSimSubsystem->GetActiveDefenders()
    );
}

UDungeonSimSubsystem* ADungeonPlayerController::GetDungeonSimSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonSimSubsystem>() : nullptr;
}

ADungeonChunkActor* ADungeonPlayerController::FindChunkActor() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    for (TActorIterator<ADungeonChunkActor> It(World); It; ++It)
    {
        return *It;
    }

    return nullptr;
}

void ADungeonPlayerController::EnsureBuildCursor()
{
    if (BuildCursorActor != nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    BuildCursorActor = World->SpawnActor<ADungeonBuildCursorActor>(ADungeonBuildCursorActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetPreviewMode(BuildMode);
    }
}

ADungeonCameraPawn* ADungeonPlayerController::GetDungeonCameraPawn() const
{
    return Cast<ADungeonCameraPawn>(GetPawn());
}
