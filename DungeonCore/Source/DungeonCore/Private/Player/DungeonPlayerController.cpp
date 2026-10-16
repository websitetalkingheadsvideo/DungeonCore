#include "Player/DungeonPlayerController.h"

#include "Data/DungeonContentSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
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

    // Controls live in the HUD now; only the misconfiguration diagnostic stays on screen.
    if (GEngine != nullptr && GetDungeonCameraPawn() == nullptr)
    {
        GEngine->AddOnScreenDebugMessage(
            2,
            15.0f,
            FColor::Red,
            TEXT("No DungeonCameraPawn — Game Mode is not DungeonGameMode (rebuild C++ or check Maps & Modes).")
        );
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

    // Number row selects the Nth placeable shop item (see GetShopHotkeyBuildableIds).
    const TArray<FKey> HotkeyKeys = {
        EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five,
        EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine
    };
    check(HotkeyKeys.Num() == DungeonCore::MaxShopHotkeys);
    for (int32 HotkeyIndex = 0; HotkeyIndex < HotkeyKeys.Num(); ++HotkeyIndex)
    {
        FInputKeyBinding HotkeyBinding(FInputChord(HotkeyKeys[HotkeyIndex]), IE_Pressed);
        HotkeyBinding.KeyDelegate.GetDelegateForManualSet().BindUObject(
            this,
            &ADungeonPlayerController::SelectShopHotkey,
            HotkeyIndex
        );
        InputComponent->KeyBindings.Add(HotkeyBinding);
    }

    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ADungeonPlayerController::RecenterCameraOnCore);
    InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ADungeonPlayerController::ZoomIn);
    InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ADungeonPlayerController::ZoomOut);
    InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ADungeonPlayerController::RestartRun);
    InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ADungeonPlayerController::ToggleDungeonStore);
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ADungeonPlayerController::CloseDungeonStore);
    InputComponent->BindKey(EKeys::F5, IE_Pressed, this, &ADungeonPlayerController::QuickSaveRun);
    InputComponent->BindKey(EKeys::F9, IE_Pressed, this, &ADungeonPlayerController::QuickLoadRun);
}

void ADungeonPlayerController::ActivateShopItem(const FName BuildableId)
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("ActivateShopItem: subsystems unavailable."));
        return;
    }

    const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
    if (Row == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("ActivateShopItem: unknown buildable '%s'."), *BuildableId.ToString());
        return;
    }

    if (Row->Category == EBuildableCategory::Upgrade)
    {
        const FPlacementResult Result = DungeonSimSubsystem->PurchaseUpgrade(BuildableId);
        if (!Result.bSuccess)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Upgrade '%s' not bought (reason %d)."),
                *BuildableId.ToString(),
                static_cast<int32>(Result.FailureReason)
            );
        }
        return;
    }

    SelectBuildable(BuildableId);
}

void ADungeonPlayerController::SelectBuildable(const FName BuildableId)
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (ContentSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Dungeon content subsystem is required to select a buildable."));
        return;
    }

    const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
    if (Row == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SelectBuildable: unknown buildable '%s'."), *BuildableId.ToString());
        return;
    }

    if (Row->Category == EBuildableCategory::Upgrade)
    {
        UE_LOG(LogTemp, Error, TEXT("SelectBuildable: '%s' is an upgrade and cannot be placed."), *BuildableId.ToString());
        return;
    }

    SelectedBuildableId = BuildableId;
    OnSelectedBuildableChanged.Broadcast(SelectedBuildableId);

    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetPreviewBuildable(Row->Category, Row->Footprint);
        BuildCursorActor->SetPreviewOriginBlock(HoveredBlockCoord);
    }

    UpdatePlacementPreview();
}

FName ADungeonPlayerController::GetSelectedBuildableId() const
{
    return SelectedBuildableId;
}

