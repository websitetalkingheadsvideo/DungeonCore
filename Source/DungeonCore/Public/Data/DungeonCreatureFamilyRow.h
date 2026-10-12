#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sim/DungeonTypes.h"

#include "DungeonCreatureFamilyRow.generated.h"

USTRUCT(BlueprintType)
struct FDungeonCreatureFamilyRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId FamilyId = ECreatureFamilyId::Undead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId StarterSpawnerRoom = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId StarterTrap = ETrapTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName SignatureMechanicTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<EUnitTypeId> AllowedUnitTypes;
};
