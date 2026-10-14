#include "Sim/Grid/DungeonGridState.h"

#include "Sim/DungeonTypes.h"
#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"

namespace DungeonGridRules
{
    bool IsInteriorPlaceableTile(const FTileData& TileData)
    {
        if (DungeonCore::IsDungeonCoreBlockCoord(TileData.BlockCoord))
        {
            return false;
        }

        if (TileData.TileState == ETileState::Cleared)
        {
            return true;
        }

        return TileData.TileState == ETileState::Room && TileData.RoomTypeId == ERoomTypeId::CoreChamber;
    }
}

void UDungeonGridState::InitializeGrid(const int32 InChunkSizeInBlocks)
{
    if (InChunkSizeInBlocks <= 0)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Chunk size must be greater than zero."));
    }

    ChunkSizeInBlocks = InChunkSizeInBlocks;
    TileMap.Reset();
}

void UDungeonGridState::ResetGrid()
{
    TileMap.Reset();
}

void UDungeonGridState::SetTileData(const FTileData& InTileData)
{
    TileMap.Add(InTileData.BlockCoord, InTileData);
}

bool UDungeonGridState::HasTileData(const FBlockCoord& BlockCoord) const
{
    return TileMap.Contains(BlockCoord);
}

const FTileData* UDungeonGridState::FindTileData(const FBlockCoord& BlockCoord) const
{
    return TileMap.Find(BlockCoord);
}

FTileData* UDungeonGridState::FindMutableTileData(const FBlockCoord& BlockCoord)
{
    return TileMap.Find(BlockCoord);
}

