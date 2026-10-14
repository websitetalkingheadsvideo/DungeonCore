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
    GlamourGrove,
    LeyLineTap,
    ManaCrystalCluster,
    SiphonTotem
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
    ThemeLocked,
    AlreadyOwned,
    MissingPrerequisite,
    UnknownBuildable
};

/** What buying a shop item does: Dig/Room/Trap are placed on the grid, Upgrade applies to the core immediately. */
UENUM(BlueprintType)
enum class EBuildableCategory : uint8
{
    Dig,
    Room,
    Trap,
    Upgrade
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

    /** Effective income after Mana Wells and adventurers inside; what the HUD shows as +x/s. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    float ManaIncomePerSecond = 0.0f;
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

    /** Fog of war: tiles this unit has seen. Adventurers only path through these and explore toward unknown ones. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FBlockCoord> KnownBlocks;
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
    EBreachState BreachState = EBreachState::Sealed;

    /** Every dug tile on the build-area edge; empty while Sealed. Adventurers enter through any of them. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    TArray<FBlockCoord> SurfaceEntryBlocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
    FBlockCoord CoreBlock;
};

namespace DungeonCore
{
    inline constexpr int32 BlockSizeUnits = 300;
    inline constexpr int32 HalfBlockSizeUnits = BlockSizeUnits / 2;
    inline constexpr int32 DefaultChunkSizeInBlocks = 32;
    /** Sealed starting core chamber (CoreChamber room): 30 ft x 30 ft = 3 x 3 blocks carved out of solid rock; core on the centre block. */
    inline constexpr int32 StartingCoreChamberWidthBlocks = 3;
    inline constexpr int32 StartingCoreChamberHeightBlocks = 3;
    /** Solid rock around the chamber that the player digs into. */
    inline constexpr int32 StartingRockMarginBlocks = 11;
    inline constexpr double GridLineThicknessUnits = 8.0;
    inline constexpr double GridLineHeightUnits = 12.0;
    inline constexpr double GridSurfaceOffsetUnits = 3.0;

    /** Drafting / architect blue for surface grid lines (visible on the light build pad). */
    inline FLinearColor GetGridLineColor()
    {
        return FLinearColor(0.0f, 0.20f, 0.45f, 1.0f);
    }
    inline constexpr double CoreChamberFloorThicknessUnits = 80.0;
    inline constexpr double CoreChamberWallThicknessUnits = 35.0;

    inline void GetStartingBuildAreaBlockBounds(int32& OutMinX, int32& OutMaxX, int32& OutMinY, int32& OutMaxY)
    {
        OutMinX = -StartingRockMarginBlocks;
        OutMaxX = StartingCoreChamberWidthBlocks + StartingRockMarginBlocks - 1;
        OutMinY = -StartingRockMarginBlocks;
        OutMaxY = StartingCoreChamberHeightBlocks + StartingRockMarginBlocks - 1;
    }

