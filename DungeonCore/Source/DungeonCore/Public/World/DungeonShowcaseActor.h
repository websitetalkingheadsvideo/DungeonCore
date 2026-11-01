#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonShowcaseActor.generated.h"

class UInstancedStaticMeshComponent;
class UPointLightComponent;
class URotatingMovementComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USpotLightComponent;
class UStaticMeshComponent;
struct FDungeonCreatureFamilyRow;
struct FDungeonThemeRow;

/** One wall-mounted light fixture: the mesh plus the point light it casts. */
USTRUCT()
struct FDungeonShowcaseWallLight
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY()
    TObjectPtr<UPointLightComponent> Light;

    /** Centre of the wall's inner face at floor level. */
    FVector FacePoint = FVector::ZeroVector;

    /** Unit axis pointing from the wall into the room. */
    FVector InwardNormal = FVector::ZeroVector;
};

/**
 * Hero shot of the 3 x 3 core chamber shown behind the run setup screen. Built at the real chamber
 * location so Start Run hands off to the chunk actor without a cut. Floor, walls, corner accents and
 * one light fixture per wall are re-skinned from a theme row; the creature slot is filled from a family row.
 */
UCLASS(DisplayName = "DungeonShowcaseActor (setup hero shot)")
class DUNGEONCORE_API ADungeonShowcaseActor : public AActor
{
    GENERATED_BODY()

public:
    ADungeonShowcaseActor();

    void ApplyTheme(const FDungeonThemeRow& ThemeRow);

    void ApplyFamily(const FDungeonCreatureFamilyRow& FamilyRow);

    /** False while the selected family has no showcase mesh in the project (art still on Fab). */
    bool HasCreatureArt() const;

private:
    void BuildChamberGeometry();

    void LayoutCoreCrystal();

    /** Uniformly scales a static mesh component to TargetHeight and seats its lowest point on FloorTopZ at XY. */
    static void SeatStaticMeshOnFloor(
        UStaticMeshComponent* MeshComponent,
        const FVector2D& WorldXY,
        const double FloorTopZ,
        const double TargetHeightUnits,
        const FRotator& Rotation
    );

    static void SeatSkeletalMeshOnFloor(
        USkeletalMeshComponent* MeshComponent,
        const FVector2D& WorldXY,
        const double FloorTopZ,
        const double TargetHeightUnits,
        const FRotator& Rotation
    );

    /**
     * Scales the mesh to TargetHeight, faces it along InwardNormal (+ FacingYaw), and pushes it back so its
     * rotated bounds touch the wall's inner face with its lowest point MountHeight above the floor.
     * Returns the world-space depth of the fixture along the normal so the light can sit just in front of it.
     */
    static double SeatStaticMeshAgainstWall(
        UStaticMeshComponent* MeshComponent,
        const FVector& FacePoint,
        const FVector& InwardNormal,
        const double FloorTopZ,
        const double MountHeightUnits,
        const double TargetHeightUnits,
        const double FacingYawDegrees
    );

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> FloorInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> WallInstances;

    /** Four inside corners; index order is min/min, min/max, max/min, max/max in block X/Y. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TArray<TObjectPtr<UStaticMeshComponent>> AccentMeshes;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> CorePivot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> CoreCrystalMesh;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UPointLightComponent> CoreGlowLight;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<URotatingMovementComponent> CoreSpin;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USkeletalMeshComponent> CreatureSkeletalMesh;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> CreatureStaticMesh;

    /** Faint theme-coloured fill above the chamber; the fixtures carry most of the colour. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UPointLightComponent> ThemeFillLight;

    /** East, south and north walls; the camera-side wall is open so it has no fixture. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TArray<FDungeonShowcaseWallLight> WallLights;

    /** Neutral key on the creature from the camera side. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USpotLightComponent> CreatureKeyLight;

    bool bHasCreatureArt = false;
};
