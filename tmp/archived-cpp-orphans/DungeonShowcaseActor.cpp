#include "World/DungeonShowcaseActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/DungeonCreatureFamilyRow.h"
#include "Data/DungeonThemeRow.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace DungeonShowcaseLayout
{
    /** Walls stand the full dig depth, like the chunk actor's chamber shell. */
    inline double WallHeightUnits()
    {
        return FMath::Max(40.0, DungeonCore::GetMountainSurfaceWorldZ() - DungeonCore::GetCoreChamberSurfaceVisualFloorZ());
    }

    /** Accents sit this far in from the two walls that meet at their corner. */
    constexpr double AccentCornerInsetUnits = 40.0;

    inline float ThemeFillIntensityCandelas()
    {
        return 60.0f;
    }

    inline float CreatureKeyIntensityCandelas()
    {
        return 120.0f;
    }

    FTransform MakeInstanceTransform(const FVector& WorldCenter, const FVector& InstanceScale)
    {
        FTransform InstanceTransform;
        InstanceTransform.SetLocation(WorldCenter);
        InstanceTransform.SetRotation(FQuat::Identity);
        InstanceTransform.SetScale3D(InstanceScale);
        return InstanceTransform;
    }

    void ConfigureInstances(UInstancedStaticMeshComponent* InstanceComponent)
    {
        InstanceComponent->SetMobility(EComponentMobility::Movable);
        InstanceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        InstanceComponent->SetCastShadow(true);
    }
}

