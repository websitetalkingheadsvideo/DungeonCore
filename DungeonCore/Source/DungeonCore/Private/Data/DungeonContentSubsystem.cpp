#include "Data/DungeonContentSubsystem.h"

void UDungeonContentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SeedDefaultContent();
}

const FDungeonBuildableRow* UDungeonContentSubsystem::FindBuildableByRoomType(const ERoomTypeId RoomTypeId) const
{
    return RoomBuildables.Find(RoomTypeId);
}

const FDungeonBuildableRow* UDungeonContentSubsystem::FindBuildableByTrapType(const ETrapTypeId TrapTypeId) const
{
    return TrapBuildables.Find(TrapTypeId);
}

const FDungeonCreatureFamilyRow* UDungeonContentSubsystem::FindFamilyRow(const ECreatureFamilyId FamilyId) const
{
    return Families.Find(FamilyId);
}

const FDungeonThemeRow* UDungeonContentSubsystem::FindThemeRow(const EDungeonThemeId ThemeId) const
{
    return Themes.Find(ThemeId);
}

const FDungeonUnitRow* UDungeonContentSubsystem::FindUnitRow(const EUnitTypeId UnitTypeId) const
{
    return Units.Find(UnitTypeId);
}

int32 UDungeonContentSubsystem::GetDigManaCost() const
{
    return DigManaCost;
}

