#include "Game/DungeonGameMode.h"

#include "DungeonCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape.h"
#include "Player/DungeonCameraPawn.h"
#include "Sim/DungeonTypes.h"
#include "GameFramework/PlayerController.h"
#include "Player/DungeonPlayerController.h"
#include "Sim/DungeonSimSubsystem.h"
#include "Sim/Grid/DungeonGridState.h"
#include "World/DungeonBuildCursorActor.h"
#include "World/DungeonChunkActor.h"

namespace DungeonCoreWorld
{
    bool IsLegacyPrototypeDungeonActor(const AActor* Actor)
    {
        if (Actor == nullptr)
        {
            return false;
        }

        if (Actor->IsA<ADungeonChunkActor>()
            || Actor->IsA<ADungeonCameraPawn>()
            || Actor->IsA<ADungeonBuildCursorActor>()
            || Actor->IsA<ADungeonPlayerController>())
        {
            return false;
        }

        const FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("DungeonChunkActor"), ESearchCase::IgnoreCase)
            || ClassName.Contains(TEXT("DungeonCameraPawn"), ESearchCase::IgnoreCase)
            || ClassName.Contains(TEXT("DungeonBuildCursorActor"), ESearchCase::IgnoreCase)
            || ClassName.Contains(TEXT("DungeonPlayerController"), ESearchCase::IgnoreCase)
            || ClassName.Equals(TEXT("DungeonGameMode"), ESearchCase::IgnoreCase))
        {
            return false;
        }

        if (ClassName.Contains(TEXT("Dungeon"), ESearchCase::IgnoreCase))
        {
            return true;
        }

        const FString ActorLabel = Actor->GetActorLabel();
        if (ActorLabel.Contains(TEXT("Dungeon"), ESearchCase::IgnoreCase)
            && !ActorLabel.Contains(TEXT("Chunk"), ESearchCase::IgnoreCase))
        {
            return true;
        }

        return false;
    }

    int32 DestroyLegacyPrototypeActors(UWorld* World)
    {
        if (World == nullptr)
        {
            return 0;
        }

        TArray<AActor*> ActorsToDestroy;
        for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
        {
            if (IsLegacyPrototypeDungeonActor(*ActorIt))
            {
                ActorsToDestroy.Add(*ActorIt);
            }
        }

        for (AActor* ActorToDestroy : ActorsToDestroy)
        {
            UE_LOG(
                LogDungeonCore,
                Warning,
                TEXT("Removing extra dungeon actor '%s' (%s) so only C++ DungeonChunkActor draws the grid."),
                *ActorToDestroy->GetActorLabel(),
                *ActorToDestroy->GetClass()->GetName()
            );
            ActorToDestroy->Destroy();
        }

        return ActorsToDestroy.Num();
    }

    void HideEditorHelperActors(UWorld* World)
    {
        if (World == nullptr)
        {
            return;
        }

        for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
        {
            const FString ActorLabel = ActorIt->GetActorLabel();
            const FString ClassName = ActorIt->GetClass()->GetName();
            if (ActorLabel.Contains(TEXT("CameraPlane"), ESearchCase::IgnoreCase)
                || ActorLabel.Contains(TEXT("BuildCamera"), ESearchCase::IgnoreCase)
                || ClassName.Contains(TEXT("CameraPlane"), ESearchCase::IgnoreCase))
            {
                ActorIt->SetActorHiddenInGame(true);
                ActorIt->SetActorEnableCollision(false);
            }
        }
    }

    void HideTemplateWorldActors(UWorld* World)
    {
        if (World == nullptr)
        {
            return;
        }

        HideEditorHelperActors(World);

        for (TActorIterator<ALandscape> LandscapeIt(World); LandscapeIt; ++LandscapeIt)
        {
            LandscapeIt->SetActorHiddenInGame(true);
            LandscapeIt->SetActorEnableCollision(false);
        }

        for (TActorIterator<AStaticMeshActor> StaticMeshIt(World); StaticMeshIt; ++StaticMeshIt)
        {
            const FString ActorLabel = StaticMeshIt->GetActorLabel();
            const UStaticMeshComponent* MeshComponent = StaticMeshIt->GetStaticMeshComponent();
            const UStaticMesh* StaticMesh = MeshComponent != nullptr ? MeshComponent->GetStaticMesh() : nullptr;
            const FString MeshName = StaticMesh != nullptr ? StaticMesh->GetName() : FString();

            const bool bIsTemplateFloor = ActorLabel.Equals(TEXT("Floor"), ESearchCase::IgnoreCase)
                || MeshName.Contains(TEXT("Template"), ESearchCase::IgnoreCase)
                || MeshName.Contains(TEXT("Grid"), ESearchCase::IgnoreCase);

            if (!bIsTemplateFloor)
            {
                continue;
            }

            StaticMeshIt->SetActorHiddenInGame(true);
            StaticMeshIt->SetActorEnableCollision(false);
        }
    }

    int32 DestroyLoosePrototypeCubeMeshes(UWorld* World)
    {
        if (World == nullptr)
        {
            return 0;
        }

        TArray<AActor*> ActorsToDestroy;
        for (TActorIterator<AStaticMeshActor> StaticMeshIt(World); StaticMeshIt; ++StaticMeshIt)
        {
            const UStaticMesh* StaticMesh = StaticMeshIt->GetStaticMeshComponent() != nullptr
                ? StaticMeshIt->GetStaticMeshComponent()->GetStaticMesh()
                : nullptr;
            if (StaticMesh == nullptr)
            {
                continue;
            }

            const FString MeshName = StaticMesh->GetName();
            if (MeshName.Contains(TEXT("Cube"), ESearchCase::IgnoreCase)
                || MeshName.Contains(TEXT("Chamfer"), ESearchCase::IgnoreCase))
            {
                ActorsToDestroy.Add(*StaticMeshIt);
            }
        }

        for (AActor* ActorToDestroy : ActorsToDestroy)
        {
            ActorToDestroy->Destroy();
        }

        return ActorsToDestroy.Num();
    }
}

