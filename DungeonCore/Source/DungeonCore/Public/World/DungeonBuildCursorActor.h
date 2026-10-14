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

    /** Shape the ghost after the selected catalog item: dig is a thin surface highlight, rooms/traps a floor slab. */
    void SetPreviewBuildable(const EBuildableCategory InCategory, const FRoomFootprint& InFootprint);

    void SetPreviewOriginBlock(const FBlockCoord& InOriginBlock);

    void SetHoveredTileState(const ETileState InTileState);

    void SetPreviewValidity(const bool bIsValid);

private:
    FVector GetPreviewWorldLocation(const FBlockCoord& InOriginBlock) const;

    FVector GetPreviewScale() const;

    bool IsDigOnSolidRock() const;

    void RefreshPreviewTransform();

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UStaticMeshComponent> PreviewMeshComponent;

    UPROPERTY()
    EBuildableCategory Category = EBuildableCategory::Dig;

    UPROPERTY()
    FRoomFootprint Footprint;

    UPROPERTY()
    FBlockCoord OriginBlock;

    UPROPERTY()
    ETileState HoveredTileState = ETileState::SolidRock;

    bool bPreviewValid = false;
};