TArray<FName> ADungeonPlayerController::GetShopHotkeyBuildableIds() const
{
    TArray<FName> HotkeyIds;

    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (ContentSubsystem == nullptr || DungeonSimSubsystem == nullptr)
    {
        return HotkeyIds;
    }

    for (const FDungeonShopGroup& Group : ContentSubsystem->GetShopGroups())
    {
        for (const FName& BuildableId : Group.BuildableIds)
        {
            const FDungeonBuildableRow* Row = ContentSubsystem->FindBuildable(BuildableId);
            if (Row == nullptr || Row->Category == EBuildableCategory::Upgrade)
            {
                continue;
            }

            if (!DungeonSimSubsystem->IsBuildableAvailableForRun(BuildableId))
            {
                continue;
            }

            HotkeyIds.Add(BuildableId);
            if (HotkeyIds.Num() >= DungeonCore::MaxShopHotkeys)
            {
                return HotkeyIds;
            }
        }
    }

    return HotkeyIds;
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

    KeepMouseCursorFree();

    bRunActive = true;
    EnsureChunkActorExists();
    RefreshChunkVisuals();
    SelectBuildable(FName(DungeonCore::DigBuildableId));
}

bool ADungeonPlayerController::IsRunActive() const
{
    return bRunActive;
}

void ADungeonPlayerController::DungeonDig(const int32 BlockX, const int32 BlockY)
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("DungeonDig: sim subsystem unavailable."));
        return;
    }

    const FBlockCoord BlockCoord(BlockX, BlockY, 0);
    const FPlacementResult Result = DungeonSimSubsystem->DigBlock(BlockCoord);
    UE_LOG(
        LogTemp,
        Log,
        TEXT("DungeonDig (%d, %d): %s (reason %d)."),
        BlockX,
        BlockY,
        Result.bSuccess ? TEXT("dug") : TEXT("refused"),
        static_cast<int32>(Result.FailureReason)
    );
}

void ADungeonPlayerController::DungeonGrantMana(const int32 Amount)
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("DungeonGrantMana: sim subsystem unavailable."));
        return;
    }
    DungeonSimSubsystem->AddMana(Amount);
}

void ADungeonPlayerController::DungeonBuy(const FString& BuildableId)
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("DungeonBuy: sim subsystem unavailable."));
        return;
    }

    const FPlacementResult Result = DungeonSimSubsystem->PurchaseUpgrade(FName(*BuildableId));
    UE_LOG(
        LogTemp,
        Log,
        TEXT("DungeonBuy %s: %s (reason %d, cost %d)."),
        *BuildableId,
        Result.bSuccess ? TEXT("bought") : TEXT("refused"),
        static_cast<int32>(Result.FailureReason),
        Result.ManaCost
    );
}

void ADungeonPlayerController::DungeonPlace(const FString& BuildableId, const int32 BlockX, const int32 BlockY)
{
    SelectBuildable(FName(*BuildableId));
    if (SelectedBuildableId != FName(*BuildableId))
    {
        return;
    }

    const FPlacementResult Result = PlaceSelectedBuildableAt(FBlockCoord(BlockX, BlockY, 0));
    UE_LOG(
        LogTemp,
        Log,
        TEXT("DungeonPlace %s at (%d, %d): %s (reason %d)."),
        *BuildableId,
        BlockX,
        BlockY,
        Result.bSuccess ? TEXT("placed") : TEXT("refused"),
        static_cast<int32>(Result.FailureReason)
    );
}

