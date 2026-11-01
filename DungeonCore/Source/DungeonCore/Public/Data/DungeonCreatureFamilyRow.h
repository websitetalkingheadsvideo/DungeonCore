#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sim/DungeonTypes.h"

#include "DungeonCreatureFamilyRow.generated.h"

class USkeletalMesh;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FDungeonCreatureFamilyRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId FamilyId = ECreatureFamilyId::Undead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    /** Base creature shown on the setup screen. Skeletal wins when both are set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<USkeletalMesh> ShowcaseSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<UStaticMesh> ShowcaseStaticMesh;

    /** Target standing height in Unreal units; the mesh is uniformly scaled to this. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float ShowcaseHeightUnits = 180.0f;

    /** Yaw added so the mesh's authored forward faces the showcase camera (packs differ on +X vs +Y forward). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float ShowcaseFacingYawDegrees = 0.0f;

    /** Where the art comes from (Design/Monsters.md), shown on screen while the mesh is not yet in the project. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    FText ShowcaseArtSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId StarterSpawnerRoom = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId StarterTrap = ETrapTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName SignatureMechanicTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<EUnitTypeId> AllowedUnitTypes;
};
