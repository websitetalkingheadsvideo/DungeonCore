#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonThemeRow.generated.h"

class UMaterialInterface;
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

    /** Library material instances applied to the chamber floor and wall blocks (one tile repeat per block). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<UMaterialInterface> FloorMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<UMaterialInterface> WallMaterial;

    /** Set dressing placed in each inside corner of the chamber (pillar, timber, crystal cluster). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<UStaticMesh> AccentMesh;

    /** Target height the accent mesh is scaled to, in Unreal units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float AccentHeightUnits = 240.0f;

    /** Light colour cast by the biome's fixtures (oil-lamp gold, torch amber, spore green, genie-lamp teal). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    FLinearColor AccentLightColor = FLinearColor::White;

    /** Light fixture placed once per wall, centred on the wall (oil lamp, torch, glowing fungus, genie lamp). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    TSoftObjectPtr<UStaticMesh> WallLightMesh;

    /** Target height of the fixture mesh in Unreal units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float WallLightHeightUnits = 60.0f;

    /** Height of the fixture's lowest point above the floor; 0 for floor-standing fixtures like fungus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float WallLightMountHeightUnits = 150.0f;

    /** Yaw added so the mesh's authored "front" faces into the room (packs differ on +X vs -X). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float WallLightFacingYawDegrees = 0.0f;

    /** Per-fixture point light intensity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Showcase")
    float WallLightIntensityCandelas = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FLinearColor FogColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TSubclassOf<AActor> ThemeDecalActorClass;
};
