#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonBuildableRow.generated.h"

/** One purchasable shop entry, loaded from Content/Data/shop-catalog.json. */
USTRUCT(BlueprintType)
struct FDungeonBuildableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName BuildableId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName ShopGroupId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    /** Gameplay summary shown as the shop tooltip. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText RoleText;

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
    EBuildableCategory Category = EBuildableCategory::Room;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId RoomTypeId = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId TrapTypeId = ETrapTypeId::None;

    /** Flat mana income this item adds while placed (room) or owned (upgrade). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float ManaPerSecond = 0.0f;

    /** Extra mana income per adventurer currently inside the dungeon; rewards maze layouts that stall parties. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float ManaPerAdventurerPerSecond = 0.0f;

    /** Upgrade that must already be owned before this one can be bought; None when unrestricted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName RequiresBuildableId;
};

/** A shop tab: ordered list of buildable ids under one heading. */
USTRUCT(BlueprintType)
struct FDungeonShopGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FName GroupId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FName> BuildableIds;
};