void UDungeonContentSubsystem::SeedDefaultContent()
{
    RoomBuildables.Reset();
    TrapBuildables.Reset();
    Families.Reset();
    Themes.Reset();
    Units.Reset();

    FDungeonThemeRow GothicTheme;
    GothicTheme.ThemeId = EDungeonThemeId::Gothic;
    GothicTheme.DisplayName = FText::FromString(TEXT("Gothic"));
    AddThemeRow(GothicTheme);

    FDungeonThemeRow GoblinTheme;
    GoblinTheme.ThemeId = EDungeonThemeId::GoblinTunnels;
    GoblinTheme.DisplayName = FText::FromString(TEXT("Goblin Tunnels"));
    AddThemeRow(GoblinTheme);

    FDungeonThemeRow FungalTheme;
    FungalTheme.ThemeId = EDungeonThemeId::FungalWarren;
    FungalTheme.DisplayName = FText::FromString(TEXT("Fungal Warren"));
    AddThemeRow(FungalTheme);

    FDungeonThemeRow SunkenTheme;
    SunkenTheme.ThemeId = EDungeonThemeId::SunkenRuins;
    SunkenTheme.DisplayName = FText::FromString(TEXT("Sunken Ruins"));
    AddThemeRow(SunkenTheme);

    FDungeonCreatureFamilyRow UndeadFamily;
    UndeadFamily.FamilyId = ECreatureFamilyId::Undead;
    UndeadFamily.DisplayName = FText::FromString(TEXT("Undead"));
    UndeadFamily.StarterSpawnerRoom = ERoomTypeId::Crypt;
    UndeadFamily.StarterTrap = ETrapTypeId::GraveGrasp;
    UndeadFamily.SignatureMechanicTag = TEXT("CorpseEconomy");
    UndeadFamily.AllowedUnitTypes = { EUnitTypeId::Skeleton, EUnitTypeId::Ghoul };
    AddFamilyRow(UndeadFamily);

    FDungeonCreatureFamilyRow OrcFamily;
    OrcFamily.FamilyId = ECreatureFamilyId::Orcs;
    OrcFamily.DisplayName = FText::FromString(TEXT("Orcs"));
    OrcFamily.StarterSpawnerRoom = ERoomTypeId::WarPens;
    OrcFamily.StarterTrap = ETrapTypeId::SpikeBarricade;
    OrcFamily.SignatureMechanicTag = TEXT("BruteForce");
    OrcFamily.AllowedUnitTypes = { EUnitTypeId::OrcGrunt };
    AddFamilyRow(OrcFamily);

    FDungeonCreatureFamilyRow SpiderFamily;
    SpiderFamily.FamilyId = ECreatureFamilyId::Spiders;
    SpiderFamily.DisplayName = FText::FromString(TEXT("Spiders"));
    SpiderFamily.StarterSpawnerRoom = ERoomTypeId::NestCluster;
    SpiderFamily.StarterTrap = ETrapTypeId::WebSnare;
    SpiderFamily.SignatureMechanicTag = TEXT("Control");
    SpiderFamily.AllowedUnitTypes = { EUnitTypeId::Skitterling };
    AddFamilyRow(SpiderFamily);

    FDungeonCreatureFamilyRow FairyFamily;
    FairyFamily.FamilyId = ECreatureFamilyId::EvilFairies;
    FairyFamily.DisplayName = FText::FromString(TEXT("Evil Fairies"));
    FairyFamily.StarterSpawnerRoom = ERoomTypeId::GlamourGrove;
    FairyFamily.StarterTrap = ETrapTypeId::PixieDustCloud;
    FairyFamily.SignatureMechanicTag = TEXT("Disruption");
    FairyFamily.AllowedUnitTypes = { EUnitTypeId::MischiefSprite };
    AddFamilyRow(FairyFamily);

    FDungeonBuildableRow ManaWell;
    ManaWell.BuildableId = TEXT("mana_well");
    ManaWell.DisplayName = FText::FromString(TEXT("Mana Well"));
    ManaWell.ManaCost = 30;
    ManaWell.Category = EBuildableCategory::Utility;
    ManaWell.RoomTypeId = ERoomTypeId::ManaWell;
    AddBuildableRow(ManaWell);

    FDungeonBuildableRow Crypt;
    Crypt.BuildableId = TEXT("crypt");
    Crypt.DisplayName = FText::FromString(TEXT("Crypt"));
    Crypt.ManaCost = 40;
    Crypt.Category = EBuildableCategory::Room;
    Crypt.RoomTypeId = ERoomTypeId::Crypt;
    Crypt.AllowedFamilies = { ECreatureFamilyId::Undead };
    AddBuildableRow(Crypt);

    FDungeonBuildableRow WarPens;
    WarPens.BuildableId = TEXT("war_pens");
    WarPens.DisplayName = FText::FromString(TEXT("War Pens"));
    WarPens.ManaCost = 40;
    WarPens.Category = EBuildableCategory::Room;
    WarPens.RoomTypeId = ERoomTypeId::WarPens;
    WarPens.AllowedFamilies = { ECreatureFamilyId::Orcs };
    AddBuildableRow(WarPens);

    FDungeonBuildableRow NestCluster;
    NestCluster.BuildableId = TEXT("nest_cluster");
    NestCluster.DisplayName = FText::FromString(TEXT("Nest Cluster"));
    NestCluster.ManaCost = 40;
    NestCluster.Category = EBuildableCategory::Room;
    NestCluster.RoomTypeId = ERoomTypeId::NestCluster;
    NestCluster.AllowedFamilies = { ECreatureFamilyId::Spiders };
    AddBuildableRow(NestCluster);

    FDungeonBuildableRow GlamourGrove;
    GlamourGrove.BuildableId = TEXT("glamour_grove");
    GlamourGrove.DisplayName = FText::FromString(TEXT("Glamour Grove"));
    GlamourGrove.ManaCost = 40;
    GlamourGrove.Category = EBuildableCategory::Room;
    GlamourGrove.RoomTypeId = ERoomTypeId::GlamourGrove;
    GlamourGrove.AllowedFamilies = { ECreatureFamilyId::EvilFairies };
    AddBuildableRow(GlamourGrove);

    FDungeonBuildableRow GraveGrasp;
    GraveGrasp.BuildableId = TEXT("grave_grasp");
    GraveGrasp.DisplayName = FText::FromString(TEXT("Grave Grasp"));
    GraveGrasp.ManaCost = 25;
    GraveGrasp.Category = EBuildableCategory::Trap;
    GraveGrasp.TrapTypeId = ETrapTypeId::GraveGrasp;
    GraveGrasp.AllowedFamilies = { ECreatureFamilyId::Undead };
    AddBuildableRow(GraveGrasp);

    FDungeonBuildableRow SpikeBarricade;
    SpikeBarricade.BuildableId = TEXT("spike_barricade");
    SpikeBarricade.DisplayName = FText::FromString(TEXT("Spike Barricade"));
    SpikeBarricade.ManaCost = 25;
    SpikeBarricade.Category = EBuildableCategory::Trap;
    SpikeBarricade.TrapTypeId = ETrapTypeId::SpikeBarricade;
    SpikeBarricade.AllowedFamilies = { ECreatureFamilyId::Orcs };
    AddBuildableRow(SpikeBarricade);

    FDungeonBuildableRow WebSnare;
    WebSnare.BuildableId = TEXT("web_snare");
    WebSnare.DisplayName = FText::FromString(TEXT("Web Snare"));
    WebSnare.ManaCost = 25;
    WebSnare.Category = EBuildableCategory::Trap;
    WebSnare.TrapTypeId = ETrapTypeId::WebSnare;
    WebSnare.AllowedFamilies = { ECreatureFamilyId::Spiders };
    AddBuildableRow(WebSnare);

    FDungeonBuildableRow PixieDustCloud;
    PixieDustCloud.BuildableId = TEXT("pixie_dust_cloud");
    PixieDustCloud.DisplayName = FText::FromString(TEXT("Pixie Dust Cloud"));
    PixieDustCloud.ManaCost = 25;
    PixieDustCloud.Category = EBuildableCategory::Trap;
    PixieDustCloud.TrapTypeId = ETrapTypeId::PixieDustCloud;
    PixieDustCloud.AllowedFamilies = { ECreatureFamilyId::EvilFairies };
    AddBuildableRow(PixieDustCloud);

    FDungeonUnitRow Skeleton;
    Skeleton.UnitTypeId = EUnitTypeId::Skeleton;
    Skeleton.DisplayName = FText::FromString(TEXT("Skeleton"));
    Skeleton.FamilyId = ECreatureFamilyId::Undead;
    Skeleton.MaxHealth = 26;
    Skeleton.Damage = 8;
    AddUnitRow(Skeleton);

    FDungeonUnitRow Ghoul;
    Ghoul.UnitTypeId = EUnitTypeId::Ghoul;
    Ghoul.DisplayName = FText::FromString(TEXT("Ghoul"));
    Ghoul.FamilyId = ECreatureFamilyId::Undead;
    Ghoul.MaxHealth = 18;
    Ghoul.Damage = 10;
    AddUnitRow(Ghoul);

    FDungeonUnitRow OrcGrunt;
    OrcGrunt.UnitTypeId = EUnitTypeId::OrcGrunt;
    OrcGrunt.DisplayName = FText::FromString(TEXT("Orc Grunt"));
    OrcGrunt.FamilyId = ECreatureFamilyId::Orcs;
    OrcGrunt.MaxHealth = 30;
    OrcGrunt.Damage = 14;
    AddUnitRow(OrcGrunt);

    FDungeonUnitRow Skitterling;
    Skitterling.UnitTypeId = EUnitTypeId::Skitterling;
    Skitterling.DisplayName = FText::FromString(TEXT("Skitterling"));
    Skitterling.FamilyId = ECreatureFamilyId::Spiders;
    Skitterling.MaxHealth = 18;
    Skitterling.Damage = 9;
    AddUnitRow(Skitterling);

    FDungeonUnitRow MischiefSprite;
    MischiefSprite.UnitTypeId = EUnitTypeId::MischiefSprite;
    MischiefSprite.DisplayName = FText::FromString(TEXT("Mischief Sprite"));
    MischiefSprite.FamilyId = ECreatureFamilyId::EvilFairies;
    MischiefSprite.MaxHealth = 14;
    MischiefSprite.Damage = 6;
    AddUnitRow(MischiefSprite);

    FDungeonUnitRow AdventurerMelee;
    AdventurerMelee.UnitTypeId = EUnitTypeId::AdventurerMelee;
    AdventurerMelee.DisplayName = FText::FromString(TEXT("Adventurer Melee"));
    AdventurerMelee.MaxHealth = 10;
    AdventurerMelee.Damage = 10;
    AddUnitRow(AdventurerMelee);

    FDungeonUnitRow AdventurerRanged;
    AdventurerRanged.UnitTypeId = EUnitTypeId::AdventurerRanged;
    AdventurerRanged.DisplayName = FText::FromString(TEXT("Adventurer Ranged"));
    AdventurerRanged.MaxHealth = 10;
    AdventurerRanged.Damage = 8;
    AddUnitRow(AdventurerRanged);
}

void UDungeonContentSubsystem::AddBuildableRow(const FDungeonBuildableRow& RowData)
{
    if (RowData.RoomTypeId != ERoomTypeId::None)
    {
        RoomBuildables.Add(RowData.RoomTypeId, RowData);
    }

    if (RowData.TrapTypeId != ETrapTypeId::None)
    {
        TrapBuildables.Add(RowData.TrapTypeId, RowData);
    }
}

void UDungeonContentSubsystem::AddFamilyRow(const FDungeonCreatureFamilyRow& RowData)
{
    Families.Add(RowData.FamilyId, RowData);
}

void UDungeonContentSubsystem::AddThemeRow(const FDungeonThemeRow& RowData)
{
    Themes.Add(RowData.ThemeId, RowData);
}

void UDungeonContentSubsystem::AddUnitRow(const FDungeonUnitRow& RowData)
{
    Units.Add(RowData.UnitTypeId, RowData);
}
