#include "Data/DungeonContentSubsystem.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    /** Identifies the item being parsed so schema errors point at the exact JSON entry. */
    struct FCatalogItemContext
    {
        FString GroupId;
        FString ItemId;
    };

    FString DescribeItem(const FCatalogItemContext& Context)
    {
        return FString::Printf(TEXT("shop-catalog group '%s' item '%s'"), *Context.GroupId, *Context.ItemId);
    }

    FString RequireStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const FCatalogItemContext& Context)
    {
        FString Value;
        if (!Object->TryGetStringField(FieldName, Value) || Value.IsEmpty())
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: missing or empty string field '%s'."), *DescribeItem(Context), FieldName);
        }
        return Value;
    }

    int32 RequireIntField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const FCatalogItemContext& Context)
    {
        int32 Value = 0;
        if (!Object->TryGetNumberField(FieldName, Value))
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: missing integer field '%s'."), *DescribeItem(Context), FieldName);
        }
        return Value;
    }

    /** Optional numeric field; absent means zero. Present-but-not-a-number is a schema error. */
    float ReadOptionalFloatField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const FCatalogItemContext& Context)
    {
        if (!Object->HasField(FieldName))
        {
            return 0.0f;
        }

        double Value = 0.0;
        if (!Object->TryGetNumberField(FieldName, Value))
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: field '%s' must be a number."), *DescribeItem(Context), FieldName);
        }
        return static_cast<float>(Value);
    }

    template <typename TEnum>
    TEnum ParseEnumField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const FCatalogItemContext& Context)
    {
        const FString ValueName = RequireStringField(Object, FieldName, Context);
        const UEnum* EnumType = StaticEnum<TEnum>();
        const int64 EnumValue = EnumType->GetValueByNameString(ValueName);
        if (EnumValue == INDEX_NONE)
        {
            UE_LOG(
                LogTemp,
                Fatal,
                TEXT("%s: '%s' is not a valid %s value for field '%s'."),
                *DescribeItem(Context),
                *ValueName,
                *EnumType->GetName(),
                FieldName
            );
        }
        return static_cast<TEnum>(EnumValue);
    }

    EBuildableCategory ParseKindField(const TSharedPtr<FJsonObject>& Object, const FCatalogItemContext& Context)
    {
        const FString Kind = RequireStringField(Object, TEXT("kind"), Context);
        if (Kind == TEXT("dig"))
        {
            return EBuildableCategory::Dig;
        }
        if (Kind == TEXT("room"))
        {
            return EBuildableCategory::Room;
        }
        if (Kind == TEXT("trap"))
        {
            return EBuildableCategory::Trap;
        }
        if (Kind == TEXT("upgrade"))
        {
            return EBuildableCategory::Upgrade;
        }

        UE_LOG(LogTemp, Fatal, TEXT("%s: unknown kind '%s' (expected dig, room, trap or upgrade)."), *DescribeItem(Context), *Kind);
        return EBuildableCategory::Dig;
    }

    FRoomFootprint ParseFootprintField(const TSharedPtr<FJsonObject>& Object, const FCatalogItemContext& Context)
    {
        const TSharedPtr<FJsonObject>* FootprintObject = nullptr;
        if (!Object->TryGetObjectField(TEXT("footprint"), FootprintObject) || FootprintObject == nullptr || !FootprintObject->IsValid())
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: rooms require a 'footprint' object."), *DescribeItem(Context));
        }

        FRoomFootprint Footprint;
        Footprint.Width = RequireIntField(*FootprintObject, TEXT("width"), Context);
        Footprint.Height = RequireIntField(*FootprintObject, TEXT("height"), Context);
        Footprint.Depth = RequireIntField(*FootprintObject, TEXT("depth"), Context);
        if (!Footprint.IsValid())
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: footprint dimensions must all be positive."), *DescribeItem(Context));
        }
        return Footprint;
    }

    FRoomFootprint SingleBlockFootprint()
    {
        FRoomFootprint Footprint;
        Footprint.Width = 1;
        Footprint.Height = 1;
        Footprint.Depth = 1;
        return Footprint;
    }

    FDungeonBuildableRow ParseBuildableItem(const TSharedPtr<FJsonObject>& ItemObject, const FString& GroupId)
    {
        FCatalogItemContext Context;
        Context.GroupId = GroupId;
        ItemObject->TryGetStringField(TEXT("id"), Context.ItemId);
        if (Context.ItemId.IsEmpty())
        {
            Context.ItemId = TEXT("<missing id>");
            UE_LOG(LogTemp, Fatal, TEXT("%s: every shop item needs a non-empty 'id'."), *DescribeItem(Context));
        }

        FDungeonBuildableRow Row;
        Row.BuildableId = FName(*Context.ItemId);
        Row.ShopGroupId = FName(*GroupId);
        Row.DisplayName = FText::FromString(RequireStringField(ItemObject, TEXT("name"), Context));
        Row.RoleText = FText::FromString(RequireStringField(ItemObject, TEXT("role"), Context));
        Row.ManaCost = RequireIntField(ItemObject, TEXT("cost"), Context);
        if (Row.ManaCost <= 0)
        {
            UE_LOG(LogTemp, Fatal, TEXT("%s: cost must be positive."), *DescribeItem(Context));
        }

        Row.Category = ParseKindField(ItemObject, Context);
        Row.ManaPerSecond = ReadOptionalFloatField(ItemObject, TEXT("manaPerSecond"), Context);
        Row.ManaPerAdventurerPerSecond = ReadOptionalFloatField(ItemObject, TEXT("manaPerAdventurerPerSecond"), Context);

        switch (Row.Category)
        {
        case EBuildableCategory::Dig:
            Row.Footprint = SingleBlockFootprint();
            break;

        case EBuildableCategory::Room:
            Row.RoomTypeId = ParseEnumField<ERoomTypeId>(ItemObject, TEXT("roomType"), Context);
            Row.Footprint = ParseFootprintField(ItemObject, Context);
            break;

        case EBuildableCategory::Trap:
            Row.TrapTypeId = ParseEnumField<ETrapTypeId>(ItemObject, TEXT("trapType"), Context);
            Row.Footprint = SingleBlockFootprint();
            break;

        case EBuildableCategory::Upgrade:
            Row.Footprint = SingleBlockFootprint();
            break;

        default:
            UE_LOG(LogTemp, Fatal, TEXT("%s: unhandled buildable category."), *DescribeItem(Context));
            break;
        }

        if (ItemObject->HasField(TEXT("family")))
        {
            Row.AllowedFamilies.Add(ParseEnumField<ECreatureFamilyId>(ItemObject, TEXT("family"), Context));
        }

        FString RequiresId;
        if (ItemObject->TryGetStringField(TEXT("requires"), RequiresId))
        {
            if (RequiresId.IsEmpty())
            {
                UE_LOG(LogTemp, Fatal, TEXT("%s: 'requires' must name another item id when present."), *DescribeItem(Context));
            }
            Row.RequiresBuildableId = FName(*RequiresId);
        }

        return Row;
    }
}

void UDungeonContentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SeedDefaultContent();
    LoadShopCatalog();
}

const FDungeonBuildableRow* UDungeonContentSubsystem::FindBuildable(const FName BuildableId) const
{
    return Buildables.Find(BuildableId);
}

const FDungeonBuildableRow* UDungeonContentSubsystem::FindBuildableByRoomType(const ERoomTypeId RoomTypeId) const
{
    for (const TPair<FName, FDungeonBuildableRow>& Entry : Buildables)
    {
        if (Entry.Value.Category == EBuildableCategory::Room && Entry.Value.RoomTypeId == RoomTypeId)
        {
            return &Entry.Value;
        }
    }
    return nullptr;
}

const FDungeonBuildableRow* UDungeonContentSubsystem::FindBuildableByTrapType(const ETrapTypeId TrapTypeId) const
{
    for (const TPair<FName, FDungeonBuildableRow>& Entry : Buildables)
    {
        if (Entry.Value.Category == EBuildableCategory::Trap && Entry.Value.TrapTypeId == TrapTypeId)
        {
            return &Entry.Value;
        }
    }
    return nullptr;
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

const TArray<FDungeonShopGroup>& UDungeonContentSubsystem::GetShopGroups() const
{
    return ShopGroups;
}

int32 UDungeonContentSubsystem::GetDigManaCost() const
{
    const FDungeonBuildableRow* DigRow = FindBuildable(FName(DungeonCore::DigBuildableId));
    if (DigRow == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop catalog has no '%s' item; the dig tool needs it for its cost."), DungeonCore::DigBuildableId);
        return 0;
    }
    return DigRow->ManaCost;
}

void UDungeonContentSubsystem::SeedDefaultContent()
{
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

void UDungeonContentSubsystem::LoadShopCatalog()
{
    Buildables.Reset();
    ShopGroups.Reset();

    const FString CatalogPath = FPaths::Combine(FPaths::ProjectContentDir(), DungeonCore::ShopCatalogRelativePath);
    FString CatalogText;
    if (!FFileHelper::LoadFileToString(CatalogText, *CatalogPath))
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop catalog not found at '%s'."), *CatalogPath);
        return;
    }

    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CatalogText);
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop catalog '%s' is not valid JSON: %s"), *CatalogPath, *Reader->GetErrorMessage());
        return;
    }

    const TArray<TSharedPtr<FJsonValue>>* GroupValues = nullptr;
    if (!RootObject->TryGetArrayField(TEXT("shopGroups"), GroupValues) || GroupValues == nullptr)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop catalog '%s' has no 'shopGroups' array."), *CatalogPath);
        return;
    }

    for (const TSharedPtr<FJsonValue>& GroupValue : *GroupValues)
    {
        const TSharedPtr<FJsonObject>* GroupObjectPtr = nullptr;
        if (!GroupValue.IsValid() || !GroupValue->TryGetObject(GroupObjectPtr) || GroupObjectPtr == nullptr)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Shop catalog '%s': every entry in 'shopGroups' must be an object."), *CatalogPath);
            return;
        }
        const TSharedPtr<FJsonObject>& GroupObject = *GroupObjectPtr;

        FString GroupId;
        if (!GroupObject->TryGetStringField(TEXT("id"), GroupId) || GroupId.IsEmpty())
        {
            UE_LOG(LogTemp, Fatal, TEXT("Shop catalog '%s': a shop group is missing its 'id'."), *CatalogPath);
            return;
        }

        FString ImplementationStatus;
        GroupObject->TryGetStringField(TEXT("implementationStatus"), ImplementationStatus);
        if (ImplementationStatus == TEXT("design_only"))
        {
            UE_LOG(LogTemp, Log, TEXT("Shop catalog: skipping design-only group '%s'."), *GroupId);
            continue;
        }

        FDungeonShopGroup Group;
        Group.GroupId = FName(*GroupId);

        FString GroupDisplayName;
        if (!GroupObject->TryGetStringField(TEXT("displayName"), GroupDisplayName) || GroupDisplayName.IsEmpty())
        {
            UE_LOG(LogTemp, Fatal, TEXT("Shop catalog group '%s' is missing 'displayName'."), *GroupId);
            return;
        }
        Group.DisplayName = FText::FromString(GroupDisplayName);

        const TArray<TSharedPtr<FJsonValue>>* ItemValues = nullptr;
        if (!GroupObject->TryGetArrayField(TEXT("items"), ItemValues) || ItemValues == nullptr)
        {
            UE_LOG(LogTemp, Fatal, TEXT("Shop catalog group '%s' has no 'items' array."), *GroupId);
            return;
        }

        for (const TSharedPtr<FJsonValue>& ItemValue : *ItemValues)
        {
            const TSharedPtr<FJsonObject>* ItemObjectPtr = nullptr;
            if (!ItemValue.IsValid() || !ItemValue->TryGetObject(ItemObjectPtr) || ItemObjectPtr == nullptr)
            {
                UE_LOG(LogTemp, Fatal, TEXT("Shop catalog group '%s': every item must be an object."), *GroupId);
                return;
            }

            const FDungeonBuildableRow Row = ParseBuildableItem(*ItemObjectPtr, GroupId);
            if (Buildables.Contains(Row.BuildableId))
            {
                UE_LOG(LogTemp, Fatal, TEXT("Shop catalog: duplicate item id '%s'."), *Row.BuildableId.ToString());
                return;
            }

            AddBuildableRow(Row);
            Group.BuildableIds.Add(Row.BuildableId);
        }

        ShopGroups.Add(Group);
    }

    // Prerequisites must point at upgrades that exist, otherwise the item could never be bought.
    for (const TPair<FName, FDungeonBuildableRow>& Entry : Buildables)
    {
        if (Entry.Value.RequiresBuildableId.IsNone())
        {
            continue;
        }

        const FDungeonBuildableRow* RequiredRow = Buildables.Find(Entry.Value.RequiresBuildableId);
        if (RequiredRow == nullptr || RequiredRow->Category != EBuildableCategory::Upgrade)
        {
            UE_LOG(
                LogTemp,
                Fatal,
                TEXT("Shop catalog item '%s' requires '%s', which is not an upgrade in the catalog."),
                *Entry.Key.ToString(),
                *Entry.Value.RequiresBuildableId.ToString()
            );
            return;
        }
    }

    if (!Buildables.Contains(FName(DungeonCore::DigBuildableId)))
    {
        UE_LOG(LogTemp, Fatal, TEXT("Shop catalog must contain a '%s' item of kind 'dig'."), DungeonCore::DigBuildableId);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Shop catalog loaded: %d groups, %d items from '%s'."), ShopGroups.Num(), Buildables.Num(), *CatalogPath);
}

void UDungeonContentSubsystem::AddBuildableRow(const FDungeonBuildableRow& RowData)
{
    Buildables.Add(RowData.BuildableId, RowData);
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
