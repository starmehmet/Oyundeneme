#include "Player/CameraManager.h"
#include "Player/CameraMath.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

UCameraManager::UCameraManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraManager::RegisterCameraRig(USpringArmComponent* InSpringArm, UCameraComponent* InCamera)
{
	SpringArm = InSpringArm;
	Camera = InCamera;
}

void UCameraManager::ApplyZoomInput(float WheelDelta)
{
	USpringArmComponent* Arm = SpringArm.Get();
	if (!Arm)
	{
		return;
	}
	Arm->TargetArmLength = SurvivalCamera::ApplyZoomStep(
		Arm->TargetArmLength, WheelDelta, ZoomStepSize, MinArmLength, MaxArmLength);
}

float UCameraManager::GetCurrentArmLength() const
{
	const USpringArmComponent* Arm = SpringArm.Get();
	return Arm ? Arm->TargetArmLength : 0.0f;
}