void ADungeonPlayerController::DungeonStatus(const float RepeatSeconds)
{
    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    if (DungeonSimSubsystem == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("DungeonStatus: sim subsystem unavailable."));
        return;
    }

    if (RepeatSeconds > 0.0f)
    {
        FTimerDelegate RepeatDelegate;
        RepeatDelegate.BindUObject(this, &ADungeonPlayerController::DungeonStatus, 0.0f);
        GetWorldTimerManager().SetTimer(StatusLogTimerHandle, RepeatDelegate, RepeatSeconds, true);
    }

    const FResourceState Resources = DungeonSimSubsystem->GetResourceState();
    const FFloorState Floor = DungeonSimSubsystem->GetFloorState();
    FString Entries;
    for (const FBlockCoord& Entry : Floor.SurfaceEntryBlocks)
    {
        Entries += FString::Printf(TEXT("(%d,%d) "), Entry.X, Entry.Y);
    }
    const TArray<FUnitInstanceData> Adventurers = DungeonSimSubsystem->GetActiveAdventurers();
    FString AdventurerPositions;
    for (const FUnitInstanceData& Adventurer : Adventurers)
    {
        AdventurerPositions += FString::Printf(
            TEXT("(%d,%d k%d) "),
            Adventurer.CurrentBlock.X,
            Adventurer.CurrentBlock.Y,
            Adventurer.KnownBlocks.Num()
        );
    }
    UE_LOG(
        LogTemp,
        Log,
        TEXT("DungeonStatus: mana %d (+%.2f/s, base %.2f) | breach %s | entries %d [%s] | wave %d | adventurers %d [%s] | defenders %d | core %d"),
        Resources.CurrentMana,
        Resources.ManaIncomePerSecond,
        Resources.BaseManaPerSecond,
        Floor.BreachState == EBreachState::Open ? TEXT("Open") : TEXT("Sealed"),
        Floor.SurfaceEntryBlocks.Num(),
        *Entries.TrimEnd(),
        DungeonSimSubsystem->GetCurrentWaveNumber(),
        Adventurers.Num(),
        *AdventurerPositions.TrimEnd(),
        DungeonSimSubsystem->GetActiveDefenders().Num(),
        DungeonSimSubsystem->GetCurrentCoreHealth()
    );
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
    bPrimaryClickStartedOverWorld = IsPointerOverDungeonWorld();
}

void ADungeonPlayerController::HandleLeftMouseReleased()
{
    if (bDungeonStoreOpen || !bPrimaryClickStartedOverWorld)
    {
        return;
    }

    if (LeftMouseDragDistancePixels <= ClickDragThresholdPixels)
    {
        HandlePrimaryAction();
    }
}

void ADungeonPlayerController::ApplyClickDragCameraInput()
{
    if (bDungeonStoreOpen)
    {
        bHasPreviousMousePosition = false;
        return;
    }

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
    else if (bLeftMouseDown && bPrimaryClickStartedOverWorld && (!FMath::IsNearlyZero(DeltaX) || !FMath::IsNearlyZero(DeltaY)))
    {
        CameraPawn->AddPanFromScreenDelta(DeltaX, DeltaY);
        LeftMouseDragDistancePixels += FVector2D(DeltaX, DeltaY).Size();
    }
}

bool ADungeonPlayerController::IsSolidRockDigTarget(const FBlockCoord& BlockCoord) const
{
    const UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    const UDungeonGridState* GridState = DungeonSimSubsystem != nullptr ? DungeonSimSubsystem->GetGridState() : nullptr;
    const FTileData* TileData = GridState != nullptr ? GridState->FindTileData(BlockCoord) : nullptr;
    return TileData != nullptr && TileData->bDiggable && TileData->TileState == ETileState::SolidRock;
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

    const FPlacementResult Result = IsSolidRockDigTarget(HoveredBlockCoord)
        ? DungeonSimSubsystem->DigBlock(HoveredBlockCoord)
        : PlaceSelectedBuildableAt(HoveredBlockCoord);

    if (HudWidget == nullptr)
    {
        UpdatePlacementPreview();
        return;
    }

    if (Result.bSuccess)
    {
        HudWidget->HideOutOfManaNotice();
    }
    else if (Result.FailureReason == EPlacementFailureReason::InsufficientMana)
    {
        HudWidget->ShowOutOfManaNotice(Result.ManaCost);
    }

    UpdatePlacementPreview();
}

void ADungeonPlayerController::SelectShopHotkey(const int32 HotkeyIndex)
{
    if (bDungeonStoreOpen)
    {
        return;
    }

    const TArray<FName> HotkeyIds = GetShopHotkeyBuildableIds();
    if (!HotkeyIds.IsValidIndex(HotkeyIndex))
    {
        return;
    }

    SelectBuildable(HotkeyIds[HotkeyIndex]);
}

