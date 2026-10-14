#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonChunkActor.generated.h"

class UInstancedStaticMeshComponent;
class UPointLightComponent;
class URotatingMovementComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UDungeonGridState;
class USceneComponent;

UCLASS(DisplayName = "DungeonChunkActor (C++ grid)")
class DUNGEONCORE_API ADungeonChunkActor : public AActor
{
    GENERATED_BODY()

public:
    ADungeonChunkActor();

    void RefreshFromSimulation(
        UDungeonGridState* GridState,
        const TArray<FUnitInstanceData>& ActiveAdventurers,
        const TArray<FUnitInstanceData>& ActiveDefenders
    );

private:
    static FTransform MakeTileInstanceTransform(const FVector& WorldCenter, const FVector& InstanceScale);

    void ConfigureInstanceComponent(UInstancedStaticMeshComponent* InstanceComponent) const;

    /** Z=0 columns that still have rock on top (SolidRock / Wall); everything else is open to the sky. */
    static TSet<FIntPoint> CollectRockColumns(const TArray<FTileData>& Tiles);

    void BuildSurfaceGrid(const TSet<FIntPoint>& RockColumns);

    void BuildRockSurfaceCaps(const TArray<FTileData>& Tiles);

    /** Thin walls on all four chamber sides, with a gap wherever the rock tile directly outside has been dug. */
    void BuildCoreChamberShell(const TSet<FIntPoint>& RockColumns);

    /**
     * One chamber side. Walks block indices MinIndex..MaxIndex along the wall, merges consecutive closed
     * tiles into a single instance, and skips tiles whose outside neighbour is no longer rock.
     */
    void AddChamberWallSide(
        const bool bWallRunsAlongY,
        const double FixedWorldCoord,
        const int32 MinIndex,
        const int32 MaxIndex,
        const TFunctionRef<bool(int32)>& IsClosedAt,
        const double WallCenterZ,
        const double WallScaleThin,
        const double WallScaleZ
    );

    void UpdateSurfaceDungeonVisuals();

    /** Scales the crystal mesh to the target height and seats its lowest point on DungeonCorePivot. */
    void LayoutDungeonCoreCrystal();

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> CoreChamberFloorVisual;

    /** Floating dungeon core crystal (Crystal_Cave SM_Crystal_D_01 + glow material) with a warm light inside. */
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> DungeonCorePivot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> DungeonCoreCrystalMesh;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UPointLightComponent> DungeonCoreGlowLight;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<URotatingMovementComponent> DungeonCoreSpin;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> GridLineInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> GridBaseInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> ChamberWallInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> RockInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> PitCavityInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> ClearedInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> RoomInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> TrapInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> SurfaceEntryInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> AdventurerInstances;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> DefenderInstances;
};
