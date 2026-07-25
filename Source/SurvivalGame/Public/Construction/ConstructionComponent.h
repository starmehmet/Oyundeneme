#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ConstructionComponent.generated.h"

class UInventoryComponent;
class AConstructionGhost;

/**
 * Sistem #7 — Yerleştirme akışının sahibi (oyuncu aktörüne eklenir). Aynı aktördeki
 * UInventoryComponent'i kullanır (malzeme kontrolü/tüketimi) — CraftingComponent ile
 * aynı desen (Sistem #6).
 *
 * Akış: StartPlacement (ghost spawn) → UpdateGhostPosition (her frame/fare hareketi,
 * ızgaraya yapışma + geçerlilik) → ConfirmPlacement (malzemeleri tüketir, binayı
 * yerleştirir) veya CancelPlacement (ghost'u malzeme harcamadan iptal eder).
 */
UCLASS(ClassGroup = (Construction), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UConstructionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UConstructionComponent();

	/** Yerleştirme önizlemesini başlatır (ghost spawn eder). Zaten aktif bir yerleştirme varsa önce iptal eder. */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool StartPlacement(FName BuildingID);

	/** Ghost'u verilen dünya konumuna taşır (X/Y ızgaraya yapışır), geçerliliği yeniden değerlendirir. */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void UpdateGhostPosition(const FVector& WorldLocation);

	/** Geçerliyse malzemeleri tüketir, binayı yerleştirir. Ghost her durumda (başarı/başarısızlık) temizlenir. */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool ConfirmPlacement();

	/** Ghost'u malzeme harcamadan iptal eder. */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void CancelPlacement();

	UFUNCTION(BlueprintPure, Category = "Construction")
	bool IsPlacementActive() const { return SpawnedGhost != nullptr; }

protected:
	virtual void BeginPlay() override;

private:
	/** Grid hücresi boş VE gerekli malzemeler envanterde mevcut mu? (CanCraftRecipe ile aynı desen) */
	bool EvaluatePlacementValidity(FName BuildingID, const FIntPoint& GridCoord) const;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> OwnerInventory;

	UPROPERTY()
	TObjectPtr<AConstructionGhost> SpawnedGhost;

	FName PendingBuildingID = NAME_None;
};
