#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/DungeonBuildableRow.h"
#include "Data/DungeonCreatureFamilyRow.h"
#include "Data/DungeonThemeRow.h"
#include "Data/DungeonUnitRow.h"

#include "DungeonContentSubsystem.generated.h"

UCLASS()
class DUNGEONCORE_API UDungeonContentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    const FDungeonBuildableRow* FindBuildableByRoomType(const ERoomTypeId RoomTypeId) const;

    const FDungeonBuildableRow* FindBuildableByTrapType(const ETrapTypeId TrapTypeId) const;

    const FDungeonCreatureFamilyRow* FindFamilyRow(const ECreatureFamilyId FamilyId) const;

    const FDungeonThemeRow* FindThemeRow(const EDungeonThemeId ThemeId) const;

    const FDungeonUnitRow* FindUnitRow(const EUnitTypeId UnitTypeId) const;

    int32 GetDigManaCost() const;

private:
    void SeedDefaultContent();

    void AddBuildableRow(const FDungeonBuildableRow& RowData);

    void AddFamilyRow(const FDungeonCreatureFamilyRow& RowData);

    void AddThemeRow(const FDungeonThemeRow& RowData);

    void AddUnitRow(const FDungeonUnitRow& RowData);

    UPROPERTY()
    TMap<ERoomTypeId, FDungeonBuildableRow> RoomBuildables;

    UPROPERTY()
    TMap<ETrapTypeId, FDungeonBuildableRow> TrapBuildables;

    UPROPERTY()
    TMap<ECreatureFamilyId, FDungeonCreatureFamilyRow> Families;

    UPROPERTY()
    TMap<EDungeonThemeId, FDungeonThemeRow> Themes;

    UPROPERTY()
    TMap<EUnitTypeId, FDungeonUnitRow> Units;

    int32 DigManaCost = DungeonCore::DigManaCost;
};
