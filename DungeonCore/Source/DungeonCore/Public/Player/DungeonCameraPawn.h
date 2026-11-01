#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "DungeonCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

UCLASS()
class DUNGEONCORE_API ADungeonCameraPawn : public APawn
{
    GENERATED_BODY()

public:
    ADungeonCameraPawn();

    void PanInWorldSpace(const FVector& WorldOffset);

    void AdjustZoom(const float ZoomDelta);

    void AddPanFromScreenDelta(const float ScreenDeltaX, const float ScreenDeltaY);

    void AddOrbitFromScreenDelta(const float ScreenDeltaX, const float ScreenDeltaY);

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void RecenterOnDungeonCore();

    /** Three-quarter hero framing of the core chamber used behind the run setup screen. */
    void FrameForShowcase();

    /** Top-down build view at the default distance; undoes FrameForShowcase and any orbiting. */
    void ResetToBuildView();

private:
    FVector GetCameraPlanarForward() const;

    FVector GetCameraPlanarRight() const;
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UCameraComponent> CameraComponent;
};
