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

    Footprint.Width = 1;
    Footprint.Height = 1;
    Footprint.Depth = 1;
    SetPreviewBuildable(EBuildableCategory::Dig, Footprint);
    SetPreviewOriginBlock(FBlockCoord(0, 0, 0));
}

void ADungeonBuildCursorActor::SetPreviewBuildable(const EBuildableCategory InCategory, const FRoomFootprint& InFootprint)
{
    if (!InFootprint.IsValid())
    {
        UE_LOG(LogTemp, Fatal, TEXT("Build cursor received an invalid footprint."));
        return;
    }

    Category = InCategory;
    Footprint = InFootprint;
    RefreshPreviewTransform();
}

void ADungeonBuildCursorActor::SetPreviewOriginBlock(const FBlockCoord& InOriginBlock)
{
    OriginBlock = InOriginBlock;
    SetActorLocation(GetPreviewWorldLocation(OriginBlock));
}

void ADungeonBuildCursorActor::SetHoveredTileState(const ETileState InTileState)
{
    HoveredTileState = InTileState;
    RefreshPreviewTransform();
}

void ADungeonBuildCursorActor::SetPreviewValidity(const bool bIsValid)
{
    bPreviewValid = bIsValid;
    RefreshPreviewTransform();
}

FVector ADungeonBuildCursorActor::GetPreviewWorldLocation(const FBlockCoord& InOriginBlock) const
{
    const double FloorScaleZ = DungeonCore::CarvedFloorThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;

    if (Category == EBuildableCategory::Dig)
    {
        if (IsDigOnSolidRock())
        {
            return DungeonCore::GetDigSurfaceHighlightWorldCenter(InOriginBlock);
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

FVector ADungeonBuildCursorActor::GetPreviewScale() const
{
    const double XScale = static_cast<double>(Footprint.Width * DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    const double YScale = static_cast<double>(Footprint.Height * DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    const double ZScale = IsDigOnSolidRock()
        ? DungeonCore::GetDigSurfaceHighlightScaleZ()
        : (DungeonCore::CarvedFloorThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits);

    return FVector(XScale, YScale, ZScale);
}

bool ADungeonBuildCursorActor::IsDigOnSolidRock() const
{
    return Category == EBuildableCategory::Dig
        && (HoveredTileState == ETileState::SolidRock || HoveredTileState == ETileState::Wall);
}

void ADungeonBuildCursorActor::RefreshPreviewTransform()
{
    PreviewMeshComponent->SetWorldScale3D(GetPreviewScale() * (bPreviewValid ? 1.0 : 0.9));
    SetPreviewOriginBlock(OriginBlock);
}
