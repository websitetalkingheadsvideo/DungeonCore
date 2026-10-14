#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "DungeonTypes.generated.h"

UENUM(BlueprintType)
enum class EDungeonThemeId : uint8
{
    Gothic,
    GoblinTunnels,
    FungalWarren,
    SunkenRuins
};

UENUM(BlueprintType)
enum class ECreatureFamilyId : uint8
{
    Undead,
    Orcs,
    Spiders,
    EvilFairies
};

UENUM(BlueprintType)
enum class ETileState : uint8
{
    SolidRock,
    Cleared,
    Wall,
    Room,
    Trap,
    SurfaceEntry
};

UENUM(BlueprintType)
enum class ERoomTypeId : uint8
{
    None,
    CoreChamber,
    Corridor,
    ManaWell,
    Crypt,
    WarPens,
    NestCluster,
    GlamourGrove
};

UENUM(BlueprintType)
enum class ETrapTypeId : uint8
{
    None,
    GraveGrasp,
    SpikeBarricade,
    WebSnare,
    PixieDustCloud
};

UENUM(BlueprintType)
enum class EUnitTypeId : uint8
{
    None,
    Skeleton,
    Ghoul,
    OrcGrunt,
    Skitterling,
    MischiefSprite,
    AdventurerMelee,
    AdventurerRanged
};

UENUM(BlueprintType)
enum class EDungeonEdgeDirection : uint8
{
    North,
    East,
    South,
    West
};

UENUM(BlueprintType)
enum class EBreachState : uint8
{
    Sealed,
    Open
};

UENUM(BlueprintType)
enum class EPlacementFailureReason : uint8
{
    None,
    OutOfBounds,
    Blocked,
    InsufficientMana,
    RequiresClearedTile,
    WouldDisconnectPath,
    InvalidFootprint,
    FamilyLocked,
    ThemeLocked
};

UENUM(BlueprintType)
enum class EBuildableCategory : uint8
{
    Room,
    Trap,
    Utility
};

UENUM(BlueprintType)
enum class EDungeonBuildMode : uint8
{
    Dig,
    ManaWell,
    StarterSpawner,
    StarterTrap
};

USTRUCT(BlueprintType)
struct FBlockCoord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Z = 0;

    FBlockCoord() = default;

    FBlockCoord(const int32 InX, const int32 InY, const int32 InZ)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    bool operator==(const FBlockCoord& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    FBlockCoord operator+(const FBlockCoord& Other) const
    {
        return FBlockCoord(X + Other.X, Y + Other.Y, Z + Other.Z);
    }
};

FORCEINLINE uint32 GetTypeHash(const FBlockCoord& Value)
{
    const uint32 XYHash = HashCombine(GetTypeHash(Value.X), GetTypeHash(Value.Y));
    return HashCombine(XYHash, GetTypeHash(Value.Z));
}

USTRUCT(BlueprintType)
struct FChunkCoord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 X = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Y = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Z = 0;

    FChunkCoord() = default;

    FChunkCoord(const int32 InX, const int32 InY, const int32 InZ)
        : X(InX), Y(InY), Z(InZ)
    {
    }

    bool operator==(const FChunkCoord& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& Value)
{
    const uint32 XYHash = HashCombine(GetTypeHash(Value.X), GetTypeHash(Value.Y));
    return HashCombine(XYHash, GetTypeHash(Value.Z));
}

USTRUCT(BlueprintType)
struct FRoomFootprint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Width = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Height = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Depth = 1;

    bool IsValid() const
    {
        return Width > 0 && Height > 0 && Depth > 0;
    }
};

USTRUCT(BlueprintType)
struct FPlacementResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EPlacementFailureReason FailureReason = EPlacementFailureReason::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FBlockCoord> AffectedBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 ManaCost = 0;
};

USTRUCT(BlueprintType)
struct FResourceState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 CurrentMana = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 CurrentDungeonPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float BaseManaPerSecond = 0.0f;
};

USTRUCT(BlueprintType)
struct FTileData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord BlockCoord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETileState TileState = ETileState::SolidRock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId RoomTypeId = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId TrapTypeId = ETrapTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bWalkable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bDiggable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FChunkCoord OwningChunk;
};

