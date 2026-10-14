#include "Player/DungeonPlayerController.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Player/DungeonCameraPawn.h"
#include "Sim/DungeonSimSubsystem.h"
#include "Sim/Grid/DungeonGridState.h"
#include "UI/DungeonHudWidget.h"
#include "UI/DungeonRunSetupWidget.h"
#include "World/DungeonBuildCursorActor.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape.h"
#include "World/DungeonChunkActor.h"

ADungeonPlayerController::ADungeonPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    bBlockInput = false;
}

void ADungeonPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bAutoManageActiveCameraTarget = true;

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

    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AStaticMeshActor> StaticMeshIt(World); StaticMeshIt; ++StaticMeshIt)
        {
            StaticMeshIt->SetActorHiddenInGame(true);
            StaticMeshIt->SetActorEnableCollision(false);
        }
        for (TActorIterator<ALandscape> LandscapeIt(World); LandscapeIt; ++LandscapeIt)
        {
            LandscapeIt->SetActorHiddenInGame(true);
            LandscapeIt->SetActorEnableCollision(false);
        }
    }

    if (APawn* CameraPawn = GetPawn())
    {
        CameraPawn->SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
        SetViewTarget(CameraPawn);
    }

    // Bootstrap the sim immediately so Play shows the dungeon grid (setup UI comes back later).
    StartRunFromSetup(EDungeonThemeId::Gothic, ECreatureFamilyId::Undead);

    if (GEngine != nullptr)
    {
        const FString PawnName = GetPawn() != nullptr ? GetPawn()->GetClass()->GetName() : TEXT("NONE");
        GEngine->AddOnScreenDebugMessage(
            1,
            15.0f,
            FColor::Green,
            FString::Printf(
                TEXT("Dungeon Core | %s | LMB drag = pan map, RMB drag = rotate camera, short LMB click = build/dig, F = recenter"),
                *PawnName
            )
        );
        if (GetDungeonCameraPawn() == nullptr)
        {
            GEngine->AddOnScreenDebugMessage(
                2,
                15.0f,
                FColor::Red,
                TEXT("No DungeonCameraPawn — Game Mode is not DungeonGameMode (rebuild C++ or check Maps & Modes).")
            );
        }
    }
}

void ADungeonPlayerController::PlayerTick(const float DeltaSeconds)
{
    Super::PlayerTick(DeltaSeconds);
    ApplyClickDragCameraInput();
    UpdateCursorFromMouse();
}

void ADungeonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    check(InputComponent != nullptr);

    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ADungeonPlayerController::HandleLeftMousePressed);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ADungeonPlayerController::HandleLeftMouseReleased);
    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ADungeonPlayerController::SetDigMode);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ADungeonPlayerController::SetManaWellMode);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ADungeonPlayerController::SetStarterSpawnerMode);
    InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ADungeonPlayerController::SetStarterTrapMode);
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ADungeonPlayerController::RecenterCameraOnCore);
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
        HudWidget->AddToViewport(10);
    }

    FInputModeGameOnly GameOnlyInput;
    GameOnlyInput.SetConsumeCaptureMouseDown(false);
    SetInputMode(GameOnlyInput);
    bShowMouseCursor = true;

    bRunActive = true;
    EnsureChunkActorExists();
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

void ADungeonPlayerController::HandleLeftMousePressed()
{
    LeftMouseDragDistancePixels = 0.0f;
}

void ADungeonPlayerController::HandleLeftMouseReleased()
{
    if (LeftMouseDragDistancePixels <= ClickDragThresholdPixels)
    {
        HandlePrimaryAction();
    }
}