ADungeonShowcaseActor::ADungeonShowcaseActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SetRootComponent(SceneRoot);

    FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
    FloorInstances->SetupAttachment(SceneRoot);
    DungeonShowcaseLayout::ConfigureInstances(FloorInstances);

    WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
    WallInstances->SetupAttachment(SceneRoot);
    DungeonShowcaseLayout::ConfigureInstances(WallInstances);

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FName ComponentName(*FString::Printf(TEXT("AccentMesh_%d"), Index));
        UStaticMeshComponent* Accent = CreateDefaultSubobject<UStaticMeshComponent>(ComponentName);
        Accent->SetupAttachment(SceneRoot);
        Accent->SetMobility(EComponentMobility::Movable);
        Accent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AccentMeshes.Add(Accent);
    }

    CorePivot = CreateDefaultSubobject<USceneComponent>(TEXT("CorePivot"));
    CorePivot->SetupAttachment(SceneRoot);
    CorePivot->SetMobility(EComponentMobility::Movable);

    CoreCrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreCrystalMesh"));
    CoreCrystalMesh->SetupAttachment(CorePivot);
    CoreCrystalMesh->SetMobility(EComponentMobility::Movable);
    CoreCrystalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CoreCrystalMesh->SetCastShadow(false);

    CoreGlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreGlowLight"));
    CoreGlowLight->SetupAttachment(CorePivot);
    CoreGlowLight->SetMobility(EComponentMobility::Movable);
    CoreGlowLight->SetIntensityUnits(ELightUnits::Candelas);
    CoreGlowLight->SetIntensity(DungeonCore::GetDungeonCoreGlowIntensityCandelas());
    CoreGlowLight->SetLightColor(DungeonCore::GetDungeonCoreGlowColor());
    CoreGlowLight->SetAttenuationRadius(DungeonCore::GetDungeonCoreGlowAttenuationRadiusUnits());
    CoreGlowLight->SetCastShadows(true);

    CoreSpin = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("CoreSpin"));
    CoreSpin->RotationRate = FRotator(0.0f, DungeonCore::GetDungeonCoreSpinDegreesPerSecond(), 0.0f);
    CoreSpin->SetUpdatedComponent(CorePivot);

    CreatureSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CreatureSkeletalMesh"));
    CreatureSkeletalMesh->SetupAttachment(SceneRoot);
    CreatureSkeletalMesh->SetMobility(EComponentMobility::Movable);
    CreatureSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CreatureSkeletalMesh->SetVisibility(false);

    CreatureStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CreatureStaticMesh"));
    CreatureStaticMesh->SetupAttachment(SceneRoot);
    CreatureStaticMesh->SetMobility(EComponentMobility::Movable);
    CreatureStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CreatureStaticMesh->SetVisibility(false);

    ThemeFillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ThemeFillLight"));
    ThemeFillLight->SetupAttachment(SceneRoot);
    ThemeFillLight->SetMobility(EComponentMobility::Movable);
    ThemeFillLight->SetIntensityUnits(ELightUnits::Candelas);
    ThemeFillLight->SetIntensity(DungeonShowcaseLayout::ThemeFillIntensityCandelas());
    ThemeFillLight->SetAttenuationRadius(1600.0f);
    ThemeFillLight->SetCastShadows(false);

    CreatureKeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("CreatureKeyLight"));
    CreatureKeyLight->SetupAttachment(SceneRoot);
    CreatureKeyLight->SetMobility(EComponentMobility::Movable);
    CreatureKeyLight->SetIntensityUnits(ELightUnits::Candelas);
    CreatureKeyLight->SetIntensity(DungeonShowcaseLayout::CreatureKeyIntensityCandelas());
    CreatureKeyLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.88f, 1.0f));
    CreatureKeyLight->SetAttenuationRadius(1400.0f);
    CreatureKeyLight->SetInnerConeAngle(18.0f);
    CreatureKeyLight->SetOuterConeAngle(34.0f);
    CreatureKeyLight->SetCastShadows(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        FloorInstances->SetStaticMesh(CubeMesh.Object);
        WallInstances->SetStaticMesh(CubeMesh.Object);
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CrystalMeshFinder(
        TEXT("/Game/Crystal_Cave/Meshes/SM_Crystal_D_01.SM_Crystal_D_01")
    );
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> CrystalGlowMaterialFinder(
        TEXT("/Game/Crystal_Cave/Materials/MI_GlowCrystalls.MI_GlowCrystalls")
    );
    if (CrystalMeshFinder.Succeeded())
    {
        CoreCrystalMesh->SetStaticMesh(CrystalMeshFinder.Object);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: library mesh missing for core crystal (see library.md migration)."));
    }
    if (CrystalGlowMaterialFinder.Succeeded())
    {
        UMaterialInstanceDynamic* CrystalMaterial = UMaterialInstanceDynamic::Create(CrystalGlowMaterialFinder.Object, this);
        CrystalMaterial->SetVectorParameterValue(TEXT("Out_Color"), DungeonCore::GetDungeonCoreCrystalShellColor());
        CrystalMaterial->SetVectorParameterValue(TEXT("Inner_Color"), DungeonCore::GetDungeonCoreCrystalInnerColor());
        CrystalMaterial->SetVectorParameterValue(TEXT("Deep_Paralax_Color"), DungeonCore::GetDungeonCoreCrystalInnerColor());
        CrystalMaterial->SetScalarParameterValue(TEXT("Inner_Glow"), DungeonCore::GetDungeonCoreCrystalInnerGlow());
        CoreCrystalMesh->SetMaterial(0, CrystalMaterial);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: library material missing for core crystal glow (see library.md migration)."));
    }

    BuildChamberGeometry();
    LayoutCoreCrystal();
}

