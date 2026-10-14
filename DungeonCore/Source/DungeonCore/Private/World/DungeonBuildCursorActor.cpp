#include "World/DungeonBuildCursorActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sim/DungeonTypes.h"
#include "UObject/ConstructorHelpers.h"

ADungeonBuildCursorActor::ADungeonBuildCursorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    PreviewMeshComponent->SetupAttachment(SceneRoot);
    PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewMeshComponent->SetMobility(EComponentMobility::Movable);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        PreviewMeshComponent->SetStaticMesh(CubeMesh.Object);
    }

    SetPreviewMode(EDungeonBuildMode::Dig);
    SetPreviewOriginBlock(FBlockCoord(0, 0, 0));
}

void ADungeonBuildCursorActor::SetPreviewMode(const EDungeonBuildMode InBuildMode)
{
    BuildMode = InBuildMode;
    PreviewMeshComponent->SetWorldScale3D(GetPreviewScaleForMode(BuildMode) * (bPreviewValid ? 1.0 : 0.9));
    SetPreviewOriginBlock(OriginBlock);
}

void ADungeonBuildCursorActor::SetPreviewOriginBlock(const FBlockCoord& InOriginBlock)
{
    OriginBlock = InOriginBlock;
    SetActorLocation(GetPreviewWorldLocation(OriginBlock));
}

void ADungeonBuildCursorActor::SetHoveredTileState(const ETileState InTileState)
{
    HoveredTileState = InTileState;
    SetPreviewOriginBlock(OriginBlock);
}

void ADungeonBuildCursorActor::SetPreviewValidity(const bool bIsValid)
{
    bPreviewValid = bIsValid;
    PreviewMeshComponent->SetWorldScale3D(GetPreviewScaleForMode(BuildMode) * (bPreviewValid ? 1.0 : 0.9));
    SetPreviewOriginBlock(OriginBlock);
}

FVector ADungeonBuildCursorActor::GetPreviewWorldLocation(const FBlockCoord& InOriginBlock) const
{
    const FRoomFootprint Footprint = GetFootprintForMode(BuildMode);
    const double FloorScaleZ = DungeonCore::CarvedFloorThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;
    const double RockScaleZ = static_cast<double>(DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;

    if (BuildMode == EDungeonBuildMode::Dig)
    {
        if (HoveredTileState == ETileState::SolidRock || HoveredTileState == ETileState::Wall)
        {
            return DungeonCore::GetRockVisualWorldCenter(InOriginBlock, RockScaleZ);
        }

        return DungeonCore::GetCarvedFloorVisualWorldCenter(InOriginBlock, FloorScaleZ);
    }

    return DungeonCore::GetCarvedFloorVisualWorldCenterForFootprint(
        InOriginBlock,
        Footprint.Width,
        Footprint.Height,
        FloorScaleZ
    );
}

FVector ADungeonBuildCursorActor::GetPreviewScaleForMode(const EDungeonBuildMode InBuildMode) const
{
    const FRoomFootprint Footprint = GetFootprintForMode(InBuildMode);
    const double XScale = static_cast<double>(Footprint.Width * DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    const double YScale = static_cast<double>(Footprint.Height * DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    const bool bDigSolidRock = InBuildMode == EDungeonBuildMode::Dig
        && (HoveredTileState == ETileState::SolidRock || HoveredTileState == ETileState::Wall);
    const double ZScale = bDigSolidRock
        ? (static_cast<double>(DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits)
        : (DungeonCore::CarvedFloorThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits);

    return FVector(XScale, YScale, ZScale);
}

FRoomFootprint ADungeonBuildCursorActor::GetFootprintForMode(const EDungeonBuildMode InBuildMode) const
{
    FRoomFootprint Footprint;

    switch (InBuildMode)
    {
    case EDungeonBuildMode::StarterSpawner:
        Footprint.Width = 2;
        Footprint.Height = 2;
        Footprint.Depth = 1;
        break;
    case EDungeonBuildMode::Dig:
    case EDungeonBuildMode::ManaWell:
    case EDungeonBuildMode::StarterTrap:
        Footprint.Width = 1;
        Footprint.Height = 1;
        Footprint.Depth = 1;
        break;
    default:
        UE_LOG(LogTemp, Fatal, TEXT("Unhandled build mode for cursor footprint."));
        break;
    }

    return Footprint;
}
