#include "Player/DungeonCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

ADungeonCameraPawn::ADungeonCameraPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(SceneRoot);
    SpringArmComponent->bDoCollisionTest = false;
    SpringArmComponent->TargetArmLength = 2600.0f;
    SpringArmComponent->SetRelativeRotation(FRotator(-60.0f, -45.0f, 0.0f));

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;

    SetActorLocation(FVector(300.0f, 300.0f, 0.0f));
}

void ADungeonCameraPawn::PanInWorldSpace(const FVector& WorldOffset)
{
    SetActorLocation(GetActorLocation() + WorldOffset);
}

void ADungeonCameraPawn::AdjustZoom(const float ZoomDelta)
{
    const float TargetArmLength = FMath::Clamp(SpringArmComponent->TargetArmLength + ZoomDelta, 1200.0f, 5000.0f);
    SpringArmComponent->TargetArmLength = TargetArmLength;
}