FPlacementResult UDungeonGridState::ValidateDig(
    const FBlockCoord& BlockCoord,
    const int32 CurrentMana,
    const int32 DigManaCost
) const
{
    FPlacementResult Result;
    Result.ManaCost = DigManaCost;
    Result.AffectedBlocks = { BlockCoord };

    if (!DungeonCore::IsBlockInStartingBuildArea(BlockCoord))
    {
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    const FTileData* ExistingTile = FindTileData(BlockCoord);
    if (ExistingTile == nullptr)
    {
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    if (CurrentMana < DigManaCost)
    {
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    if (!ExistingTile->bDiggable || ExistingTile->TileState != ETileState::SolidRock)
    {
        Result.FailureReason = EPlacementFailureReason::Blocked;
        return Result;
    }

    if (!HasAdjacentWalkableBlock(BlockCoord))
    {
        Result.FailureReason = EPlacementFailureReason::WouldDisconnectPath;
        return Result;
    }

    Result.bSuccess = true;
    return Result;
}

bool UDungeonGridState::ApplyDig(const FBlockCoord& BlockCoord, const bool bMakeSurfaceEntry)
{
    FTileData* MutableTile = FindMutableTileData(BlockCoord);
    if (MutableTile == nullptr)
    {
        return false;
    }

    MutableTile->TileState = bMakeSurfaceEntry ? ETileState::SurfaceEntry : ETileState::Cleared;
    MutableTile->RoomTypeId = ERoomTypeId::None;
    MutableTile->TrapTypeId = ETrapTypeId::None;
    MutableTile->bWalkable = true;
    MutableTile->bDiggable = false;
    return true;
}

FPlacementResult UDungeonGridState::ValidateRoomPlacement(
    const FBlockCoord& OriginBlock,
    const FRoomFootprint& Footprint,
    const int32 CurrentMana,
    const int32 ManaCost
) const
{
    FPlacementResult Result;
    Result.ManaCost = ManaCost;
    Result.AffectedBlocks = GetBlocksForFootprint(OriginBlock, Footprint);

    if (!Footprint.IsValid())
    {
        Result.FailureReason = EPlacementFailureReason::InvalidFootprint;
        return Result;
    }

    if (CurrentMana < ManaCost)
    {
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    for (const FBlockCoord& BlockCoord : Result.AffectedBlocks)
    {
        const FTileData* ExistingTile = FindTileData(BlockCoord);
        if (ExistingTile == nullptr)
        {
            Result.FailureReason = EPlacementFailureReason::OutOfBounds;
            return Result;
        }

        if (!DungeonGridRules::IsInteriorPlaceableTile(*ExistingTile))
        {
            Result.FailureReason = EPlacementFailureReason::RequiresClearedTile;
            return Result;
        }
    }

    Result.bSuccess = true;
    return Result;
}

bool UDungeonGridState::ApplyRoomPlacement(const FRoomInstanceData& RoomInstanceData)
{
    const TArray<FBlockCoord> Blocks = GetBlocksForFootprint(RoomInstanceData.OriginBlock, RoomInstanceData.Footprint);
    for (const FBlockCoord& BlockCoord : Blocks)
    {
        FTileData* MutableTile = FindMutableTileData(BlockCoord);
        if (MutableTile == nullptr)
        {
            return false;
        }

        MutableTile->TileState = ETileState::Room;
        MutableTile->RoomTypeId = RoomInstanceData.RoomTypeId;
        MutableTile->TrapTypeId = ETrapTypeId::None;
        MutableTile->bWalkable = true;
        MutableTile->bDiggable = false;
    }

    return true;
}

FPlacementResult UDungeonGridState::ValidateTrapPlacement(
    const FBlockCoord& OriginBlock,
    const int32 CurrentMana,
    const int32 ManaCost
) const
{
    FPlacementResult Result;
    Result.ManaCost = ManaCost;
    Result.AffectedBlocks = { OriginBlock };

    const FTileData* ExistingTile = FindTileData(OriginBlock);
    if (ExistingTile == nullptr)
    {
        Result.FailureReason = EPlacementFailureReason::OutOfBounds;
        return Result;
    }

    if (CurrentMana < ManaCost)
    {
        Result.FailureReason = EPlacementFailureReason::InsufficientMana;
        return Result;
    }

    if (!DungeonGridRules::IsInteriorPlaceableTile(*ExistingTile))
    {
        Result.FailureReason = EPlacementFailureReason::RequiresClearedTile;
        return Result;
    }

    Result.bSuccess = true;
    return Result;
}

bool UDungeonGridState::ApplyTrapPlacement(const FTrapInstanceData& TrapInstanceData)
{
    FTileData* MutableTile = FindMutableTileData(TrapInstanceData.OriginBlock);
    if (MutableTile == nullptr)
    {
        return false;
    }

    MutableTile->TileState = ETileState::Trap;
    MutableTile->TrapTypeId = TrapInstanceData.TrapTypeId;
    MutableTile->RoomTypeId = ERoomTypeId::None;
    MutableTile->bWalkable = true;
    MutableTile->bDiggable = false;
    return true;
}

bool UDungeonGridState::HasAdjacentWalkableBlock(const FBlockCoord& BlockCoord) const
{
    const TArray<FBlockCoord> NeighborOffsets = {
        FBlockCoord(1, 0, 0),
        FBlockCoord(-1, 0, 0),
        FBlockCoord(0, 1, 0),
        FBlockCoord(0, -1, 0)
    };

    for (const FBlockCoord& Offset : NeighborOffsets)
    {
        const FTileData* NeighborTile = FindTileData(BlockCoord + Offset);
        if (NeighborTile != nullptr && NeighborTile->bWalkable)
        {
            return true;
        }
    }

    return false;
}

TArray<FBlockCoord> UDungeonGridState::GetBlocksForFootprint(
    const FBlockCoord& OriginBlock,
    const FRoomFootprint& Footprint
) const
{
    TArray<FBlockCoord> Blocks;
    Blocks.Reserve(Footprint.Width * Footprint.Height * Footprint.Depth);

    for (int32 XOffset = 0; XOffset < Footprint.Width; ++XOffset)
    {
        for (int32 YOffset = 0; YOffset < Footprint.Height; ++YOffset)
        {
            for (int32 ZOffset = 0; ZOffset < Footprint.Depth; ++ZOffset)
            {
                Blocks.Add(FBlockCoord(
                    OriginBlock.X + XOffset,
                    OriginBlock.Y + YOffset,
                    OriginBlock.Z + ZOffset
                ));
            }
        }
    }

    return Blocks;
}

TArray<FTileData> UDungeonGridState::GetAllTiles() const
{
    TArray<FTileData> Tiles;
    TileMap.GenerateValueArray(Tiles);
    return Tiles;
}

FChunkCoord UDungeonGridState::GetChunkCoordForBlock(const FBlockCoord& BlockCoord) const
{
    return FChunkCoord(
        FMath::FloorToInt(static_cast<float>(BlockCoord.X) / static_cast<float>(ChunkSizeInBlocks)),
        FMath::FloorToInt(static_cast<float>(BlockCoord.Y) / static_cast<float>(ChunkSizeInBlocks)),
        FMath::FloorToInt(static_cast<float>(BlockCoord.Z) / static_cast<float>(ChunkSizeInBlocks))
    );
}
