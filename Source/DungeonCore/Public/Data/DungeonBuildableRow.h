#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonBuildableRow.generated.h"

USTRUCT(BlueprintType)
struct FDungeonBuildableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName BuildableId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 ManaCost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FRoomFootprint Footprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<ECreatureFamilyId> AllowedFamilies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<EDungeonThemeId> AllowedThemes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSubclassOf<AActor> VisualActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EBuildableCategory Category = EBuildableCategory::Utility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId RoomTypeId = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId TrapTypeId = ETrapTypeId::None;
};
