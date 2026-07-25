#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SurvivalPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Sistem #2 — Oyuncu girişinin tek sahibi. Enhanced Input action'larını dinler,
 * mevcut pawn'a (APlayerCharacter) yönlendirir. MappingContext/Action referansları
 * EditDefaultsOnly — gerçek .uasset varlıkları ve değerleri EDİTÖRDE bir Blueprint
 * alt sınıfında (örn. BP_PlayerController) atanmalı; bu dosyalar ikili (binary)
 * olduğundan kod düzenleyiciyle oluşturulamaz (bkz. README kurulum notu).
 */
UCLASS()
class SURVIVALGAME_API ASurvivalPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void Handle_Move(const FInputActionValue& Value);
	void Handle_Look(const FInputActionValue& Value);
	void Handle_JumpStarted(const FInputActionValue& Value);
	void Handle_JumpCompleted(const FInputActionValue& Value);
	void Handle_Zoom(const FInputActionValue& Value);
	void Handle_Interact(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
};
