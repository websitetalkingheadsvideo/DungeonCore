#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonThemeRow.generated.h"

class USoundBase;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FDungeonThemeRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EDungeonThemeId ThemeId = EDungeonThemeId::Gothic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSoftObjectPtr<UStaticMesh> FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSoftObjectPtr<UStaticMesh> WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FLinearColor FogColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSubclassOf<AActor> ThemeDecalActorClass;
};
