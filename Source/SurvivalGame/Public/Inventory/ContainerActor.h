#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "ContainerActor.generated.h"

class UInventoryComponent;
class UStaticMeshComponent;

/**
 * Sistem #4 — Minimal test/içerik konteyneri: kendi envanterini tutar, etkileşilince
 * TÜMÜNÜ oyuncuya aktarır ("hepsini topla"). Envanter UI'ı (Sistem #20) gelene kadar
 * konteyner içeriğini görsel olarak seçip aktarma akışı yok — bu, TransferItemTo'yu
 * PIE'de gerçek bir sahne nesnesiyle sınayan en küçük anlamlı içerik.
 */
UCLASS()
class SURVIVALGAME_API AContainerActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AContainerActor();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventory() const { return Inventory; }

	// IInteractableInterface
	virtual bool CanInteract_Implementation(APlayerCharacter* Interactor) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual float GetInteractionDistance_Implementation() const override;
	virtual void OnInteract_Implementation(APlayerCharacter* Interactor) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float InteractionDistance = 250.0f;

	/** Seviye tasarımcısının sandığa koyduğu başlangıç içeriği (ItemID → Adet). */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TMap<FName, int32> StartingItems;
};