FPlacementResult ADungeonPlayerController::PlaceSelectedBuildableAt(const FBlockCoord& BlockCoord)
{
    FPlacementResult Result;

    UDungeonSimSubsystem* DungeonSimSubsystem = GetDungeonSimSubsystem();
    const FDungeonBuildableRow* Row = GetSelectedBuildableRow();
    if (DungeonSimSubsystem == nullptr || Row == nullptr)
    {
        Result.FailureReason = EPlacementFailureReason::UnknownBuildable;
        return Result;
    }

    switch (Row->Category)
    {
    case EBuildableCategory::Dig:
        return DungeonSimSubsystem->DigBlock(BlockCoord);

    case EBuildableCategory::Room:
        return DungeonSimSubsystem->PlaceRoom(BlockCoord, Row->RoomTypeId, Row->Footprint, Row->ManaCost);

    case EBuildableCategory::Trap:
        return DungeonSimSubsystem->PlaceTrap(BlockCoord, Row->TrapTypeId, Row->ManaCost);

    case EBuildableCategory::Upgrade:
        UE_LOG(LogTemp, Error, TEXT("Upgrade '%s' is selected as a placement tool; this should be impossible."), *Row->BuildableId.ToString());
        Result.FailureReason = EPlacementFailureReason::UnknownBuildable;
        return Result;

    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled buildable category while placing."));
        return Result;
    }
}

const FDungeonBuildableRow* ADungeonPlayerController::GetSelectedBuildableRow() const
{
    const UDungeonContentSubsystem* ContentSubsystem = GetDungeonContentSubsystem();
    if (ContentSubsystem == nullptr || SelectedBuildableId.IsNone())
    {
        return nullptr;
    }

    return ContentSubsystem->FindBuildable(SelectedBuildableId);
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
    if (bDungeonStoreOpen)
    {
        if (BuildCursorActor != nullptr)
        {
            BuildCursorActor->SetActorHiddenInGame(true);
        }
        return;
    }

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
    if (IsSolidRockDigTarget(HoveredBlockCoord))
    {
        BuildCursorActor->SetPreviewBuildable(EBuildableCategory::Dig, FRoomFootprint());
    }
    else if (const FDungeonBuildableRow* Row = GetSelectedBuildableRow())
    {
        BuildCursorActor->SetPreviewBuildable(Row->Category, Row->Footprint);
    }
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

    const FDungeonBuildableRow* Row = GetSelectedBuildableRow();
    if (IsSolidRockDigTarget(HoveredBlockCoord))
    {
        CurrentPlacementPreview = DungeonSimSubsystem->PreviewDigAtBlock(HoveredBlockCoord);
    }
    else if (Row == nullptr)
    {
        CurrentPlacementPreview = FPlacementResult();
        CurrentPlacementPreview.FailureReason = EPlacementFailureReason::UnknownBuildable;
    }
    else
    {
        switch (Row->Category)
        {
        case EBuildableCategory::Dig:
            CurrentPlacementPreview = DungeonSimSubsystem->PreviewDigAtBlock(HoveredBlockCoord);
            break;

        case EBuildableCategory::Room:
            CurrentPlacementPreview = DungeonSimSubsystem->PreviewRoomPlacement(HoveredBlockCoord, Row->Footprint, Row->ManaCost);
            break;

        case EBuildableCategory::Trap:
            CurrentPlacementPreview = DungeonSimSubsystem->PreviewTrapPlacement(HoveredBlockCoord, Row->ManaCost);
            break;

        case EBuildableCategory::Upgrade:
            CurrentPlacementPreview = FPlacementResult();
            CurrentPlacementPreview.FailureReason = EPlacementFailureReason::UnknownBuildable;
            break;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled buildable category while updating placement preview."));
            break;
        }
    }

    if (BuildCursorActor != nullptr)
    {
        BuildCursorActor->SetPreviewValidity(CurrentPlacementPreview.bSuccess);
    }

    OnPlacementPreviewChanged.Broadcast(CurrentPlacementPreview);
}