ADungeonGameMode::ADungeonGameMode()
{
    DefaultPawnClass = ADungeonCameraPawn::StaticClass();
    PlayerControllerClass = ADungeonPlayerController::StaticClass();
}

namespace DungeonCoreWorld
{
    ADungeonChunkActor* FindOrSpawnChunkActor(UWorld* World)
    {
        if (World == nullptr)
        {
            return nullptr;
        }

        for (TActorIterator<ADungeonChunkActor> ChunkIt(World); ChunkIt; ++ChunkIt)
        {
            return *ChunkIt;
        }

        return World->SpawnActor<ADungeonChunkActor>(
            ADungeonChunkActor::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator
        );
    }

    void BootstrapAuthoritativeDungeonGrid(UWorld* World)
    {
        if (World == nullptr)
        {
            return;
        }

        UGameInstance* GameInstance = World->GetGameInstance();
        UDungeonSimSubsystem* DungeonSimSubsystem = GameInstance != nullptr
            ? GameInstance->GetSubsystem<UDungeonSimSubsystem>()
            : nullptr;
        ADungeonChunkActor* ChunkActor = FindOrSpawnChunkActor(World);
        if (DungeonSimSubsystem == nullptr || ChunkActor == nullptr)
        {
            UE_LOG(LogDungeonCore, Error, TEXT("Failed to bootstrap dungeon grid (sim or chunk actor missing)."));
            return;
        }

        FRunSetup RunSetup;
        RunSetup.ThemeId = EDungeonThemeId::Gothic;
        RunSetup.CreatureFamilyId = ECreatureFamilyId::Undead;
        RunSetup.Seed = 1;
        RunSetup.StartingFloorIndex = 1;
        DungeonSimSubsystem->StartNewRun(RunSetup);

        ChunkActor->RefreshFromSimulation(
            DungeonSimSubsystem->GetGridState(),
            DungeonSimSubsystem->GetActiveAdventurers(),
            DungeonSimSubsystem->GetActiveDefenders()
        );

        const int32 TileCount = DungeonSimSubsystem->GetGridState() != nullptr
            ? DungeonSimSubsystem->GetGridState()->GetAllTiles().Num()
            : 0;

        UE_LOG(
            LogDungeonCore,
            Log,
            TEXT("Dungeon grid built: %d tiles, %dx%d block CoreChamber carved at origin, %duu blocks."),
            TileCount,
            DungeonCore::StartingCoreChamberWidthBlocks,
            DungeonCore::StartingCoreChamberHeightBlocks,
            DungeonCore::BlockSizeUnits
        );

        if (GEngine != nullptr)
        {
            GEngine->AddOnScreenDebugMessage(
                0,
                12.0f,
                FColor::Cyan,
                FString::Printf(
                    TEXT("Dungeon grid: %d tiles | %dx%d block (%d ft) starting chamber carved into rock."),
                    TileCount,
                    DungeonCore::StartingCoreChamberWidthBlocks,
                    DungeonCore::StartingCoreChamberHeightBlocks,
                    DungeonCore::StartingCoreChamberWidthBlocks * 10
                )
            );
        }
    }
}

void ADungeonGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (NewPlayer == nullptr)
    {
        return;
    }

    if (APawn* CameraPawn = NewPlayer->GetPawn())
    {
        CameraPawn->SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
        NewPlayer->SetViewTarget(CameraPawn);
    }
}

void ADungeonGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const int32 RemovedLegacyActors = DungeonCoreWorld::DestroyLegacyPrototypeActors(World);
    const int32 RemovedLooseCubes = DungeonCoreWorld::DestroyLoosePrototypeCubeMeshes(World);
    if (RemovedLegacyActors > 0 || RemovedLooseCubes > 0)
    {
        UE_LOG(
            LogDungeonCore,
            Warning,
            TEXT("Cleared %d extra dungeon actor(s) and %d loose cube mesh(es) from the level."),
            RemovedLegacyActors,
            RemovedLooseCubes
        );
    }

    DungeonCoreWorld::HideTemplateWorldActors(World);
    DungeonCoreWorld::BootstrapAuthoritativeDungeonGrid(World);

    UE_LOG(LogDungeonCore, Log, TEXT("DungeonGameMode BeginPlay complete."));
}
