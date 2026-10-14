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

    UFUNCTION(BlueprintCallable, Category = "Dungeon")
    void RecenterOnDungeonCore();

protected:
    virtual void Tick(float DeltaSeconds) override;

private:
    void ApplyKeyboardPan(const float DeltaSeconds);

    void ApplyEdgePan(const float DeltaSeconds);

    void ApplyScrollZoom();

    FVector GetCameraPlanarForward() const;

    FVector GetCameraPlanarRight() const;
    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Dungeon")
    TObjectPtr<UCameraComponent> CameraComponent;
};