USTRUCT(BlueprintType)
struct FRoomInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FGuid RoomInstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ERoomTypeId RoomTypeId = ERoomTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord OriginBlock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FRoomFootprint Footprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId OwningFamily = ECreatureFamilyId::Undead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 HitPoints = 0;
};

USTRUCT(BlueprintType)
struct FTrapInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FGuid TrapInstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ETrapTypeId TrapTypeId = ETrapTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord OriginBlock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float CooldownRemaining = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId OwningFamily = ECreatureFamilyId::Undead;
};

USTRUCT(BlueprintType)
struct FUnitInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FGuid UnitInstanceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EUnitTypeId UnitTypeId = EUnitTypeId::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord CurrentBlock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord HomeBlock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 CurrentHealth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FName> StatusTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bIsAdventurer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Damage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 PathStepIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FBlockCoord> CurrentPath;
};

USTRUCT(BlueprintType)
struct FChunkState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FChunkCoord ChunkCoord;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FTileData> Tiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FGuid> RoomInstanceIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FGuid> TrapInstanceIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    bool bIsDirty = false;
};

USTRUCT(BlueprintType)
struct FRunSetup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EDungeonThemeId ThemeId = EDungeonThemeId::Gothic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    ECreatureFamilyId CreatureFamilyId = ECreatureFamilyId::Undead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 Seed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 StartingFloorIndex = 1;
};

USTRUCT(BlueprintType)
struct FFloorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    int32 FloorIndex = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EDungeonEdgeDirection OutsideEdgeDirection = EDungeonEdgeDirection::East;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    EBreachState BreachState = EBreachState::Sealed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord SurfaceEntryBlock;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord CoreBlock;
};

namespace DungeonCore
{
    inline constexpr int32 BlockSizeUnits = 300;
    inline constexpr int32 HalfBlockSizeUnits = BlockSizeUnits / 2;
    inline constexpr int32 DefaultChunkSizeInBlocks = 32;
    inline constexpr int32 StartingCoreChamberWidthBlocks = 20;
    inline constexpr int32 StartingCoreChamberHeightBlocks = 20;
    inline constexpr int32 StartingMana = 120;
    inline constexpr int32 DigManaCost = 10;
    inline constexpr int32 ManaWellCost = 30;
    inline constexpr int32 StarterSpawnerCost = 40;
    inline constexpr int32 StarterTrapCost = 25;

    inline FVector BlockToWorld(const FBlockCoord& BlockCoord)
    {
        return FVector(
            static_cast<double>(BlockCoord.X * BlockSizeUnits),
            static_cast<double>(BlockCoord.Y * BlockSizeUnits),
            static_cast<double>(BlockCoord.Z * BlockSizeUnits)
        );
    }

    inline FVector BlockCenterToWorld(const FBlockCoord& BlockCoord)
    {
        return BlockToWorld(BlockCoord)
            + FVector(
                static_cast<double>(HalfBlockSizeUnits),
                static_cast<double>(HalfBlockSizeUnits),
                static_cast<double>(HalfBlockSizeUnits)
            );
    }

    inline FVector GetCoreChamberCenterWorldLocation()
    {
        const double CenterBlock = static_cast<double>(StartingCoreChamberWidthBlocks) * 0.5 - 0.5;
        const double CenterWorld = static_cast<double>(BlockSizeUnits) * CenterBlock;
        return FVector(CenterWorld, CenterWorld, 0.0);
    }

    /** World center of the core block tile (run starts with core at block 0,0,0). */
    inline FVector GetDungeonCoreBlockWorldCenter()
    {
        return BlockToWorld(FBlockCoord(0, 0, 0))
            + FVector(
                static_cast<double>(HalfBlockSizeUnits),
                static_cast<double>(HalfBlockSizeUnits),
                0.0
            );
    }

    inline FBlockCoord WorldToBlock(const FVector& WorldLocation)
    {
        return FBlockCoord(
            FMath::RoundToInt(WorldLocation.X / static_cast<double>(BlockSizeUnits)),
            FMath::RoundToInt(WorldLocation.Y / static_cast<double>(BlockSizeUnits)),
            FMath::RoundToInt(WorldLocation.Z / static_cast<double>(BlockSizeUnits))
        );
    }
}
