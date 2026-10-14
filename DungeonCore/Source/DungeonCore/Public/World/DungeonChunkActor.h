#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonChunkActor.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
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

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UInstancedStaticMeshComponent> RockInstances;

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
