#include "World/DungeonChunkActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sim/DungeonTypes.h"
#include "Sim/Grid/DungeonGridState.h"
#include "UObject/ConstructorHelpers.h"

ADungeonChunkActor::ADungeonChunkActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    RockInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RockInstances"));
    RockInstances->SetupAttachment(SceneRoot);

    ClearedInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ClearedInstances"));
    ClearedInstances->SetupAttachment(SceneRoot);

    RoomInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoomInstances"));
    RoomInstances->SetupAttachment(SceneRoot);

    TrapInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TrapInstances"));
    TrapInstances->SetupAttachment(SceneRoot);

    SurfaceEntryInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SurfaceEntryInstances"));
    SurfaceEntryInstances->SetupAttachment(SceneRoot);

    AdventurerInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AdventurerInstances"));
    AdventurerInstances->SetupAttachment(SceneRoot);

    DefenderInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DefenderInstances"));
    DefenderInstances->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        RockInstances->SetStaticMesh(CubeMesh.Object);
        ClearedInstances->SetStaticMesh(CubeMesh.Object);
        RoomInstances->SetStaticMesh(CubeMesh.Object);
        TrapInstances->SetStaticMesh(CubeMesh.Object);
        SurfaceEntryInstances->SetStaticMesh(CubeMesh.Object);
        AdventurerInstances->SetStaticMesh(CubeMesh.Object);
        DefenderInstances->SetStaticMesh(CubeMesh.Object);
    }

    ConfigureInstanceComponent(RockInstances, FVector(3.0, 3.0, 3.0));
    ConfigureInstanceComponent(ClearedInstances, FVector(3.0, 3.0, 0.15));
    ConfigureInstanceComponent(RoomInstances, FVector(3.0, 3.0, 0.3));
    ConfigureInstanceComponent(TrapInstances, FVector(1.5, 1.5, 1.5));
    ConfigureInstanceComponent(SurfaceEntryInstances, FVector(3.0, 3.0, 0.5));
    ConfigureInstanceComponent(AdventurerInstances, FVector(1.0, 1.0, 1.0));
    ConfigureInstanceComponent(DefenderInstances, FVector(1.0, 1.0, 0.7));
}

void ADungeonChunkActor::RefreshFromSimulation(
    UDungeonGridState* GridState,
    const TArray<FUnitInstanceData>& ActiveAdventurers,
    const TArray<FUnitInstanceData>& ActiveDefenders
)
{
    RockInstances->ClearInstances();
    ClearedInstances->ClearInstances();
    RoomInstances->ClearInstances();
    TrapInstances->ClearInstances();
    SurfaceEntryInstances->ClearInstances();
    AdventurerInstances->ClearInstances();
    DefenderInstances->ClearInstances();

    if (GridState == nullptr)
    {
        return;
    }

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    for (const FTileData& TileData : Tiles)
    {
        const FVector BaseLocation = DungeonCore::BlockToWorld(TileData.BlockCoord);

        switch (TileData.TileState)
        {
        case ETileState::SolidRock:
            RockInstances->AddInstance(FTransform(BaseLocation + FVector(0.0, 0.0, DungeonCore::HalfBlockSizeUnits)));
            break;

        case ETileState::Cleared:
            ClearedInstances->AddInstance(FTransform(BaseLocation));
            break;

        case ETileState::Room:
            RoomInstances->AddInstance(FTransform(BaseLocation + FVector(0.0, 0.0, 10.0)));
            break;

        case ETileState::Trap:
            TrapInstances->AddInstance(FTransform(BaseLocation + FVector(0.0, 0.0, 60.0)));
            break;

        case ETileState::SurfaceEntry:
            SurfaceEntryInstances->AddInstance(FTransform(BaseLocation + FVector(0.0, 0.0, 10.0)));
            break;

        case ETileState::Wall:
            RockInstances->AddInstance(FTransform(BaseLocation + FVector(0.0, 0.0, DungeonCore::HalfBlockSizeUnits)));
            break;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled tile state while refreshing chunk visuals."));
            break;
        }
    }

    for (const FUnitInstanceData& AdventurerData : ActiveAdventurers)
    {
        AdventurerInstances->AddInstance(FTransform(
            DungeonCore::BlockToWorld(AdventurerData.CurrentBlock) + FVector(0.0, 0.0, 90.0)
        ));
    }

    for (const FUnitInstanceData& DefenderData : ActiveDefenders)
    {
        DefenderInstances->AddInstance(FTransform(
            DungeonCore::BlockToWorld(DefenderData.CurrentBlock) + FVector(0.0, 0.0, 55.0)
        ));
    }
}

void ADungeonChunkActor::ConfigureInstanceComponent(
    UInstancedStaticMeshComponent* InstanceComponent,
    const FVector& VisualScale
) const
{
    if (InstanceComponent == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Instance component must be valid."));
    }

    InstanceComponent->SetRelativeScale3D(VisualScale);
    InstanceComponent->SetMobility(EComponentMobility::Movable);
    InstanceComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InstanceComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    InstanceComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}
