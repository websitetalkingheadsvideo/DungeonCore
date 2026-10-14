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
