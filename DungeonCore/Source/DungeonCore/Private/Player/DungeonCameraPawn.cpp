#include "Player/DungeonCameraPawn.h"

#include "Sim/DungeonTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

ADungeonCameraPawn::ADungeonCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(SceneRoot);
    SpringArmComponent->bDoCollisionTest = false;
    SpringArmComponent->TargetArmLength = 3600.0f;
    SpringArmComponent->SetRelativeRotation(FRotator(-60.0f, -45.0f, 0.0f));

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;

    SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
}

void ADungeonCameraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    ApplyKeyboardPan(DeltaSeconds);
    ApplyEdgePan(DeltaSeconds);
    ApplyScrollZoom();
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

void ADungeonCameraPawn::RecenterOnDungeonCore()
{
    // Core chamber is 20x20 from block (0,0); camera belongs over its center, not the corner tile.
    SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
    SpringArmComponent->TargetArmLength = 3600.0f;
}

void ADungeonCameraPawn::ApplyKeyboardPan(const float DeltaSeconds)
{
    const APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController == nullptr)
    {
        return;
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::F))
    {
        RecenterOnDungeonCore();
    }

    FVector MoveDelta(0.0, 0.0, 0.0);
    const float PanSpeed = 2200.0f * DeltaSeconds;
    if (PlayerController->IsInputKeyDown(EKeys::W))
    {
        MoveDelta += GetCameraPlanarForward() * PanSpeed;
    }
    if (PlayerController->IsInputKeyDown(EKeys::S))
    {
        MoveDelta -= GetCameraPlanarForward() * PanSpeed;
    }
    if (PlayerController->IsInputKeyDown(EKeys::D))
    {
        MoveDelta += GetCameraPlanarRight() * PanSpeed;
    }
    if (PlayerController->IsInputKeyDown(EKeys::A))
    {
        MoveDelta -= GetCameraPlanarRight() * PanSpeed;
    }

    if (!MoveDelta.IsNearlyZero())
    {
        AddActorWorldOffset(MoveDelta, false);
    }
}

void ADungeonCameraPawn::ApplyEdgePan(const float DeltaSeconds)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController == nullptr)
    {
        return;
    }

    int32 ViewportX = 0;
    int32 ViewportY = 0;
    PlayerController->GetViewportSize(ViewportX, ViewportY);
    if (ViewportX <= 0 || ViewportY <= 0)
    {
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    if (!PlayerController->GetMousePosition(MouseX, MouseY))
    {
        return;
    }

    const float EdgeThreshold = 20.0f;
    const float PanSpeed = 1600.0f * DeltaSeconds;
    FVector MoveDelta(0.0, 0.0, 0.0);

    if (MouseY <= EdgeThreshold)
    {
        MoveDelta += GetCameraPlanarForward() * PanSpeed;
    }
    else if (MouseY >= static_cast<float>(ViewportY) - EdgeThreshold)
    {
        MoveDelta -= GetCameraPlanarForward() * PanSpeed;
    }

    if (MouseX <= EdgeThreshold)
    {
        MoveDelta -= GetCameraPlanarRight() * PanSpeed;
    }
    else if (MouseX >= static_cast<float>(ViewportX) - EdgeThreshold)
    {
        MoveDelta += GetCameraPlanarRight() * PanSpeed;
    }

    if (!MoveDelta.IsNearlyZero())
    {
        AddActorWorldOffset(MoveDelta, false);
    }
}

void ADungeonCameraPawn::ApplyScrollZoom()
{
    const APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController == nullptr)
    {
        return;
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::MouseScrollUp))
    {
        AdjustZoom(-200.0f);
    }
    if (PlayerController->WasInputKeyJustPressed(EKeys::MouseScrollDown))
    {
        AdjustZoom(200.0f);
    }
}

FVector ADungeonCameraPawn::GetCameraPlanarForward() const
{
    const float YawDegrees = SpringArmComponent->GetComponentRotation().Yaw;
    return FRotationMatrix(FRotator(0.0f, YawDegrees, 0.0f)).GetUnitAxis(EAxis::X);
}

FVector ADungeonCameraPawn::GetCameraPlanarRight() const
{
    const float YawDegrees = SpringArmComponent->GetComponentRotation().Yaw;
    return FRotationMatrix(FRotator(0.0f, YawDegrees, 0.0f)).GetUnitAxis(EAxis::Y);
}
