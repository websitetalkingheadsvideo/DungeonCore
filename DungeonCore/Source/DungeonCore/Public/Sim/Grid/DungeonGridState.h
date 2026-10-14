#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Sim/DungeonTypes.h"

#include "DungeonGridState.generated.h"

UCLASS()
class DUNGEONCORE_API UDungeonGridState : public UObject
{
    GENERATED_BODY()

public:
    void InitializeGrid(const int32 InChunkSizeInBlocks);

    void ResetGrid();

    void SetTileData(const FTileData& InTileData);

    bool HasTileData(const FBlockCoord& BlockCoord) const;

    const FTileData* FindTileData(const FBlockCoord& BlockCoord) const;

    FTileData* FindMutableTileData(const FBlockCoord& BlockCoord);

    FPlacementResult ValidateDig(const FBlockCoord& BlockCoord, const int32 CurrentMana, const int32 DigManaCost) const;

    bool ApplyDig(const FBlockCoord& BlockCoord, const bool bMakeSurfaceEntry);

    FPlacementResult ValidateRoomPlacement(
        const FBlockCoord& OriginBlock,
        const FRoomFootprint& Footprint,
        const int32 CurrentMana,
        const int32 ManaCost
    ) const;

    bool ApplyRoomPlacement(const FRoomInstanceData& RoomInstanceData);

    FPlacementResult ValidateTrapPlacement(const FBlockCoord& OriginBlock, const int32 CurrentMana, const int32 ManaCost) const;

    bool ApplyTrapPlacement(const FTrapInstanceData& TrapInstanceData);

    bool HasAdjacentWalkableBlock(const FBlockCoord& BlockCoord) const;

    TArray<FBlockCoord> GetBlocksForFootprint(const FBlockCoord& OriginBlock, const FRoomFootprint& Footprint) const;

    TArray<FTileData> GetAllTiles() const;

private:
    FChunkCoord GetChunkCoordForBlock(const FBlockCoord& BlockCoord) const;

    int32 ChunkSizeInBlocks = DungeonCore::DefaultChunkSizeInBlocks;

    UPROPERTY()
    TMap<FBlockCoord, FTileData> TileMap;
};
