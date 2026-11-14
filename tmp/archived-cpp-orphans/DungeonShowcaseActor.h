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

/**
 * Hero shot of the 3 x 3 core chamber shown behind the run setup screen. Built at the real chamber
 * location so Start Run hands off to the chunk actor without a cut. Floor, walls and corner accents
 * are re-skinned from a theme row; the creature slot is filled from a family row.
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

    /** Theme-coloured fill above the chamber. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UPointLightComponent> ThemeFillLight;

    /** Neutral key on the creature from the camera side. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USpotLightComponent> CreatureKeyLight;

    bool bHasCreatureArt = false;
};
