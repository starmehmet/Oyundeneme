#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCameraManager;
class UInteractionComponent;
class UInventoryComponent;
class UCraftingComponent;
class UConstructionComponent;
class UHealthComponent;
class UHUDController;

/**
 * Sistem #2 — Oyuncu karakteri. Üçüncü şahıs hareket + kamera kabuğu.
 * Hareket/bakış girişi ASurvivalPlayerController'dan Enhanced Input ile gelir ve
 * AddMovementInput/AddControllerYawInput gibi APawn/ACharacter native API'lerine
 * yönlendirilir — ayrı bir input-buffer katmanı YOK: ControlInputVector zaten
 * motor tarafından her Character tick'inde birikip tüketiliyor (bkz. ADR).
 */
UCLASS()
class SURVIVALGAME_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	/** IA_Move'dan gelen 2D eksen değeri (X=sağ/sol, Y=ileri/geri), kontrolör yaw'ına göre dünya uzayına çevrilir. */
	void ApplyMoveInput(const FVector2D& AxisValue);

	/** IA_Look'tan gelen 2D eksen değeri (X=yaw, Y=pitch). */
	void ApplyLookInput(const FVector2D& AxisValue);

	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraManager* GetCameraManager() const { return CameraManagerComp; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UInteractionComponent* GetInteractionComponent() const { return InteractionComp; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComp; }

	UFUNCTION(BlueprintPure, Category = "Crafting")
	UCraftingComponent* GetCraftingComponent() const { return CraftingComp; }

	UFUNCTION(BlueprintPure, Category = "Construction")
	UConstructionComponent* GetConstructionComponent() const { return ConstructionComp; }

	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const { return HealthComp; }

	UFUNCTION(BlueprintPure, Category = "UI")
	UHUDController* GetHUDController() const { return HUDControllerComp; }

	/** Sistem #12 — Vücut sıcaklığı (°C). Gerçek sürüklenme/hasar mantığı UTemperatureSimulation'da. */
	UFUNCTION(BlueprintPure, Category = "Temperature")
	float GetBodyTemperature() const { return BodyTemperature; }

	UFUNCTION(BlueprintCallable, Category = "Temperature")
	void SetBodyTemperature(float NewTemperature) { BodyTemperature = NewTemperature; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraManager> CameraManagerComp;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UInteractionComponent> InteractionComp;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere, Category = "Crafting")
	TObjectPtr<UCraftingComponent> CraftingComp;

	UPROPERTY(VisibleAnywhere, Category = "Construction")
	TObjectPtr<UConstructionComponent> ConstructionComp;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UHUDController> HUDControllerComp;

protected:
	// BlueprintReadOnly private uyede UHT hatasi verir (InventoryComponent::CurrentWeight /
	// ResourceSimulation::EnergyBudget ile ayni desen) — PIE/MCP dogrulamasi icin protected.
	// GetBodyTemperature() zaten public getter.
	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float BodyTemperature = 37.0f;
};
