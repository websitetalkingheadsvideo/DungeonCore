#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Sim/DungeonTypes.h"

#include "DungeonRunSaveGame.generated.h"

UCLASS()
class DUNGEONCORE_API UDungeonRunSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FRunSetup RunSetup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FResourceState ResourceState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FFloorState FloorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FTileData> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FRoomInstanceData> Rooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FTrapInstanceData> Traps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FUnitInstanceData> ActiveAdventurers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FUnitInstanceData> ActiveDefenders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 CurrentWaveNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 CurrentCoreHealth = 0;
};