void ADungeonShowcaseActor::BuildChamberGeometry()
{
    int32 MinX = 0;
    int32 MaxX = 0;
    int32 MinY = 0;
    int32 MaxY = 0;
    DungeonCore::GetCoreChamberBlockBounds(MinX, MaxX, MinY, MaxY);

    const double BlockSize = static_cast<double>(DungeonCore::BlockSizeUnits);
    const double CubeSize = DungeonCore::DefaultEngineCubeSizeUnits;
    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    const double FloorThickness = DungeonCore::CoreChamberFloorThicknessUnits;
    const double WallThickness = DungeonCore::CoreChamberWallThicknessUnits;
    const double WallHeight = DungeonShowcaseLayout::WallHeightUnits();

    // One floor slab per block so a tiled material repeats once per gameplay block.
    FloorInstances->ClearInstances();
    const FVector FloorScale(BlockSize / CubeSize, BlockSize / CubeSize, FloorThickness / CubeSize);
    for (int32 X = MinX; X <= MaxX; ++X)
    {
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            const FVector ColumnXY = DungeonCore::GetBlockColumnWorldXY(FBlockCoord(X, Y, 0));
            FloorInstances->AddInstance(DungeonShowcaseLayout::MakeInstanceTransform(
                FVector(ColumnXY.X, ColumnXY.Y, FloorTopZ - FloorThickness * 0.5),
                FloorScale
            ));
        }
    }

    // Walls on three sides; the camera-facing (min X) side stays open like a dollhouse cutaway.
    WallInstances->ClearInstances();
    const double WallCenterZ = FloorTopZ + WallHeight * 0.5;
    const FVector WallScaleAlongY(WallThickness / CubeSize, BlockSize / CubeSize, WallHeight / CubeSize);
    const FVector WallScaleAlongX(BlockSize / CubeSize, WallThickness / CubeSize, WallHeight / CubeSize);
    const double EastX = static_cast<double>(MaxX + 1) * BlockSize - WallThickness * 0.5;
    const double SouthY = static_cast<double>(MinY) * BlockSize + WallThickness * 0.5;
    const double NorthY = static_cast<double>(MaxY + 1) * BlockSize - WallThickness * 0.5;
    for (int32 Y = MinY; Y <= MaxY; ++Y)
    {
        const double Along = (static_cast<double>(Y) + 0.5) * BlockSize;
        WallInstances->AddInstance(DungeonShowcaseLayout::MakeInstanceTransform(FVector(EastX, Along, WallCenterZ), WallScaleAlongY));
    }
    for (int32 X = MinX; X <= MaxX; ++X)
    {
        const double Along = (static_cast<double>(X) + 0.5) * BlockSize;
        WallInstances->AddInstance(DungeonShowcaseLayout::MakeInstanceTransform(FVector(Along, SouthY, WallCenterZ), WallScaleAlongX));
        WallInstances->AddInstance(DungeonShowcaseLayout::MakeInstanceTransform(FVector(Along, NorthY, WallCenterZ), WallScaleAlongX));
    }

    const FVector ChamberCenter = DungeonCore::GetInclusiveBlockBoundsWorldCenter(MinX, MinY, MaxX, MaxY);
    CorePivot->SetRelativeLocation(FVector(
        ChamberCenter.X,
        ChamberCenter.Y,
        FloorTopZ + DungeonCore::GetDungeonCoreCrystalHoverUnits()
    ));

    ThemeFillLight->SetRelativeLocation(FVector(ChamberCenter.X, ChamberCenter.Y, FloorTopZ + WallHeight + 300.0));

    // Key light comes from the open side, above and slightly behind where the camera sits.
    const FVector CreatureXY = DungeonCore::GetBlockColumnWorldXY(DungeonCore::GetShowcaseCreatureBlockCoord());
    const FVector KeyLocation(CreatureXY.X - 700.0, CreatureXY.Y - 250.0, FloorTopZ + 650.0);
    CreatureKeyLight->SetRelativeLocation(KeyLocation);
    CreatureKeyLight->SetRelativeRotation(
        (FVector(CreatureXY.X, CreatureXY.Y, FloorTopZ + 90.0) - KeyLocation).Rotation()
    );
}

void ADungeonShowcaseActor::LayoutCoreCrystal()
{
    const UStaticMesh* CrystalMesh = CoreCrystalMesh->GetStaticMesh();
    if (CrystalMesh == nullptr)
    {
        return;
    }

    const double TotalHeight = DungeonCore::GetDungeonCoreCrystalHeightUnits();
    const FBoxSphereBounds MeshBounds = CrystalMesh->GetBounds();
    const double MeshHeight = MeshBounds.BoxExtent.Z * 2.0;
    if (MeshHeight <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: core crystal mesh has no vertical extent."));
        return;
    }

    const double Scale = TotalHeight / MeshHeight;
    const FRotator MeshRotation = DungeonCore::GetDungeonCoreCrystalMeshRotation();
    const FVector RotatedOrigin = MeshRotation.RotateVector(MeshBounds.Origin) * Scale;
    const double RotatedBottomZ = RotatedOrigin.Z - MeshBounds.BoxExtent.Z * Scale;
    CoreCrystalMesh->SetRelativeScale3D(FVector(Scale, Scale, Scale));
    CoreCrystalMesh->SetRelativeRotation(MeshRotation);
    CoreCrystalMesh->SetRelativeLocation(FVector(-RotatedOrigin.X, -RotatedOrigin.Y, -RotatedBottomZ));
    CoreGlowLight->SetRelativeLocation(FVector(0.0, 0.0, TotalHeight * 0.45));
}

