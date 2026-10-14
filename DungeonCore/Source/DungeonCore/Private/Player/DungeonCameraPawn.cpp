#include "Player/DungeonCameraPawn.h"

#include "Sim/DungeonTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"

ADungeonCameraPawn::ADungeonCameraPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComponent->SetupAttachment(SceneRoot);
    SpringArmComponent->bDoCollisionTest = false;
    SpringArmComponent->TargetArmLength = DungeonCore::GetCameraDefaultSpringArmLength();
    SpringArmComponent->SetRelativeRotation(
        FRotator(DungeonCore::GetCameraPitchDegrees(), DungeonCore::GetCameraYawDegrees(), 0.0f)
    );

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;
    CameraComponent->PostProcessSettings.bOverride_AutoExposureMethod = true;
    CameraComponent->PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    CameraComponent->PostProcessSettings.bOverride_AutoExposureBias = true;
    CameraComponent->PostProcessSettings.AutoExposureBias = DungeonCore::GetCameraManualExposureBiasEV();

    SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
}

void ADungeonCameraPawn::PanInWorldSpace(const FVector& WorldOffset)
{
    SetActorLocation(GetActorLocation() + WorldOffset);
}

void ADungeonCameraPawn::AdjustZoom(const float ZoomDelta)
{
    const float TargetArmLength = FMath::Clamp(
        SpringArmComponent->TargetArmLength + ZoomDelta,
        DungeonCore::GetCameraMinSpringArmLength(),
        DungeonCore::GetCameraMaxSpringArmLength()
    );
    SpringArmComponent->TargetArmLength = TargetArmLength;
}

void ADungeonCameraPawn::AddPanFromScreenDelta(const float ScreenDeltaX, const float ScreenDeltaY)
{
    const float PanUnitsPerPixel = SpringArmComponent->TargetArmLength * 0.0015f;
    const FVector Right = GetCameraPlanarRight();
    const FVector Forward = GetCameraPlanarForward();
    const FVector Offset = (-Right * ScreenDeltaX - Forward * ScreenDeltaY) * PanUnitsPerPixel;
    AddActorWorldOffset(Offset, false);
}

void ADungeonCameraPawn::AddOrbitFromScreenDelta(const float ScreenDeltaX, const float ScreenDeltaY)
{
    const float YawDegreesPerPixel = 0.35f;
    const float PitchDegreesPerPixel = 0.28f;

    FRotator ArmRotation = SpringArmComponent->GetRelativeRotation();
    ArmRotation.Yaw += ScreenDeltaX * YawDegreesPerPixel;
    ArmRotation.Pitch = FMath::Clamp(
        ArmRotation.Pitch + ScreenDeltaY * PitchDegreesPerPixel,
        -89.0f,
        -20.0f
    );
    SpringArmComponent->SetRelativeRotation(ArmRotation);
}

void ADungeonCameraPawn::RecenterOnDungeonCore()
{
    SetActorLocation(DungeonCore::GetCoreChamberCenterWorldLocation());
    SpringArmComponent->TargetArmLength = DungeonCore::GetCameraDefaultSpringArmLength();
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
