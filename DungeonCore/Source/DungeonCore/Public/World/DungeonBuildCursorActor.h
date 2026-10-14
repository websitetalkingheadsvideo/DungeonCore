#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/DungeonTypes.h"

#include "DungeonBuildCursorActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class DUNGEONCORE_API ADungeonBuildCursorActor : public AActor
{
    GENERATED_BODY()

public:
    ADungeonBuildCursorActor();

    void SetPreviewMode(const EDungeonBuildMode InBuildMode);

    void SetPreviewOriginBlock(const FBlockCoord& InOriginBlock);

    void SetHoveredTileState(const ETileState InTileState);

    void SetPreviewValidity(const bool bIsValid);

private:
    FVector GetPreviewWorldLocation(const FBlockCoord& OriginBlock) const;

    FVector GetPreviewScaleForMode(const EDungeonBuildMode InBuildMode) const;

    FRoomFootprint GetFootprintForMode(const EDungeonBuildMode InBuildMode) const;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

    UPROPERTY()
    EDungeonBuildMode BuildMode = EDungeonBuildMode::Dig;

    UPROPERTY()
    FBlockCoord OriginBlock;

    UPROPERTY()
    ETileState HoveredTileState = ETileState::SolidRock;

    bool bPreviewValid = false;
};
