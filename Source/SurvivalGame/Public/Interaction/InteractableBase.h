#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "InteractableBase.generated.h"

class UStaticMeshComponent;

/**
 * Sistem #3 — C++ etkileşilebilir taban sınıfı. İçerik nesneleri (kapı, sandık,
 * toplama noktası) bundan ya da doğrudan arayüzden türer.
 *
 * Çarpışma sözleşmesi: ECC_Interaction kanalının varsayılan yanıtı Ignore'dur
 * (DefaultEngine.ini) — yalnızca etkileşilebilirler Block ile kaydolur; bu yüzden
 * constructor mesh'e açıkça Block yanıtı verir. Türeyen sınıflar bunu bozmamalı.
 */
UCLASS()
class SURVIVALGAME_API AInteractableBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractableBase();

	// IInteractableInterface
	virtual bool CanInteract_Implementation(APlayerCharacter* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual float GetInteractionDistance_Implementation() const override;
	virtual void OnInteract_Implementation(APlayerCharacter* Interactor) override;

	/** PIE/test dogrulamasi icin: kac kez etkilesildi (MCP get_properties ile okunur). */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	int32 GetInteractionCount() const { return InteractionCount; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText InteractionPrompt;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "50.0"))
	float InteractionDistance = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	int32 InteractionCount = 0;
};