    inline bool IsBlockInStartingBuildArea(const FBlockCoord& BlockCoord)
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetStartingBuildAreaBlockBounds(MinX, MaxX, MinY, MaxY);
        return BlockCoord.Z == 0
            && BlockCoord.X >= MinX
            && BlockCoord.X <= MaxX
            && BlockCoord.Y >= MinY
            && BlockCoord.Y <= MaxY;
    }

    /** The outer ring of the build area is the mountain's surface; digging into it opens the dungeon to adventurers. */
    inline bool IsSurfaceEdgeBlock(const FBlockCoord& BlockCoord)
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetStartingBuildAreaBlockBounds(MinX, MaxX, MinY, MaxY);
        return IsBlockInStartingBuildArea(BlockCoord)
            && (BlockCoord.X == MinX || BlockCoord.X == MaxX || BlockCoord.Y == MinY || BlockCoord.Y == MaxY);
    }

    inline bool IsDungeonCoreBlockCoord(const FBlockCoord& BlockCoord)
    {
        const int32 CoreX = (StartingCoreChamberWidthBlocks - 1) / 2;
        const int32 CoreY = (StartingCoreChamberHeightBlocks - 1) / 2;
        return BlockCoord.X == CoreX && BlockCoord.Y == CoreY && BlockCoord.Z == 0;
    }

    inline void GetCoreChamberBlockBounds(int32& OutMinX, int32& OutMaxX, int32& OutMinY, int32& OutMaxY)
    {
        OutMinX = 0;
        OutMinY = 0;
        OutMaxX = StartingCoreChamberWidthBlocks - 1;
        OutMaxY = StartingCoreChamberHeightBlocks - 1;
    }

    inline bool IsBlockInsideCoreChamberFootprint(const FBlockCoord& BlockCoord)
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetCoreChamberBlockBounds(MinX, MaxX, MinY, MaxY);
        return BlockCoord.Z == 0
            && BlockCoord.X >= MinX
            && BlockCoord.X <= MaxX
            && BlockCoord.Y >= MinY
            && BlockCoord.Y <= MaxY;
    }

    inline FVector GetInclusiveBlockBoundsWorldCenter(
        const int32 MinBlockX,
        const int32 MinBlockY,
        const int32 MaxBlockX,
        const int32 MaxBlockY
    )
    {
        const double MinWorldX = static_cast<double>(MinBlockX * BlockSizeUnits);
        const double MinWorldY = static_cast<double>(MinBlockY * BlockSizeUnits);
        const double MaxWorldX = static_cast<double>((MaxBlockX + 1) * BlockSizeUnits);
        const double MaxWorldY = static_cast<double>((MaxBlockY + 1) * BlockSizeUnits);
        return FVector(
            (MinWorldX + MaxWorldX) * 0.5,
            (MinWorldY + MaxWorldY) * 0.5,
            0.0
        );
    }

    inline FVector GetInclusiveBlockBoundsWorldSize(
        const int32 MinBlockX,
        const int32 MinBlockY,
        const int32 MaxBlockX,
        const int32 MaxBlockY
    )
    {
        return FVector(
            static_cast<double>((MaxBlockX - MinBlockX + 1) * BlockSizeUnits),
            static_cast<double>((MaxBlockY - MinBlockY + 1) * BlockSizeUnits),
            0.0
        );
    }

    inline double GetMountainSurfaceWorldZ()
    {
        return GridSurfaceOffsetUnits;
    }
    inline constexpr int32 StartingMana = 120;
    /**
     * Mana economy: the core trickles mana on its own, faster once open, more per adventurer wandering inside.
     * Per-item income (wells, taps, attunements) comes from the shop catalog rows, not from constants here.
     */
    inline constexpr float SealedManaPerSecond = 0.4f;
    inline constexpr float OpenManaPerSecond = 0.6f;
    inline constexpr float ManaPerAdventurerInsidePerSecond = 0.5f;
    inline constexpr int32 MaxAdventurersCountedForMana = 10;
    inline constexpr float ManaTickIntervalSeconds = 1.0f;
    /** How far an adventurer sees down a straight corridor (in blocks) before rock blocks the view. */
    inline constexpr int32 AdventurerSightBlocks = 2;
    /** Shop catalog JSON, relative to the project Content directory. */
    inline const TCHAR* ShopCatalogRelativePath = TEXT("Data/shop-catalog.json");
    /** Catalog item id that the dig tool reads its cost from. */
    inline const TCHAR* DigBuildableId = TEXT("dig");
    inline constexpr int32 MaxShopHotkeys = 9;

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

    inline constexpr double CarvedFloorThicknessUnits = 40.0;
    inline constexpr double CarvedChamberDepthUnits = 255.0;
    inline constexpr double DefaultEngineCubeSizeUnits = 100.0;

    /** Mountain surface for this block column (floor 1 lives at world Z=0, deeper Z blocks go down). */
    inline double GetBlockTopWorldZ(const FBlockCoord& BlockCoord)
    {
        return -static_cast<double>(BlockCoord.Z) * static_cast<double>(BlockSizeUnits);
    }

    inline double GetBlockBottomWorldZ(const FBlockCoord& BlockCoord)
    {
        return GetBlockTopWorldZ(BlockCoord) - static_cast<double>(BlockSizeUnits);
    }

    inline FVector GetFootprintWorldCenter(
        const FBlockCoord& OriginBlock,
        const int32 WidthBlocks,
        const int32 HeightBlocks
    )
    {
        const FVector OriginCorner = BlockToWorld(OriginBlock);
        return FVector(
            OriginCorner.X + (static_cast<double>(WidthBlocks) * static_cast<double>(BlockSizeUnits) * 0.5),
            OriginCorner.Y + (static_cast<double>(HeightBlocks) * static_cast<double>(BlockSizeUnits) * 0.5),
            0.0
        );
    }

    inline double MeshHalfHeightFromInstanceScaleZ(const double InstanceScaleZ)
    {
        return (DefaultEngineCubeSizeUnits * InstanceScaleZ) * 0.5;
    }

    inline FVector GetBlockColumnWorldXY(const FBlockCoord& BlockCoord)
    {
        const FVector BlockCorner = BlockToWorld(BlockCoord);
        return FVector(
            BlockCorner.X + static_cast<double>(HalfBlockSizeUnits),
            BlockCorner.Y + static_cast<double>(HalfBlockSizeUnits),
            0.0
        );
    }

    /** Solid rock column: top face flush with the block surface (nothing sticks above ground). */
    inline FVector GetRockVisualWorldCenter(const FBlockCoord& BlockCoord, const double InstanceScaleZ)
    {
        const FVector ColumnXY = GetBlockColumnWorldXY(BlockCoord);
        const double SurfaceZ = GetBlockTopWorldZ(BlockCoord);
        const double HalfHeight = MeshHalfHeightFromInstanceScaleZ(InstanceScaleZ);
        return FVector(ColumnXY.X, ColumnXY.Y, SurfaceZ - HalfHeight);
    }

    inline double GetCarvedFloorTopWorldZ(const FBlockCoord& BlockCoord)
    {
        const double SurfaceZ = GetBlockTopWorldZ(BlockCoord);
        const double BottomZ = GetBlockBottomWorldZ(BlockCoord);
        const double DeepestFloorTop = BottomZ + CarvedFloorThicknessUnits;
        const double TargetFloorTop = SurfaceZ - CarvedChamberDepthUnits;
        return FMath::Max(DeepestFloorTop, TargetFloorTop);
    }

    /** Carved room/tunnel floor slab at the bottom of the dig volume (below world surface). */
    inline FVector GetCarvedFloorVisualWorldCenter(const FBlockCoord& BlockCoord, const double FloorInstanceScaleZ)
    {
        const FVector ColumnXY = GetBlockColumnWorldXY(BlockCoord);
        const double FloorTopZ = GetCarvedFloorTopWorldZ(BlockCoord);
        const double FloorHalfHeight = MeshHalfHeightFromInstanceScaleZ(FloorInstanceScaleZ);
        return FVector(ColumnXY.X, ColumnXY.Y, FloorTopZ - FloorHalfHeight);
    }

    inline FVector GetCarvedFloorVisualWorldCenterForFootprint(
        const FBlockCoord& OriginBlock,
        const int32 WidthBlocks,
        const int32 HeightBlocks,
        const double FloorInstanceScaleZ
    )
    {
        const FVector FootprintCenter = GetFootprintWorldCenter(OriginBlock, WidthBlocks, HeightBlocks);
        const double FloorTopZ = GetCarvedFloorTopWorldZ(OriginBlock);
        const double FloorHalfHeight = MeshHalfHeightFromInstanceScaleZ(FloorInstanceScaleZ);
        return FVector(FootprintCenter.X, FootprintCenter.Y, FloorTopZ - FloorHalfHeight);
    }

    inline double GetCarvedWalkableWorldZ(const FBlockCoord& BlockCoord)
    {
        return GetCarvedFloorTopWorldZ(BlockCoord);
    }

    /** Starting chamber floor is the same carved floor plane as any dug tile on floor 1. */
    inline double GetCoreChamberSurfaceVisualFloorZ()
    {
        return GetCarvedFloorTopWorldZ(FBlockCoord(0, 0, 0));
    }

    /** Dungeon core crystal: ~6 ft tall, floating above the chamber floor, top tip below the mountain surface. */
    inline double GetDungeonCoreCrystalHeightUnits()
    {
        return 180.0;
    }

    inline double GetDungeonCoreCrystalHoverUnits()
    {
        return 25.0;
    }

    inline FLinearColor GetDungeonCoreGlowColor()
    {
        return FLinearColor(1.0f, 0.62f, 0.25f, 1.0f);
    }

    inline float GetDungeonCoreGlowIntensityCandelas()
    {
        return 40.0f;
    }

    inline float GetDungeonCoreGlowAttenuationRadiusUnits()
    {
        return 1200.0f;
    }

    inline float GetDungeonCoreSpinDegreesPerSecond()
    {
        return 20.0f;
    }

    /** SM_Crystal_D_01 is authored as a hanging crystal (point down); roll it so the point faces up. */
    inline FRotator GetDungeonCoreCrystalMeshRotation()
    {
        return FRotator(0.0, 0.0, 180.0);
    }

    /** MI_GlowCrystalls parameters: pale translucent shell with a warm flame-like interior. */
    inline FLinearColor GetDungeonCoreCrystalShellColor()
    {
        return FLinearColor(0.92f, 0.94f, 1.0f, 1.0f);
    }

    inline FLinearColor GetDungeonCoreCrystalInnerColor()
    {
        return FLinearColor(1.0f, 0.55f, 0.18f, 1.0f);
    }

    inline float GetDungeonCoreCrystalInnerGlow()
    {
        return 6.0f;
    }

    /** Dig preview on solid rock: a thin highlight on the rock's top face, never a block above ground. */
    inline FVector GetDigSurfaceHighlightWorldCenter(const FBlockCoord& BlockCoord)
    {
        const FVector ColumnXY = GetBlockColumnWorldXY(BlockCoord);
        return FVector(ColumnXY.X, ColumnXY.Y, GetMountainSurfaceWorldZ() + GridLineHeightUnits);
    }

    inline double GetDigSurfaceHighlightScaleZ()
    {
        return GridLineHeightUnits / DefaultEngineCubeSizeUnits;
    }

    /** Fills the dug volume between mountain surface and carved floor (reads as a pit, not a raised slab). */
    inline FVector GetCarvedCavityVisualWorldCenter(const FBlockCoord& BlockCoord, FVector& OutInstanceScale)
    {
        const FVector ColumnXY = GetBlockColumnWorldXY(BlockCoord);
        const double SurfaceZ = GetBlockTopWorldZ(BlockCoord);
        const double FloorTopZ = GetCarvedFloorTopWorldZ(BlockCoord);
        const double CavityDepth = FMath::Max(1.0, SurfaceZ - FloorTopZ);
        const double BlockMeshScale = static_cast<double>(BlockSizeUnits) / DefaultEngineCubeSizeUnits;
        OutInstanceScale = FVector(BlockMeshScale, BlockMeshScale, CavityDepth / DefaultEngineCubeSizeUnits);
        return FVector(ColumnXY.X, ColumnXY.Y, FloorTopZ + (CavityDepth * 0.5));
    }

    inline bool IsCarvedTileState(const ETileState TileState)
    {
        return TileState == ETileState::Cleared
            || TileState == ETileState::Room
            || TileState == ETileState::Trap
            || TileState == ETileState::SurfaceEntry;
    }

    /** Core pillar inside the carved chamber (does not extend above the surface). */
    inline FVector GetDungeonCorePillarVisual(
        const FBlockCoord& BlockCoord,
        const double InstanceScaleXY,
        FVector& OutInstanceScale
    )
    {
        const FVector ColumnXY = GetBlockColumnWorldXY(BlockCoord);
        const double FloorTopZ = GetCarvedFloorTopWorldZ(BlockCoord);
        const double PillarTopZ = GetBlockTopWorldZ(BlockCoord) - 20.0;
        const double PillarHeight = FMath::Max(40.0, PillarTopZ - FloorTopZ);
        const double PillarScaleZ = PillarHeight / DefaultEngineCubeSizeUnits;
        OutInstanceScale = FVector(InstanceScaleXY, InstanceScaleXY, PillarScaleZ);
        return FVector(ColumnXY.X, ColumnXY.Y, FloorTopZ + (PillarHeight * 0.5));
    }

    inline FVector GetCoreChamberCenterWorldLocation()
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetCoreChamberBlockBounds(MinX, MaxX, MinY, MaxY);
        const FVector Center = GetInclusiveBlockBoundsWorldCenter(MinX, MinY, MaxX, MaxY);
        return FVector(Center.X, Center.Y, 0.0);
    }

    inline float GetCameraPitchDegrees()
    {
        return -87.0f;
    }

    inline float GetCameraYawDegrees()
    {
        return 0.0f;
    }

    /** Fixed exposure for the build camera; auto-exposure keys on the dark rock and washes everything out. */
    inline float GetCameraManualExposureBiasEV()
    {
        return 9.0f;
    }

    /** Close enough to inspect a single 2 x 2 block room. */
    inline float GetCameraMinSpringArmLength()
    {
        return 1200.0f;
    }

    inline float GetCameraMaxSpringArmLength()
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetStartingBuildAreaBlockBounds(MinX, MaxX, MinY, MaxY);
        const double BlocksAcross = static_cast<double>(FMath::Max(MaxX - MinX + 1, MaxY - MinY + 1));
        const double WorldSpan = BlocksAcross * static_cast<double>(BlockSizeUnits);
        return static_cast<float>(WorldSpan * 2.25);
    }

    inline float GetCameraDefaultSpringArmLength()
    {
        int32 MinX = 0;
        int32 MaxX = 0;
        int32 MinY = 0;
        int32 MaxY = 0;
        GetStartingBuildAreaBlockBounds(MinX, MaxX, MinY, MaxY);
        const double BlocksAcross = static_cast<double>(FMath::Max(MaxX - MinX + 1, MaxY - MinY + 1));
        const double WorldSpan = BlocksAcross * static_cast<double>(BlockSizeUnits);
        return static_cast<float>(WorldSpan * 1.08);
    }

    inline FBlockCoord GetStartingCoreBlockCoord()
    {
        return FBlockCoord(
            (StartingCoreChamberWidthBlocks - 1) / 2,
            (StartingCoreChamberHeightBlocks - 1) / 2,
            0
        );
    }

    inline FVector GetDungeonCoreBlockWorldCenter()
    {
        const FBlockCoord CoreBlock = GetStartingCoreBlockCoord();
        const FVector ColumnXY = GetBlockColumnWorldXY(CoreBlock);
        return FVector(ColumnXY.X, ColumnXY.Y, GetCarvedWalkableWorldZ(CoreBlock));
    }

    inline int32 WorldLocationToBlockZ(const double WorldZ)
    {
        const int32 BlockZ = FMath::FloorToInt((GetBlockTopWorldZ(FBlockCoord(0, 0, 0)) - WorldZ) / static_cast<double>(BlockSizeUnits));
        return FMath::Max(0, BlockZ);
    }

    inline FBlockCoord WorldToBlock(const FVector& WorldLocation)
    {
        return FBlockCoord(
            FMath::FloorToInt(WorldLocation.X / static_cast<double>(BlockSizeUnits)),
            FMath::FloorToInt(WorldLocation.Y / static_cast<double>(BlockSizeUnits)),
            WorldLocationToBlockZ(WorldLocation.Z)
        );
    }
}
