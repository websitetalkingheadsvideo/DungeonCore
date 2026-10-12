#include "World/DungeonBuildCursorActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADungeonBuildCursorActor::ADungeonBuildCursorActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
    PreviewMeshComponent->SetupAttachment(SceneRoot);
    PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

void ADungeonBuildCursorActor::SetPreviewValidity(const bool bIsValid)
{
    bPreviewValid = bIsValid;
    PreviewMeshComponent->SetWorldScale3D(GetPreviewScaleForMode(BuildMode) * (bPreviewValid ? 1.0 : 0.9));
    SetPreviewOriginBlock(OriginBlock);
}

FVector ADungeonBuildCursorActor::GetPreviewWorldLocation(const FBlockCoord& InOriginBlock) const
{
    const FRoomFootprint Footprint = GetFootprintForMode(BuildMode);
    const FVector BaseLocation = DungeonCore::BlockToWorld(InOriginBlock);

    return BaseLocation + FVector(
        static_cast<double>((Footprint.Width - 1) * DungeonCore::HalfBlockSizeUnits),
        static_cast<double>((Footprint.Height - 1) * DungeonCore::HalfBlockSizeUnits),
        BuildMode == EDungeonBuildMode::Dig
            ? static_cast<double>(DungeonCore::HalfBlockSizeUnits - (bPreviewValid ? 0 : 30))
            : (bPreviewValid ? 10.0 : -10.0)
    );
}

FVector ADungeonBuildCursorActor::GetPreviewScaleForMode(const EDungeonBuildMode InBuildMode) const
{
    const FRoomFootprint Footprint = GetFootprintForMode(InBuildMode);
    const double XScale = static_cast<double>(Footprint.Width * DungeonCore::BlockSizeUnits) / 100.0;
    const double YScale = static_cast<double>(Footprint.Height * DungeonCore::BlockSizeUnits) / 100.0;
    const double ZScale = InBuildMode == EDungeonBuildMode::Dig ? 3.0 : 0.2;

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
