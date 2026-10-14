#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/DungeonBuildableRow.h"
#include "Data/DungeonCreatureFamilyRow.h"
#include "Data/DungeonThemeRow.h"
#include "Data/DungeonUnitRow.h"

#include "DungeonContentSubsystem.generated.h"

/**
 * Static game content. Themes, families and units are seeded in C++; everything the player can buy
 * (dig, rooms, traps, upgrades) is loaded from Content/Data/shop-catalog.json at startup.
 */
UCLASS()
class DUNGEONCORE_API UDungeonContentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    const FDungeonBuildableRow* FindBuildable(const FName BuildableId) const;

    const FDungeonBuildableRow* FindBuildableByRoomType(const ERoomTypeId RoomTypeId) const;

    const FDungeonBuildableRow* FindBuildableByTrapType(const ETrapTypeId TrapTypeId) const;

    const FDungeonCreatureFamilyRow* FindFamilyRow(const ECreatureFamilyId FamilyId) const;

    const FDungeonThemeRow* FindThemeRow(const EDungeonThemeId ThemeId) const;

    const FDungeonUnitRow* FindUnitRow(const EUnitTypeId UnitTypeId) const;

    /** Shop tabs in catalog order; design-only groups are excluded. */
    const TArray<FDungeonShopGroup>& GetShopGroups() const;

    int32 GetDigManaCost() const;

private:
    void SeedDefaultContent();

    /** Parses the shop catalog JSON. Any schema problem is fatal; there is no built-in fallback catalog. */
    void LoadShopCatalog();

    void AddBuildableRow(const FDungeonBuildableRow& RowData);

    void AddFamilyRow(const FDungeonCreatureFamilyRow& RowData);

    void AddThemeRow(const FDungeonThemeRow& RowData);

    void AddUnitRow(const FDungeonUnitRow& RowData);

    UPROPERTY()
    TMap<FName, FDungeonBuildableRow> Buildables;

    UPROPERTY()
    TArray<FDungeonShopGroup> ShopGroups;

    UPROPERTY()
    TMap<ECreatureFamilyId, FDungeonCreatureFamilyRow> Families;

    UPROPERTY()
    TMap<EDungeonThemeId, FDungeonThemeRow> Themes;

    UPROPERTY()
    TMap<EUnitTypeId, FDungeonUnitRow> Units;
};
