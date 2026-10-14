#include "World/DungeonChunkActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sim/DungeonTypes.h"
#include "Sim/Grid/DungeonGridState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace DungeonChunkVisuals
{
    inline FVector BlockMeshScaleXY()
    {
        const double BlockMeshScale = static_cast<double>(DungeonCore::BlockSizeUnits) / 100.0;
        return FVector(BlockMeshScale, BlockMeshScale, BlockMeshScale);
    }

    inline FVector BlockMeshScaleRoomFloor()
    {
        const double BlockMeshScale = static_cast<double>(DungeonCore::BlockSizeUnits) / 100.0;
        return FVector(BlockMeshScale, BlockMeshScale, BlockMeshScale * 0.22);
    }

    inline FVector BlockMeshScaleRock()
    {
        const double BlockMeshScale = static_cast<double>(DungeonCore::BlockSizeUnits) / 100.0;
        return FVector(BlockMeshScale, BlockMeshScale, BlockMeshScale);
    }
}

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

    static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterialFinder(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")
    );
    if (BaseMaterialFinder.Succeeded())
    {
        UMaterial* BaseMaterial = BaseMaterialFinder.Object;
        if (UMaterialInstanceDynamic* RockMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            RockMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.12f, 0.11f, 0.14f));
            RockInstances->SetMaterial(0, RockMaterial);
        }
        if (UMaterialInstanceDynamic* RoomMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            RoomMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.55f, 0.42f, 0.72f));
            RoomInstances->SetMaterial(0, RoomMaterial);
        }
        if (UMaterialInstanceDynamic* ClearedMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            ClearedMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.22f, 0.20f, 0.18f));
            ClearedInstances->SetMaterial(0, ClearedMaterial);
        }
        if (UMaterialInstanceDynamic* EntryMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
        {
            EntryMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.72f, 0.35f));
            SurfaceEntryInstances->SetMaterial(0, EntryMaterial);
        }
    }

    ConfigureInstanceComponent(RockInstances);
    ConfigureInstanceComponent(ClearedInstances);
    ConfigureInstanceComponent(RoomInstances);
    ConfigureInstanceComponent(TrapInstances);
    ConfigureInstanceComponent(SurfaceEntryInstances);
    ConfigureInstanceComponent(AdventurerInstances);
    ConfigureInstanceComponent(DefenderInstances);
}

FTransform ADungeonChunkActor::MakeTileInstanceTransform(const FVector& WorldCenter, const FVector& InstanceScale)
{
    FTransform InstanceTransform;
    InstanceTransform.SetLocation(WorldCenter);
    InstanceTransform.SetRotation(FQuat::Identity);
    InstanceTransform.SetScale3D(InstanceScale);
    return InstanceTransform;
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

    const FVector RockScale = DungeonChunkVisuals::BlockMeshScaleRock();
    const FVector RoomScale = DungeonChunkVisuals::BlockMeshScaleRoomFloor();
    const FVector ClearedScale = FVector(RockScale.X, RockScale.Y, RockScale.Z * 0.12);

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    for (const FTileData& TileData : Tiles)
    {
        const FVector BlockCenter = DungeonCore::BlockCenterToWorld(TileData.BlockCoord);

        switch (TileData.TileState)
        {
        case ETileState::SolidRock:
            RockInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, RockScale));
            break;

        case ETileState::Cleared:
            ClearedInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, ClearedScale));
            break;

        case ETileState::Room:
            RoomInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, RoomScale));
            break;

        case ETileState::Trap:
            TrapInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, RockScale * 0.5));
            break;

        case ETileState::SurfaceEntry:
            SurfaceEntryInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, RoomScale * 1.1));
            break;

        case ETileState::Wall:
            RockInstances->AddInstance(MakeTileInstanceTransform(BlockCenter, RockScale));
            break;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled tile state while refreshing chunk visuals."));
            break;
        }
    }

    const FVector UnitScale = FVector(1.0, 1.0, 1.0);
    for (const FUnitInstanceData& AdventurerData : ActiveAdventurers)
    {
        AdventurerInstances->AddInstance(MakeTileInstanceTransform(
            DungeonCore::BlockCenterToWorld(AdventurerData.CurrentBlock),
            UnitScale
        ));
    }

    for (const FUnitInstanceData& DefenderData : ActiveDefenders)
    {
        DefenderInstances->AddInstance(MakeTileInstanceTransform(
            DungeonCore::BlockCenterToWorld(DefenderData.CurrentBlock),
            FVector(1.0, 1.0, 0.7)
        ));
    }

    RockInstances->MarkRenderStateDirty();
    RoomInstances->MarkRenderStateDirty();
    ClearedInstances->MarkRenderStateDirty();
}

void ADungeonChunkActor::ConfigureInstanceComponent(UInstancedStaticMeshComponent* InstanceComponent) const
{
    if (InstanceComponent == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Instance component must be valid."));
    }

    InstanceComponent->SetRelativeScale3D(FVector::OneVector);
    InstanceComponent->SetMobility(EComponentMobility::Static);
    InstanceComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InstanceComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    InstanceComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}