void ADungeonPlayerController::ApplyClickDragCameraInput()
{
    ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn();
    if (CameraPawn == nullptr)
    {
        bHasPreviousMousePosition = false;
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    if (!GetMousePosition(MouseX, MouseY))
    {
        bHasPreviousMousePosition = false;
        return;
    }

    if (!bHasPreviousMousePosition)
    {
        PreviousMouseX = MouseX;
        PreviousMouseY = MouseY;
        bHasPreviousMousePosition = true;
        return;
    }

    const float DeltaX = MouseX - PreviousMouseX;
    const float DeltaY = MouseY - PreviousMouseY;
    PreviousMouseX = MouseX;
    PreviousMouseY = MouseY;

    const bool bRightMouseDown = IsInputKeyDown(EKeys::RightMouseButton);
    const bool bLeftMouseDown = IsInputKeyDown(EKeys::LeftMouseButton);

    if (bRightMouseDown && (!FMath::IsNearlyZero(DeltaX) || !FMath::IsNearlyZero(DeltaY)))
    {
        CameraPawn->AddOrbitFromScreenDelta(DeltaX, DeltaY);
    }
    else if (bLeftMouseDown && (!FMath::IsNearlyZero(DeltaX) || !FMath::IsNearlyZero(DeltaY)))
    {
        CameraPawn->AddPanFromScreenDelta(DeltaX, DeltaY);
        LeftMouseDragDistancePixels += FVector2D(DeltaX, DeltaY).Size();
    }
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

bool ADungeonPlayerController::ProjectMouseToDungeonSurface(FVector& OutWorldPoint) const
{
    FVector WorldOrigin = FVector::ZeroVector;
    FVector WorldDirection = FVector::ZeroVector;
    if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
    {
        return false;
    }

    if (FMath::IsNearlyZero(WorldDirection.Z))
    {
        return false;
    }

    const double SurfaceZ = DungeonCore::GetBlockTopWorldZ(FBlockCoord(0, 0, 0));
    const double TraceDistance = (SurfaceZ - WorldOrigin.Z) / WorldDirection.Z;
    if (TraceDistance < 0.0)
    {
        return false;
    }

    OutWorldPoint = WorldOrigin + (WorldDirection * TraceDistance);
    return true;
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

    FVector SurfacePoint = FVector::ZeroVector;
    if (!ProjectMouseToDungeonSurface(SurfacePoint))
    {
        bHasHoveredBlock = false;
        BuildCursorActor->SetActorHiddenInGame(true);
        UpdatePlacementPreview();
        return;
    }

    HoveredBlockCoord = DungeonCore::WorldToBlock(SurfacePoint);
    HoveredBlockCoord.Z = 0;

    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    UDungeonGridState* GridState = DungeonSimSubsystem != nullptr ? DungeonSimSubsystem->GetGridState() : nullptr;
    if (GridState == nullptr
        || !DungeonCore::IsBlockInStartingBuildArea(HoveredBlockCoord)
        || !GridState->HasTileData(HoveredBlockCoord))
    {
        bHasHoveredBlock = false;
        BuildCursorActor->SetActorHiddenInGame(true);
        UpdatePlacementPreview();
        return;
    }

    bHasHoveredBlock = true;

    ETileState HoveredTileState = ETileState::SolidRock;
    if (const FTileData* TileData = GridState->FindTileData(HoveredBlockCoord))
    {
        HoveredTileState = TileData->TileState;
    }

    BuildCursorActor->SetActorHiddenInGame(false);
    BuildCursorActor->SetHoveredTileState(HoveredTileState);
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

void ADungeonPlayerController::ZoomIn()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->AdjustZoom(-350.0f);
    }
}

void ADungeonPlayerController::ZoomOut()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->AdjustZoom(350.0f);
    }
}

void ADungeonPlayerController::RecenterCameraOnCore()
{
    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->RecenterOnDungeonCore();
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

    UDungeonGridState* GridState = DungeonSimSubsystem->GetGridState();
    ChunkActor->RefreshFromSimulation(
        GridState,
        DungeonSimSubsystem->GetActiveAdventurers(),
        DungeonSimSubsystem->GetActiveDefenders()
    );

    if (GEngine != nullptr && GridState != nullptr)
    {
        const int32 TileCount = GridState->GetAllTiles().Num();
        GEngine->AddOnScreenDebugMessage(
            3,
            8.0f,
            FColor::Yellow,
            FString::Printf(TEXT("Dungeon tiles: %d"), TileCount)
        );
    }
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

void ADungeonPlayerController::EnsureChunkActorExists()
{
    if (FindChunkActor() != nullptr)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    World->SpawnActor<ADungeonChunkActor>(ADungeonChunkActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
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