void ADungeonShowcaseActor::ApplyTheme(const FDungeonThemeRow& ThemeRow)
{
    UMaterialInterface* FloorMaterial = ThemeRow.FloorMaterial.LoadSynchronous();
    if (FloorMaterial == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: theme '%s' floor material failed to load (%s)."), *ThemeRow.DisplayName.ToString(), *ThemeRow.FloorMaterial.ToString());
    }
    else
    {
        FloorInstances->SetMaterial(0, FloorMaterial);
    }

    UMaterialInterface* WallMaterial = ThemeRow.WallMaterial.LoadSynchronous();
    if (WallMaterial == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: theme '%s' wall material failed to load (%s)."), *ThemeRow.DisplayName.ToString(), *ThemeRow.WallMaterial.ToString());
    }
    else
    {
        WallInstances->SetMaterial(0, WallMaterial);
    }

    UStaticMesh* AccentMesh = ThemeRow.AccentMesh.LoadSynchronous();
    if (AccentMesh == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: theme '%s' accent mesh failed to load (%s)."), *ThemeRow.DisplayName.ToString(), *ThemeRow.AccentMesh.ToString());
    }

    int32 MinX = 0;
    int32 MaxX = 0;
    int32 MinY = 0;
    int32 MaxY = 0;
    DungeonCore::GetCoreChamberBlockBounds(MinX, MaxX, MinY, MaxY);
    const double BlockSize = static_cast<double>(DungeonCore::BlockSizeUnits);
    const double Inset = DungeonCore::CoreChamberWallThicknessUnits + DungeonShowcaseLayout::AccentCornerInsetUnits;
    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    const double WestX = static_cast<double>(MinX) * BlockSize + Inset;
    const double EastX = static_cast<double>(MaxX + 1) * BlockSize - Inset;
    const double SouthY = static_cast<double>(MinY) * BlockSize + Inset;
    const double NorthY = static_cast<double>(MaxY + 1) * BlockSize - Inset;
    const FVector2D Corners[4] = {
        FVector2D(WestX, SouthY),
        FVector2D(WestX, NorthY),
        FVector2D(EastX, SouthY),
        FVector2D(EastX, NorthY)
    };

    for (int32 Index = 0; Index < AccentMeshes.Num(); ++Index)
    {
        UStaticMeshComponent* Accent = AccentMeshes[Index];
        Accent->SetStaticMesh(AccentMesh);
        Accent->SetVisibility(AccentMesh != nullptr);
        if (AccentMesh == nullptr)
        {
            continue;
        }
        // Corner accents face into the room: rotate by quadrant so the same mesh reads as four matching pieces.
        const FRotator Facing(0.0, 90.0 * static_cast<double>(Index), 0.0);
        SeatStaticMeshOnFloor(Accent, Corners[Index], FloorTopZ, ThemeRow.AccentHeightUnits, Facing);
    }

    ThemeFillLight->SetLightColor(ThemeRow.AccentLightColor);
}

