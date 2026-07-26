#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableBase.h"
#include "HarvestNode.generated.h"

/**
 * Sistem #29 — AInteractableBase'in ilk somut alt sinifi: dunyada yerlestirilmis, E ile
 * toplanabilen ham-madde noktasi (agac, kaya, vb.). Tur farkliligi (Agac/Kaya/...) tamamen
 * DT_HarvestNodes satirlarindan gelir — v1'de sifir yeni C++ sinifi gerektirir (bkz.
 * Docs/MIMARI.md #29). Kendi tick'i YOK — yeniden-dogma UHarvestNodeManager'da (kare-bolumlemeli).
 */
UCLASS()
class SURVIVALGAME_API AHarvestNode : public AInteractableBase
{
	GENERATED_BODY()

public:
	AHarvestNode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual bool CanInteract_Implementation(APlayerCharacter* Interactor) const override;
	virtual void OnInteract_Implementation(APlayerCharacter* Interactor) override;

	/** UHarvestNodeManager'in yeniden-dogma sirasinda cagirdigi durum sifirlama — Manager
	 * kayit defterine (DepletedNodes) DOKUNMAZ, o sorumluluk cagiran tarafta kalir (bkz. .cpp). */
	void Respawn();

	UFUNCTION(BlueprintPure, Category = "Harvest")
	FName GetNodeID() const { return NodeID; }

	/** PIE/test dogrulamasi icin (MCP get_properties ile okunur, InteractionCount deseniyle ayni). */
	UFUNCTION(BlueprintPure, Category = "Harvest")
	bool IsDepleted() const { return bDepleted; }

	UFUNCTION(BlueprintPure, Category = "Harvest")
	int32 GetRemainingHarvests() const { return RemainingHarvests; }

	double GetDepletionGameTime() const { return DepletionGameTime; }
	float GetRespawnSeconds() const { return CachedRespawnSeconds; }

protected:
	/** DT_HarvestNodes satir anahtari. */
	UPROPERTY(EditAnywhere, Category = "Harvest")
	FName NodeID;

	// BlueprintReadOnly private uyede UHT hatasi verir (InventoryComponent::CurrentWeight ile ayni
	// bilinen kisit) — bu yuzden protected, zaten GetRemainingHarvests()/IsDepleted() ile public okunuyor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	int32 RemainingHarvests = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	bool bDepleted = false;

private:
	double DepletionGameTime = 0.0;
	float CachedRespawnSeconds = 60.0f;
};
