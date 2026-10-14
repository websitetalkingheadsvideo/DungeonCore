#include "World/DungeonChunkActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sim/DungeonTypes.h"
#include "Sim/Grid/DungeonGridState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace DungeonChunkVisuals
{
    void ApplyTintedMaterial(
        UInstancedStaticMeshComponent* InstanceComponent,
        UMaterial* ParentMaterial,
        UObject* Outer,
        const FLinearColor& Color,
        const float EmissiveScale
    )
    {
        if (InstanceComponent == nullptr || ParentMaterial == nullptr || Outer == nullptr)
        {
            return;
        }

        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ParentMaterial, Outer);
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("Base Color"), Color);
        const FLinearColor Emissive = Color * EmissiveScale;
        DynamicMaterial->SetVectorParameterValue(TEXT("Emissive"), Emissive);
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
        InstanceComponent->SetMaterial(0, DynamicMaterial);
    }

    void ApplyTintedMaterialToMesh(
        UStaticMeshComponent* MeshComponent,
        UMaterial* ParentMaterial,
        UObject* Outer,
        const FLinearColor& Color,
        const float EmissiveScale
    )
    {
        if (MeshComponent == nullptr || ParentMaterial == nullptr || Outer == nullptr)
        {
            return;
        }

        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ParentMaterial, Outer);
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("Base Color"), Color);
        const FLinearColor Emissive = Color * EmissiveScale;
        DynamicMaterial->SetVectorParameterValue(TEXT("Emissive"), Emissive);
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
        MeshComponent->SetMaterial(0, DynamicMaterial);
    }

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
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SetRootComponent(SceneRoot);

    GridBaseInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridBaseInstances"));
    GridBaseInstances->SetupAttachment(SceneRoot);

    GridLineInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridLineInstances"));
    GridLineInstances->SetupAttachment(SceneRoot);

    ChamberWallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ChamberWallInstances"));
    ChamberWallInstances->SetupAttachment(SceneRoot);

    CoreChamberFloorVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreChamberFloorVisual"));
    CoreChamberFloorVisual->SetupAttachment(SceneRoot);
    CoreChamberFloorVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CoreChamberFloorVisual->SetMobility(EComponentMobility::Movable);

    DungeonCoreVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DungeonCoreVisual"));
    DungeonCoreVisual->SetupAttachment(SceneRoot);
    DungeonCoreVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DungeonCoreVisual->SetMobility(EComponentMobility::Movable);

    RockInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RockInstances"));
    RockInstances->SetupAttachment(SceneRoot);

    PitCavityInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PitCavityInstances"));
    PitCavityInstances->SetupAttachment(SceneRoot);

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
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (CubeMesh.Succeeded())
    {
        CoreChamberFloorVisual->SetStaticMesh(CubeMesh.Object);
        GridBaseInstances->SetStaticMesh(CubeMesh.Object);
        GridLineInstances->SetStaticMesh(CubeMesh.Object);
        ChamberWallInstances->SetStaticMesh(CubeMesh.Object);
        RockInstances->SetStaticMesh(CubeMesh.Object);
        PitCavityInstances->SetStaticMesh(CubeMesh.Object);
        ClearedInstances->SetStaticMesh(CubeMesh.Object);
        RoomInstances->SetStaticMesh(CubeMesh.Object);
        TrapInstances->SetStaticMesh(CubeMesh.Object);
        SurfaceEntryInstances->SetStaticMesh(CubeMesh.Object);
        AdventurerInstances->SetStaticMesh(CubeMesh.Object);
        DefenderInstances->SetStaticMesh(CubeMesh.Object);
    }
    if (SphereMesh.Succeeded())
    {
        DungeonCoreVisual->SetStaticMesh(SphereMesh.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterialFinder(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")
    );
    static ConstructorHelpers::FObjectFinder<UMaterial> LitColorMaterialFinder(
        TEXT("/Engine/EngineDebugMaterials/LevelColorationLitMaterial.LevelColorationLitMaterial")
    );
    UMaterial* TintParentMaterial = BaseMaterialFinder.Succeeded() ? BaseMaterialFinder.Object : nullptr;
    if (LitColorMaterialFinder.Succeeded())
    {
        TintParentMaterial = LitColorMaterialFinder.Object;
    }

    if (TintParentMaterial != nullptr)
    {
        DungeonChunkVisuals::ApplyTintedMaterial(GridBaseInstances, TintParentMaterial, this, FLinearColor(0.18f, 0.17f, 0.20f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(
            GridLineInstances,
            TintParentMaterial,
            this,
            DungeonCore::GetGridLineColor(),
            2.0f
        );
        DungeonChunkVisuals::ApplyTintedMaterial(ChamberWallInstances, TintParentMaterial, this, FLinearColor(0.20f, 0.18f, 0.22f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(RockInstances, TintParentMaterial, this, FLinearColor(0.12f, 0.11f, 0.14f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(
            RoomInstances,
            TintParentMaterial,
            this,
            FLinearColor(0.78f, 0.15f, 1.0f),
            6.0f
        );
        DungeonChunkVisuals::ApplyTintedMaterial(PitCavityInstances, TintParentMaterial, this, FLinearColor(0.02f, 0.02f, 0.03f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(ClearedInstances, TintParentMaterial, this, FLinearColor(0.22f, 0.20f, 0.18f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(
            SurfaceEntryInstances,
            TintParentMaterial,
            this,
            FLinearColor(0.85f, 0.72f, 0.35f),
            1.5f
        );
        DungeonChunkVisuals::ApplyTintedMaterialToMesh(
            CoreChamberFloorVisual,
            TintParentMaterial,
            this,
            FLinearColor(0.55f, 0.12f, 0.82f),
            3.0f
        );
        DungeonChunkVisuals::ApplyTintedMaterialToMesh(
            DungeonCoreVisual,
            TintParentMaterial,
            this,
            FLinearColor(1.0f, 0.72f, 0.08f),
            8.0f
        );
    }

    ConfigureInstanceComponent(GridBaseInstances);
    ConfigureInstanceComponent(GridLineInstances);
    ConfigureInstanceComponent(ChamberWallInstances);
    ConfigureInstanceComponent(RockInstances);
    ConfigureInstanceComponent(PitCavityInstances);
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

void ADungeonChunkActor::BuildSurfaceGrid()
{
    GridLineInstances->ClearInstances();
    GridBaseInstances->ClearInstances();

    int32 MinBlockX = 0;
    int32 MaxBlockX = 0;
    int32 MinBlockY = 0;
    int32 MaxBlockY = 0;
    DungeonCore::GetStartingBuildAreaBlockBounds(MinBlockX, MaxBlockX, MinBlockY, MaxBlockY);

    const double MinWorldX = static_cast<double>(MinBlockX * DungeonCore::BlockSizeUnits);
    const double MinWorldY = static_cast<double>(MinBlockY * DungeonCore::BlockSizeUnits);
    const double SpanX = static_cast<double>((MaxBlockX - MinBlockX + 1) * DungeonCore::BlockSizeUnits);
    const double SpanY = static_cast<double>((MaxBlockY - MinBlockY + 1) * DungeonCore::BlockSizeUnits);
    const double CenterX = MinWorldX + (SpanX * 0.5);
    const double CenterY = MinWorldY + (SpanY * 0.5);
    const double GridZ = DungeonCore::GetMountainSurfaceWorldZ() + (DungeonCore::GridLineHeightUnits * 0.5);

    const double LineScaleX = DungeonCore::GridLineThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;
    const double LineScaleY = DungeonCore::GridLineThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;
    const double LineScaleZ = DungeonCore::GridLineHeightUnits / DungeonCore::DefaultEngineCubeSizeUnits;

    for (int32 GridX = MinBlockX; GridX <= MaxBlockX + 1; ++GridX)
    {
        const double WorldX = static_cast<double>(GridX * DungeonCore::BlockSizeUnits);
        GridLineInstances->AddInstance(MakeTileInstanceTransform(
            FVector(WorldX, CenterY, GridZ),
            FVector(LineScaleX, SpanY / DungeonCore::DefaultEngineCubeSizeUnits, LineScaleZ)
        ));
    }

    for (int32 GridY = MinBlockY; GridY <= MaxBlockY + 1; ++GridY)
    {
        const double WorldY = static_cast<double>(GridY * DungeonCore::BlockSizeUnits);
        GridLineInstances->AddInstance(MakeTileInstanceTransform(
            FVector(CenterX, WorldY, GridZ),
            FVector(SpanX / DungeonCore::DefaultEngineCubeSizeUnits, LineScaleY, LineScaleZ)
        ));
    }
}

void ADungeonChunkActor::BuildRockSurfaceCaps(const TArray<FTileData>& Tiles)
{
    GridBaseInstances->ClearInstances();

    const double CapThickness = 22.0;
    const double CapScaleZ = CapThickness / DungeonCore::DefaultEngineCubeSizeUnits;
    const double CapScaleXY = static_cast<double>(DungeonCore::BlockSizeUnits) / DungeonCore::DefaultEngineCubeSizeUnits;
    const double CapCenterZ = DungeonCore::GetMountainSurfaceWorldZ() - (CapThickness * 0.5);

    for (const FTileData& TileData : Tiles)
    {
        if (TileData.TileState != ETileState::SolidRock && TileData.TileState != ETileState::Wall)
        {
            continue;
        }

        const FVector ColumnXY = DungeonCore::GetBlockColumnWorldXY(TileData.BlockCoord);
        GridBaseInstances->AddInstance(MakeTileInstanceTransform(
            FVector(ColumnXY.X, ColumnXY.Y, CapCenterZ),
            FVector(CapScaleXY, CapScaleXY, CapScaleZ)
        ));
    }
}

void ADungeonChunkActor::UpdateSurfaceDungeonVisuals()
{
    int32 ChamberMinX = 0;
    int32 ChamberMaxX = 0;
    int32 ChamberMinY = 0;
    int32 ChamberMaxY = 0;
    DungeonCore::GetCoreChamberBlockBounds(ChamberMinX, ChamberMaxX, ChamberMinY, ChamberMaxY);

    const FVector ChamberSize = DungeonCore::GetInclusiveBlockBoundsWorldSize(
        ChamberMinX,
        ChamberMinY,
        ChamberMaxX,
        ChamberMaxY
    );
    const FVector ChamberCenter = DungeonCore::GetInclusiveBlockBoundsWorldCenter(
        ChamberMinX,
        ChamberMinY,
        ChamberMaxX,
        ChamberMaxY
    );

    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    const double FloorThickness = DungeonCore::CoreChamberFloorThicknessUnits;
    const double FloorCenterZ = FloorTopZ - (FloorThickness * 0.5);

    CoreChamberFloorVisual->SetRelativeLocation(FVector(ChamberCenter.X, ChamberCenter.Y, FloorCenterZ));
    CoreChamberFloorVisual->SetRelativeScale3D(FVector(
        static_cast<float>(ChamberSize.X / DungeonCore::DefaultEngineCubeSizeUnits),
        static_cast<float>(ChamberSize.Y / DungeonCore::DefaultEngineCubeSizeUnits),
        static_cast<float>(FloorThickness / DungeonCore::DefaultEngineCubeSizeUnits)
    ));
    CoreChamberFloorVisual->SetVisibility(true);

    const double CoreDiameter = DungeonCore::GetDungeonCoreVisualDiameterUnits();
    const float CoreScale = static_cast<float>(CoreDiameter / DungeonCore::DefaultEngineCubeSizeUnits);
    DungeonCoreVisual->SetRelativeLocation(FVector(
        ChamberCenter.X,
        ChamberCenter.Y,
        FloorTopZ + (CoreDiameter * 0.5)
    ));
    DungeonCoreVisual->SetRelativeScale3D(FVector(CoreScale, CoreScale, CoreScale));
    DungeonCoreVisual->SetVisibility(true);
}

void ADungeonChunkActor::BuildCoreChamberShell(const FBlockCoord& SurfaceEntryBlock)
{
    UpdateSurfaceDungeonVisuals();

    int32 ChamberMinX = 0;
    int32 ChamberMaxX = 0;
    int32 ChamberMinY = 0;
    int32 ChamberMaxY = 0;
    DungeonCore::GetCoreChamberBlockBounds(ChamberMinX, ChamberMaxX, ChamberMinY, ChamberMaxY);

    const FVector ChamberSize = DungeonCore::GetInclusiveBlockBoundsWorldSize(
        ChamberMinX,
        ChamberMinY,
        ChamberMaxX,
        ChamberMaxY
    );
    const FVector ChamberCenter = DungeonCore::GetInclusiveBlockBoundsWorldCenter(
        ChamberMinX,
        ChamberMinY,
        ChamberMaxX,
        ChamberMaxY
    );

    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    const double SurfaceZ = DungeonCore::GetMountainSurfaceWorldZ();
    const double WallHeight = FMath::Max(40.0, SurfaceZ - FloorTopZ);
    const double WallCenterZ = FloorTopZ + (WallHeight * 0.5);
    const double WallThickness = DungeonCore::CoreChamberWallThicknessUnits;
    const double WallScaleZ = WallHeight / DungeonCore::DefaultEngineCubeSizeUnits;
    const double WallScaleThin = WallThickness / DungeonCore::DefaultEngineCubeSizeUnits;
    const double WallScaleLongX = ChamberSize.X / DungeonCore::DefaultEngineCubeSizeUnits;
    const double WallScaleLongY = ChamberSize.Y / DungeonCore::DefaultEngineCubeSizeUnits;

    const double WestX = static_cast<double>(ChamberMinX * DungeonCore::BlockSizeUnits) + (WallThickness * 0.5);
    const double EastX = static_cast<double>((ChamberMaxX + 1) * DungeonCore::BlockSizeUnits) - (WallThickness * 0.5);
    const double SouthY = static_cast<double>(ChamberMinY * DungeonCore::BlockSizeUnits) + (WallThickness * 0.5);
    const double NorthY = static_cast<double>((ChamberMaxY + 1) * DungeonCore::BlockSizeUnits) - (WallThickness * 0.5);

    ChamberWallInstances->AddInstance(MakeTileInstanceTransform(
        FVector(WestX, ChamberCenter.Y, WallCenterZ),
        FVector(WallScaleThin, WallScaleLongY, WallScaleZ)
    ));
    ChamberWallInstances->AddInstance(MakeTileInstanceTransform(
        FVector(ChamberCenter.X, SouthY, WallCenterZ),
        FVector(WallScaleLongX, WallScaleThin, WallScaleZ)
    ));
    ChamberWallInstances->AddInstance(MakeTileInstanceTransform(
        FVector(ChamberCenter.X, NorthY, WallCenterZ),
        FVector(WallScaleLongX, WallScaleThin, WallScaleZ)
    ));

    const bool bEntryOnEast = SurfaceEntryBlock.X == ChamberMaxX;
    if (!bEntryOnEast)
    {
        ChamberWallInstances->AddInstance(MakeTileInstanceTransform(
            FVector(EastX, ChamberCenter.Y, WallCenterZ),
            FVector(WallScaleThin, WallScaleLongY, WallScaleZ)
        ));
        return;
    }

    // East wall with an exact one-tile gap at the surface entry: segments south and north of the entry tile.
    const double ChamberSouthWorldY = static_cast<double>(ChamberMinY * DungeonCore::BlockSizeUnits);
    const double ChamberNorthWorldY = static_cast<double>((ChamberMaxY + 1) * DungeonCore::BlockSizeUnits);
    const double EntrySouthWorldY = static_cast<double>(SurfaceEntryBlock.Y * DungeonCore::BlockSizeUnits);
    const double EntryNorthWorldY = EntrySouthWorldY + static_cast<double>(DungeonCore::BlockSizeUnits);
    AddEastWallSegment(EastX, ChamberSouthWorldY, EntrySouthWorldY, WallCenterZ, WallScaleThin, WallScaleZ);
    AddEastWallSegment(EastX, EntryNorthWorldY, ChamberNorthWorldY, WallCenterZ, WallScaleThin, WallScaleZ);
}

void ADungeonChunkActor::AddEastWallSegment(
    const double WallX,
    const double FromWorldY,
    const double ToWorldY,
    const double WallCenterZ,
    const double WallScaleThin,
    const double WallScaleZ
)
{
    const double SegmentLength = ToWorldY - FromWorldY;
    if (SegmentLength <= 0.0)
    {
        return;
    }

    ChamberWallInstances->AddInstance(MakeTileInstanceTransform(
        FVector(WallX, FromWorldY + (SegmentLength * 0.5), WallCenterZ),
        FVector(WallScaleThin, SegmentLength / DungeonCore::DefaultEngineCubeSizeUnits, WallScaleZ)
    ));
}

void ADungeonChunkActor::RefreshFromSimulation(
    UDungeonGridState* GridState,
    const TArray<FUnitInstanceData>& ActiveAdventurers,
    const TArray<FUnitInstanceData>& ActiveDefenders
)
{
    RockInstances->ClearInstances();
    ChamberWallInstances->ClearInstances();
    PitCavityInstances->ClearInstances();
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

    FBlockCoord SurfaceEntryBlock(
        DungeonCore::StartingCoreChamberWidthBlocks - 1,
        DungeonCore::GetStartingCoreBlockCoord().Y,
        0
    );

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    BuildSurfaceGrid();
    BuildRockSurfaceCaps(Tiles);

    for (const FTileData& TileData : Tiles)
    {
        if (TileData.TileState == ETileState::SurfaceEntry)
        {
            SurfaceEntryBlock = TileData.BlockCoord;
        }

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
            if (TileData.RoomTypeId == ERoomTypeId::CoreChamber)
            {
                break;
            }

            RoomInstances->AddInstance(MakeTileInstanceTransform(
                DungeonCore::GetCarvedFloorVisualWorldCenter(TileData.BlockCoord, CarvedFloorScale.Z),
                CarvedFloorScale
            ));
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

    BuildCoreChamberShell(SurfaceEntryBlock);

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Chunk visuals: %d sim tiles | surface grid + core chamber shell (floor, walls, core pillar)."),
        Tiles.Num()
    );

    GridLineInstances->MarkRenderStateDirty();
    GridBaseInstances->MarkRenderStateDirty();
    ChamberWallInstances->MarkRenderStateDirty();
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
