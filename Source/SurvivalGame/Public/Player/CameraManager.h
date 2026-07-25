#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraManager.generated.h"

class USpringArmComponent;
class UCameraComponent;

/**
 * Sistem #2 — Kamera mantığı sahibi. Sahne bileşenlerini (SpringArm/Camera) SAHİPLENMEZ —
 * onlar APlayerCharacter'ın alt nesneleridir (attachment hiyerarşisi bir UActorComponent'e
 * bağlanamaz, USceneComponent gerekir). Bu bileşen yalnızca onların DAVRANIŞINI yönetir:
 * zoom, gelecekte mod geçişleri. RegisterCameraRig ile Character BeginPlay'de bağlanır.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UCameraManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraManager();

	/** Character BeginPlay'de kendi SpringArm/Camera'sını buraya kaydeder. */
	void RegisterCameraRig(USpringArmComponent* InSpringArm, UCameraComponent* InCamera);

	/** Fare tekerleği girişini zoom'a uygula (WheelDelta tipik olarak -1/0/+1). */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyZoomInput(float WheelDelta);

	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetCurrentArmLength() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "0.0"))
	float ZoomStepSize = 40.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "0.0"))
	float MinArmLength = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Zoom", meta = (ClampMin = "0.0"))
	float MaxArmLength = 600.0f;

	UPROPERTY()
	TWeakObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY()
	TWeakObjectPtr<UCameraComponent> Camera;
};