void ADungeonShowcaseActor::ApplyFamily(const FDungeonCreatureFamilyRow& FamilyRow)
{
    const FVector CreatureColumn = DungeonCore::GetBlockColumnWorldXY(DungeonCore::GetShowcaseCreatureBlockCoord());
    const FVector2D CreatureXY(CreatureColumn.X, CreatureColumn.Y);
    const double FloorTopZ = DungeonCore::GetCoreChamberSurfaceVisualFloorZ();
    // Camera sits on the min-X side, so facing it means looking down -X.
    const FRotator Facing(0.0, 180.0 + static_cast<double>(FamilyRow.ShowcaseFacingYawDegrees), 0.0);

    USkeletalMesh* SkeletalMesh = FamilyRow.ShowcaseSkeletalMesh.IsNull() ? nullptr : FamilyRow.ShowcaseSkeletalMesh.LoadSynchronous();
    UStaticMesh* StaticMesh = FamilyRow.ShowcaseStaticMesh.IsNull() ? nullptr : FamilyRow.ShowcaseStaticMesh.LoadSynchronous();

    if (!FamilyRow.ShowcaseSkeletalMesh.IsNull() && SkeletalMesh == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: family '%s' skeletal mesh failed to load (%s)."), *FamilyRow.DisplayName.ToString(), *FamilyRow.ShowcaseSkeletalMesh.ToString());
    }
    if (!FamilyRow.ShowcaseStaticMesh.IsNull() && StaticMesh == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: family '%s' static mesh failed to load (%s)."), *FamilyRow.DisplayName.ToString(), *FamilyRow.ShowcaseStaticMesh.ToString());
    }

    CreatureSkeletalMesh->SetSkeletalMesh(SkeletalMesh);
    CreatureSkeletalMesh->SetVisibility(SkeletalMesh != nullptr);
    CreatureStaticMesh->SetStaticMesh(SkeletalMesh == nullptr ? StaticMesh : nullptr);
    CreatureStaticMesh->SetVisibility(SkeletalMesh == nullptr && StaticMesh != nullptr);

    if (SkeletalMesh != nullptr)
    {
        SeatSkeletalMeshOnFloor(CreatureSkeletalMesh, CreatureXY, FloorTopZ, FamilyRow.ShowcaseHeightUnits, Facing);
    }
    else if (StaticMesh != nullptr)
    {
        SeatStaticMeshOnFloor(CreatureStaticMesh, CreatureXY, FloorTopZ, FamilyRow.ShowcaseHeightUnits, Facing);
    }

    bHasCreatureArt = SkeletalMesh != nullptr || StaticMesh != nullptr;
    CreatureKeyLight->SetVisibility(bHasCreatureArt);
    if (!bHasCreatureArt)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Showcase: family '%s' has no creature mesh in the project yet (%s)."),
            *FamilyRow.DisplayName.ToString(),
            *FamilyRow.ShowcaseArtSource.ToString()
        );
    }
}

bool ADungeonShowcaseActor::HasCreatureArt() const
{
    return bHasCreatureArt;
}

void ADungeonShowcaseActor::SeatStaticMeshOnFloor(
    UStaticMeshComponent* MeshComponent,
    const FVector2D& WorldXY,
    const double FloorTopZ,
    const double TargetHeightUnits,
    const FRotator& Rotation
)
{
    const UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
    if (Mesh == nullptr)
    {
        return;
    }
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const double MeshHeight = Bounds.BoxExtent.Z * 2.0;
    if (MeshHeight <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: mesh '%s' has no vertical extent."), *Mesh->GetName());
        return;
    }
    const double Scale = TargetHeightUnits / MeshHeight;
    const FVector RotatedOrigin = Rotation.RotateVector(Bounds.Origin) * Scale;
    const double BottomZ = RotatedOrigin.Z - Bounds.BoxExtent.Z * Scale;
    MeshComponent->SetRelativeScale3D(FVector(Scale, Scale, Scale));
    MeshComponent->SetRelativeRotation(Rotation);
    MeshComponent->SetRelativeLocation(FVector(WorldXY.X - RotatedOrigin.X, WorldXY.Y - RotatedOrigin.Y, FloorTopZ - BottomZ));
}

void ADungeonShowcaseActor::SeatSkeletalMeshOnFloor(
    USkeletalMeshComponent* MeshComponent,
    const FVector2D& WorldXY,
    const double FloorTopZ,
    const double TargetHeightUnits,
    const FRotator& Rotation
)
{
    const USkeletalMesh* Mesh = MeshComponent->GetSkeletalMeshAsset();
    if (Mesh == nullptr)
    {
        return;
    }
    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const double MeshHeight = Bounds.BoxExtent.Z * 2.0;
    if (MeshHeight <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error, TEXT("Showcase: skeletal mesh '%s' has no vertical extent."), *Mesh->GetName());
        return;
    }
    const double Scale = TargetHeightUnits / MeshHeight;
    const FVector RotatedOrigin = Rotation.RotateVector(Bounds.Origin) * Scale;
    const double BottomZ = RotatedOrigin.Z - Bounds.BoxExtent.Z * Scale;
    MeshComponent->SetRelativeScale3D(FVector(Scale, Scale, Scale));
    MeshComponent->SetRelativeRotation(Rotation);
    MeshComponent->SetRelativeLocation(FVector(WorldXY.X - RotatedOrigin.X, WorldXY.Y - RotatedOrigin.Y, FloorTopZ - BottomZ));
}
