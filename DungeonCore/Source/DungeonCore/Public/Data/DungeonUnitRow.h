#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonUnitRow.generated.h"

USTRUCT(BlueprintType)
struct FDungeonUnitRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EUnitTypeId UnitTypeId = EUnitTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId FamilyId = ECreatureFamilyId::Undead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 MaxHealth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Damage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float AttackInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float MoveSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FName> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSubclassOf<AActor> VisualActorClass;
};
