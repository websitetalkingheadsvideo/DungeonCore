#include "World/DungeonChunkActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sim/DungeonTypes.h"
#include "Sim/Grid/DungeonGridState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace DungeonChunkVisuals
{
    inline double BlockMeshScaleAxis()
    {
        return static_cast<double>(DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    }

    inline FVector BlockMeshScaleRock()
    {
        const double BlockMeshScale = BlockMeshScaleAxis();
        return FVector(BlockMeshScale, BlockMeshScale, BlockMeshScale);
    }

    inline FVector BlockMeshScaleCarvedFloor()
    {
        const double BlockMeshScale = BlockMeshScaleAxis();
        const double FloorScaleZ = DungeonCore::CarvedFloorThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;
        return FVector(BlockMeshScale, BlockMeshScale, FloorScaleZ);
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
    const FVector CarvedFloorScale = DungeonChunkVisuals::BlockMeshScaleCarvedFloor();

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    for (const FTileData& TileData : Tiles)
    {
        switch (TileData.TileState)
        {
        case ETileState::SolidRock:
            RockInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetRockVisualWorldCenter(TileData.BlockCoord, RockScale.Z),
                RockScale
            ));
            break;

        case ETileState::Cleared:
            ClearedInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetCarvedFloorVisualWorldCenter(TileData.BlockCoord, CarvedFloorScale.Z),
                CarvedFloorScale
            ));
            break;

        case ETileState::Room:
        {
            const bool bIsDungeonCoreBlock = TileData.RoomTypeId == ERoomTypeId::CoreChamber
                && TileData.BlockCoord.X == DungeonCore::GetStartingCoreBlockCoord().X
                && TileData.BlockCoord.Y == DungeonCore::GetStartingCoreBlockCoord().Y
                && TileData.BlockCoord.Z == DungeonCore::GetStartingCoreBlockCoord().Z;
            if (bIsDungeonCoreBlock)
            {
                FVector PillarScale(1.0, 1.0, 1.0);
                const FVector PillarCenter = DungeonCore::GetDungeonCorePillarVisual(
                    TileData.BlockCoord,
                    RockScale.X,
                    PillarScale
                );
                RoomInstances->AddInstance(MakeTileInstanceTransform(PillarCenter, PillarScale));
            }
            else
            {
                RoomInstances->AddInstance(MakeTileInstanceTransform(
                    DungeonCore::GetCarvedFloorVisualWorldCenter(TileData.BlockCoord, CarvedFloorScale.Z),
                    CarvedFloorScale
                ));
            }
            break;
        }

        case ETileState::Trap:
        {
            const FVector TrapScale = FVector(RockScale.X * 0.45, RockScale.Y * 0.45, CarvedFloorScale.Z * 1.5);
            TrapInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetCarvedFloorVisualWorldCenter(TileData.BlockCoord, TrapScale.Z),
                TrapScale
            ));
            break;
        }

        case ETileState::SurfaceEntry:
            SurfaceEntryInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetCarvedFloorVisualWorldCenter(TileData.BlockCoord, CarvedFloorScale.Z),
                CarvedFloorScale
            ));
            break;

        case ETileState::Wall:
            RockInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetRockVisualWorldCenter(TileData.BlockCoord, RockScale.Z),
                RockScale
            ));
            break;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("Unhandled tile state while refreshing chunk visuals."));
            break;
        }
    }

    const FVector UnitScale = FVector(1.0, 1.0, 1.0);
    for (const FUnitInstanceData& AdventurerData : ActiveAdventurers)
    {
        const FVector ColumnXY = DungeonCore::GetBlockColumnWorldXY(AdventurerData.CurrentBlock);
        const double WalkableZ = DungeonCore::GetCarvedWalkableWorldZ(AdventurerData.CurrentBlock);
        AdventurerInstances->AddInstance(MakeTileInstanceTransform(
            FVector(ColumnXY.X, ColumnXY.Y, WalkableZ + 50.0),
            UnitScale
        ));
    }

    for (const FUnitInstanceData& DefenderData : ActiveDefenders)
    {
        const FVector ColumnXY = DungeonCore::GetBlockColumnWorldXY(DefenderData.CurrentBlock);
        const double WalkableZ = DungeonCore::GetCarvedWalkableWorldZ(DefenderData.CurrentBlock);
        DefenderInstances->AddInstance(MakeTileInstanceTransform(
            FVector(ColumnXY.X, ColumnXY.Y, WalkableZ + 35.0),
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
    InstanceComponent->SetMobility(EComponentMobility::Movable);
    InstanceComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InstanceComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    InstanceComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}
