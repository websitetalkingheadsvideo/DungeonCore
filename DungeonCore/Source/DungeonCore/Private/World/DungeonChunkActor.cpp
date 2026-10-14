#include "World/DungeonChunkActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Materials/MaterialInterface.h"
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

    /** Library assets are required content; a missing one is a migration error, not something to paper over. */
    void AssignLibraryMesh(
        UStaticMeshComponent* MeshComponent,
        const ConstructorHelpers::FObjectFinder<UStaticMesh>& Finder,
        const TCHAR* Purpose
    )
    {
        if (!Finder.Succeeded())
        {
            UE_LOG(LogTemp, Error, TEXT("Dungeon library mesh missing for %s (see library.md migration)."), Purpose);
            return;
        }
        MeshComponent->SetStaticMesh(Finder.Object);
    }

    void AssignLibraryMaterial(
        UMeshComponent* MeshComponent,
        const ConstructorHelpers::FObjectFinder<UMaterialInterface>& Finder,
        const TCHAR* Purpose
    )
    {
        if (!Finder.Succeeded())
        {
            UE_LOG(LogTemp, Error, TEXT("Dungeon library material missing for %s (see library.md migration)."), Purpose);
            return;
        }
        MeshComponent->SetMaterial(0, Finder.Object);
    }

    /** Retint Crystal_Cave's MI_GlowCrystalls (Out_Color / Inner_Color / Deep_Paralax_Color / Inner_Glow) for the core. */
    void ApplyCoreCrystalLook(
        UStaticMeshComponent* MeshComponent,
        const ConstructorHelpers::FObjectFinder<UMaterialInterface>& Finder,
        UObject* Outer
    )
    {
        if (!Finder.Succeeded())
        {
            UE_LOG(LogTemp, Error, TEXT("Dungeon library material missing for core crystal glow (see library.md migration)."));
            return;
        }
        UMaterialInstanceDynamic* CrystalMaterial = UMaterialInstanceDynamic::Create(Finder.Object, Outer);
        CrystalMaterial->SetVectorParameterValue(TEXT("Out_Color"), DungeonCore::GetDungeonCoreCrystalShellColor());
        CrystalMaterial->SetVectorParameterValue(TEXT("Inner_Color"), DungeonCore::GetDungeonCoreCrystalInnerColor());
        CrystalMaterial->SetVectorParameterValue(TEXT("Deep_Paralax_Color"), DungeonCore::GetDungeonCoreCrystalInnerColor());
        CrystalMaterial->SetScalarParameterValue(TEXT("Inner_Glow"), DungeonCore::GetDungeonCoreCrystalInnerGlow());
        MeshComponent->SetMaterial(0, CrystalMaterial);
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

    DungeonCorePivot = CreateDefaultSubobject<USceneComponent>(TEXT("DungeonCorePivot"));
    DungeonCorePivot->SetupAttachment(SceneRoot);
    DungeonCorePivot->SetMobility(EComponentMobility::Movable);

    DungeonCoreCrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DungeonCoreCrystalMesh"));
    DungeonCoreCrystalMesh->SetupAttachment(DungeonCorePivot);
    DungeonCoreCrystalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DungeonCoreCrystalMesh->SetMobility(EComponentMobility::Movable);
    // The glow light sits inside the crystal; the crystal must not shadow its own light.
    DungeonCoreCrystalMesh->SetCastShadow(false);

    DungeonCoreGlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DungeonCoreGlowLight"));
    DungeonCoreGlowLight->SetupAttachment(DungeonCorePivot);
    DungeonCoreGlowLight->SetMobility(EComponentMobility::Movable);
    DungeonCoreGlowLight->SetIntensityUnits(ELightUnits::Candelas);
    DungeonCoreGlowLight->SetIntensity(DungeonCore::GetDungeonCoreGlowIntensityCandelas());
    DungeonCoreGlowLight->SetLightColor(DungeonCore::GetDungeonCoreGlowColor());
    DungeonCoreGlowLight->SetAttenuationRadius(DungeonCore::GetDungeonCoreGlowAttenuationRadiusUnits());
    DungeonCoreGlowLight->SetCastShadows(true);

    DungeonCoreSpin = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("DungeonCoreSpin"));
    DungeonCoreSpin->RotationRate = FRotator(0.0f, DungeonCore::GetDungeonCoreSpinDegreesPerSecond(), 0.0f);
    DungeonCoreSpin->SetUpdatedComponent(DungeonCorePivot);

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

    // Migrated library assets (see library.md): Crystal_Cave for the core + rock, Leartes Fantasy Dungeon for stone.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CrystalMeshFinder(
        TEXT("/Game/Crystal_Cave/Meshes/SM_Crystal_D_01.SM_Crystal_D_01")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> CrystalGlowMaterialFinder(
        TEXT("/Game/Crystal_Cave/Materials/MI_GlowCrystalls.MI_GlowCrystalls")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> RockMaterialFinder(
        TEXT("/Game/Crystal_Cave/Materials/MI_Rock_A.MI_Rock_A")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneFloorMaterialFinder(
        TEXT("/Game/FantasyDungeon/Materials/MI_FloorTiles_Clean_V01.MI_FloorTiles_Clean_V01")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> StoneWallMaterialFinder(
        TEXT("/Game/FantasyDungeon/Materials/MI_StoneWall_V03.MI_StoneWall_V03")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DirtFloorMaterialFinder(
        TEXT("/Game/FantasyDungeon/Materials/MI_Dirt_Floor_v01.MI_Dirt_Floor_v01")
    );
    DungeonChunkVisuals::AssignLibraryMesh(DungeonCoreCrystalMesh, CrystalMeshFinder, TEXT("core crystal mesh"));
    DungeonChunkVisuals::ApplyCoreCrystalLook(DungeonCoreCrystalMesh, CrystalGlowMaterialFinder, this);
    DungeonChunkVisuals::AssignLibraryMaterial(RockInstances, RockMaterialFinder, TEXT("rock blocks"));
    DungeonChunkVisuals::AssignLibraryMaterial(GridBaseInstances, RockMaterialFinder, TEXT("rock surface caps"));
    DungeonChunkVisuals::AssignLibraryMaterial(CoreChamberFloorVisual, StoneFloorMaterialFinder, TEXT("chamber stone floor"));
    DungeonChunkVisuals::AssignLibraryMaterial(ChamberWallInstances, StoneWallMaterialFinder, TEXT("chamber stone walls"));
    DungeonChunkVisuals::AssignLibraryMaterial(ClearedInstances, DirtFloorMaterialFinder, TEXT("dug dirt floor"));

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
        DungeonChunkVisuals::ApplyTintedMaterial(
            GridLineInstances,
            TintParentMaterial,
            this,
            DungeonCore::GetGridLineColor(),
            2.0f
        );
        DungeonChunkVisuals::ApplyTintedMaterial(
            RoomInstances,
            TintParentMaterial,
            this,
            FLinearColor(0.78f, 0.15f, 1.0f),
            6.0f
        );
        DungeonChunkVisuals::ApplyTintedMaterial(PitCavityInstances, TintParentMaterial, this, FLinearColor(0.02f, 0.02f, 0.03f), 0.0f);
        DungeonChunkVisuals::ApplyTintedMaterial(
            SurfaceEntryInstances,
            TintParentMaterial,
            this,
            FLinearColor(0.85f, 0.72f, 0.35f),
            1.5f
        );
    }

    LayoutDungeonCoreCrystal();

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

TSet<FIntPoint> ADungeonChunkActor::CollectRockColumns(const TArray<FTileData>& Tiles)
{
    TSet<FIntPoint> RockColumns;
    for (const FTileData& TileData : Tiles)
    {
        if (TileData.BlockCoord.Z == 0
            && (TileData.TileState == ETileState::SolidRock || TileData.TileState == ETileState::Wall))
        {
            RockColumns.Add(FIntPoint(TileData.BlockCoord.X, TileData.BlockCoord.Y));
        }
    }
    return RockColumns;
}

void ADungeonChunkActor::BuildSurfaceGrid(const TSet<FIntPoint>& RockColumns)
{
    GridLineInstances->ClearInstances();

    // Grid lines live on the mountain surface, so an edge is only drawn where at least one of the two
    // tiles it separates still has rock on top. Edges between two carved tiles (open pit) get nothing.
    int32 MinBlockX = 0;
    int32 MaxBlockX = 0;
    int32 MinBlockY = 0;
    int32 MaxBlockY = 0;
    DungeonCore::GetStartingBuildAreaBlockBounds(MinBlockX, MaxBlockX, MinBlockY, MaxBlockY);

    const double GridZ = DungeonCore::GetMountainSurfaceWorldZ() + (DungeonCore::GridLineHeightUnits * 0.5);
    const double BlockSize = static_cast<double>(DungeonCore::BlockSizeUnits);
    const double LineScaleThin = DungeonCore::GridLineThicknessUnits / DungeonCore::DefaultEngineCubeSizeUnits;
    const double LineScaleZ = DungeonCore::GridLineHeightUnits / DungeonCore::DefaultEngineCubeSizeUnits;

    const auto IsRockColumn = [&RockColumns](const int32 BlockX, const int32 BlockY) -> bool
    {
        return RockColumns.Contains(FIntPoint(BlockX, BlockY));
    };

    // Lines of constant X: edge between column (GridX - 1) and column GridX, merged along Y.
    for (int32 GridX = MinBlockX; GridX <= MaxBlockX + 1; ++GridX)
    {
        const double WorldX = static_cast<double>(GridX) * BlockSize;
        int32 RunStart = MinBlockY;
        bool bInRun = false;
        for (int32 GridY = MinBlockY; GridY <= MaxBlockY + 1; ++GridY)
        {
            const bool bDraw = GridY <= MaxBlockY && (IsRockColumn(GridX - 1, GridY) || IsRockColumn(GridX, GridY));
            if (bDraw && !bInRun)
            {
                RunStart = GridY;
                bInRun = true;
            }
            else if (!bDraw && bInRun)
            {
                const double FromY = static_cast<double>(RunStart) * BlockSize;
                const double ToY = static_cast<double>(GridY) * BlockSize;
                GridLineInstances->AddInstance(MakeTileInstanceTransform(
                    FVector(WorldX, (FromY + ToY) * 0.5, GridZ),
                    FVector(LineScaleThin, (ToY - FromY) / DungeonCore::DefaultEngineCubeSizeUnits, LineScaleZ)
                ));
                bInRun = false;
            }
        }
    }

    // Lines of constant Y: edge between row (GridY - 1) and row GridY, merged along X.
    for (int32 GridY = MinBlockY; GridY <= MaxBlockY + 1; ++GridY)
    {
        const double WorldY = static_cast<double>(GridY) * BlockSize;
        int32 RunStart = MinBlockX;
        bool bInRun = false;
        for (int32 GridX = MinBlockX; GridX <= MaxBlockX + 1; ++GridX)
        {
            const bool bDraw = GridX <= MaxBlockX && (IsRockColumn(GridX, GridY - 1) || IsRockColumn(GridX, GridY));
            if (bDraw && !bInRun)
            {
                RunStart = GridX;
                bInRun = true;
            }
            else if (!bDraw && bInRun)
            {
                const double FromX = static_cast<double>(RunStart) * BlockSize;
                const double ToX = static_cast<double>(GridX) * BlockSize;
                GridLineInstances->AddInstance(MakeTileInstanceTransform(
                    FVector((FromX + ToX) * 0.5, WorldY, GridZ),
                    FVector((ToX - FromX) / DungeonCore::DefaultEngineCubeSizeUnits, LineScaleThin, LineScaleZ)
                ));
                bInRun = false;
            }
        }
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

    DungeonCorePivot->SetRelativeLocation(FVector(
        ChamberCenter.X,
        ChamberCenter.Y,
        FloorTopZ + DungeonCore::GetDungeonCoreCrystalHoverUnits()
    ));
}

void ADungeonChunkActor::LayoutDungeonCoreCrystal()
{
    const UStaticMesh* CrystalMesh = DungeonCoreCrystalMesh->GetStaticMesh();
    if (CrystalMesh == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Core crystal mesh not assigned; cannot lay out the dungeon core."));
        return;
    }

    const double TotalHeight = DungeonCore::GetDungeonCoreCrystalHeightUnits();
    const FBoxSphereBounds MeshBounds = CrystalMesh->GetBounds();
    const double MeshHeight = MeshBounds.BoxExtent.Z * 2.0;
    if (MeshHeight <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("Core crystal mesh has no vertical extent."));
        return;
    }

    // Uniform scale to the target height, roll so the point faces up, then seat the lowest point of the
    // rotated bounds on the pivot. The rotation is applied in local space, so the bounds centre is rotated too.
    const double Scale = TotalHeight / MeshHeight;
    const FRotator MeshRotation = DungeonCore::GetDungeonCoreCrystalMeshRotation();
    const FVector RotatedOrigin = MeshRotation.RotateVector(MeshBounds.Origin) * Scale;
    const double RotatedBottomZ = RotatedOrigin.Z - MeshBounds.BoxExtent.Z * Scale;
    DungeonCoreCrystalMesh->SetRelativeScale3D(FVector(Scale, Scale, Scale));
    DungeonCoreCrystalMesh->SetRelativeRotation(MeshRotation);
    DungeonCoreCrystalMesh->SetRelativeLocation(FVector(-RotatedOrigin.X, -RotatedOrigin.Y, -RotatedBottomZ));

    DungeonCoreGlowLight->SetRelativeLocation(FVector(0.0, 0.0, TotalHeight * 0.45));
}

void ADungeonChunkActor::BuildCoreChamberShell(const TSet<FIntPoint>& RockColumns)
{
    UpdateSurfaceDungeonVisuals();

    int32 ChamberMinX = 0;
    int32 ChamberMaxX = 0;
    int32 ChamberMinY = 0;
    int32 ChamberMaxY = 0;
    DungeonCore::GetCoreChamberBlockBounds(ChamberMinX, ChamberMaxX, ChamberMinY, ChamberMaxY);

    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    const double SurfaceZ = DungeonCore::GetMountainSurfaceWorldZ();
    const double WallHeight = FMath::Max(40.0, SurfaceZ - FloorTopZ);
    const double WallCenterZ = FloorTopZ + (WallHeight * 0.5);
    const double WallThickness = DungeonCore::CoreChamberWallThicknessUnits;
    const double WallScaleZ = WallHeight / DungeonCore::DefaultEngineCubeSizeUnits;
    const double WallScaleThin = WallThickness / DungeonCore::DefaultEngineCubeSizeUnits;

    const double WestX = static_cast<double>(ChamberMinX * DungeonCore::BlockSizeUnits) + (WallThickness * 0.5);
    const double EastX = static_cast<double>((ChamberMaxX + 1) * DungeonCore::BlockSizeUnits) - (WallThickness * 0.5);
    const double SouthY = static_cast<double>(ChamberMinY * DungeonCore::BlockSizeUnits) + (WallThickness * 0.5);
    const double NorthY = static_cast<double>((ChamberMaxY + 1) * DungeonCore::BlockSizeUnits) - (WallThickness * 0.5);

    const auto IsRock = [&RockColumns](const int32 BlockX, const int32 BlockY) -> bool
    {
        return RockColumns.Contains(FIntPoint(BlockX, BlockY));
    };

    // A wall tile stays closed while the rock directly outside it is intact; digging that rock opens the gap.
    AddChamberWallSide(true, WestX, ChamberMinY, ChamberMaxY,
        [&IsRock, ChamberMinX](const int32 BlockY) { return IsRock(ChamberMinX - 1, BlockY); },
        WallCenterZ, WallScaleThin, WallScaleZ);
    AddChamberWallSide(true, EastX, ChamberMinY, ChamberMaxY,
        [&IsRock, ChamberMaxX](const int32 BlockY) { return IsRock(ChamberMaxX + 1, BlockY); },
        WallCenterZ, WallScaleThin, WallScaleZ);
    AddChamberWallSide(false, SouthY, ChamberMinX, ChamberMaxX,
        [&IsRock, ChamberMinY](const int32 BlockX) { return IsRock(BlockX, ChamberMinY - 1); },
        WallCenterZ, WallScaleThin, WallScaleZ);
    AddChamberWallSide(false, NorthY, ChamberMinX, ChamberMaxX,
        [&IsRock, ChamberMaxY](const int32 BlockX) { return IsRock(BlockX, ChamberMaxY + 1); },
        WallCenterZ, WallScaleThin, WallScaleZ);
}

void ADungeonChunkActor::AddChamberWallSide(
    const bool bWallRunsAlongY,
    const double FixedWorldCoord,
    const int32 MinIndex,
    const int32 MaxIndex,
    const TFunctionRef<bool(int32)>& IsClosedAt,
    const double WallCenterZ,
    const double WallScaleThin,
    const double WallScaleZ
)
{
    const double BlockSize = static_cast<double>(DungeonCore::BlockSizeUnits);
    int32 RunStart = MinIndex;
    bool bInRun = false;

    for (int32 Index = MinIndex; Index <= MaxIndex + 1; ++Index)
    {
        const bool bClosed = Index <= MaxIndex && IsClosedAt(Index);
        if (bClosed && !bInRun)
        {
            RunStart = Index;
            bInRun = true;
            continue;
        }
        if (bClosed || !bInRun)
        {
            continue;
        }

        const double From = static_cast<double>(RunStart) * BlockSize;
        const double To = static_cast<double>(Index) * BlockSize;
        const double Along = (From + To) * 0.5;
        const double LongScale = (To - From) / DungeonCore::DefaultEngineCubeSizeUnits;
        const FVector Center = bWallRunsAlongY
            ? FVector(FixedWorldCoord, Along, WallCenterZ)
            : FVector(Along, FixedWorldCoord, WallCenterZ);
        const FVector Scale = bWallRunsAlongY
            ? FVector(WallScaleThin, LongScale, WallScaleZ)
            : FVector(LongScale, WallScaleThin, WallScaleZ);
        ChamberWallInstances->AddInstance(MakeTileInstanceTransform(Center, Scale));
        bInRun = false;
    }
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

    const TArray<FTileData> Tiles = GridState->GetAllTiles();
    const TSet<FIntPoint> RockColumns = CollectRockColumns(Tiles);
    BuildSurfaceGrid(RockColumns);
    BuildRockSurfaceCaps(Tiles);

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

    BuildCoreChamberShell(RockColumns);

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