void ADungeonPlayerController::ZoomIn()
{
    if (bDungeonStoreOpen)
    {
        return;
    }

    if (ADungeonCameraPawn* CameraPawn = GetDungeonCameraPawn())
    {
        CameraPawn->AdjustZoom(-350.0f);
    }
}

void ADungeonPlayerController::ZoomOut()
{
    if (bDungeonStoreOpen)
    {
        return;
    }

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

void ADungeonPlayerController::KeepMouseCursorFree()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameViewportClient* Viewport = World->GetGameViewport())
        {
            Viewport->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
            Viewport->SetMouseLockMode(EMouseLockMode::DoNotLock);
            Viewport->SetHideCursorDuringCapture(false);
        }
    }

    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().ReleaseAllPointerCapture();
    }

    // Stay in Game+UI so the vault button and the world both take clicks. Closing the vault must not
    // drop back to game-only, or the vault chip stops receiving the mouse.
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    SetShowMouseCursor(true);
}

bool ADungeonPlayerController::IsPointerOverDungeonWorld() const
{
    if (!FSlateApplication::IsInitialized())
    {
        return true;
    }

    FSlateApplication& Slate = FSlateApplication::Get();
    const FWidgetPath Path = Slate.LocateWindowUnderMouse(Slate.GetCursorPos(), Slate.GetInteractiveTopLevelWindows());
    if (!Path.IsValid())
    {
        return true;
    }

    // The editor window itself is an SBorder. Only widgets under our UMG (SObjectWidget) are the HUD.
    bool bInsideGameWidget = false;
    for (int32 WidgetIndex = 0; WidgetIndex < Path.Widgets.Num(); ++WidgetIndex)
    {
        const FName WidgetType = Path.Widgets[WidgetIndex].Widget->GetType();
        if (WidgetType == FName(TEXT("SObjectWidget")))
        {
            bInsideGameWidget = true;
            continue;
        }

        if (!bInsideGameWidget)
        {
            continue;
        }

        if (WidgetType == FName(TEXT("SButton"))
            || WidgetType == FName(TEXT("SBorder"))
            || WidgetType == FName(TEXT("STextBlock"))
            || WidgetType == FName(TEXT("SScrollBox")))
        {
            return false;
        }
    }

    return true;
}

void ADungeonPlayerController::SetDungeonStoreOpen(const bool bOpen)
{
    if (!bRunActive || HudWidget == nullptr)
    {
        return;
    }

    if (bDungeonStoreOpen == bOpen)
    {
        return;
    }

    bDungeonStoreOpen = bOpen;
    HudWidget->SetStoreVisible(bOpen);

    KeepMouseCursorFree();
}

void ADungeonPlayerController::ToggleDungeonStore()
{
    SetDungeonStoreOpen(!bDungeonStoreOpen);
}

void ADungeonPlayerController::CloseDungeonStore()
{
    if (bDungeonStoreOpen)
    {
        SetDungeonStoreOpen(false);
    }
}

bool ADungeonPlayerController::IsDungeonStoreOpen() const
{
    return bDungeonStoreOpen;
}

void ADungeonPlayerController::RestartRun()
{
    if (!bRunActive)
    {
        return;
    }

    if (bDungeonStoreOpen)
    {
        SetDungeonStoreOpen(false);
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

UDungeonContentSubsystem* ADungeonPlayerController::GetDungeonContentSubsystem() const
{
    UGameInstance* GameInstance = GetGameInstance();
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UDungeonContentSubsystem>() : nullptr;
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
        if (const FDungeonBuildableRow* Row = GetSelectedBuildableRow())
        {
            BuildCursorActor->SetPreviewBuildable(Row->Category, Row->Footprint);
        }
    }
}

ADungeonCameraPawn* ADungeonPlayerController::GetDungeonCameraPawn() const
{
    return Cast<ADungeonCameraPawn>(GetPawn());
}
