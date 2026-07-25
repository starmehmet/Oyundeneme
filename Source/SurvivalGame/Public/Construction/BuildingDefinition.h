#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Crafting/RecipeDefinition.h"
#include "BuildingDefinition.generated.h"

class ABuildingBase;

/**
 * Sistem #7 — İnşaat: tek bir bina tanımı (DT_Buildings satırı). Kimlik, DataTable
 * satır adının kendisi (FName) — ItemDefinition/RecipeDefinition ile aynı
 * tek-doğruluk-kaynağı deseni.
 *
 * RequiredMaterials, MIMARI.md taslağındaki TArray<FInventorySlot> YERİNE
 * TArray<FRecipeIngredient> kullanır (Crafting'den yeniden kullanılır) — FInventorySlot
 * çalışma-zamanı slot durumu (Durability dahil) taşır, bir "gereksinim" için anlamsız;
 * FRecipeIngredient (ItemID+Count) tam olarak istenen şey (bkz. ADR).
 *
 * ConstructionTime ve Dimensions şimdilik yalnızca VERİ — bu sistemde TÜKETİLMİYOR
 * (inşaat ANLIK tamamlanır, çok hücreli yerleşim yok — bkz. ADR, RecipeDefinition'daki
 * WorkstationType ile aynı "veri var, mekanizma yok" desenine paralel).
 */
USTRUCT(BlueprintType)
struct FBuildingDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TArray<FRecipeIngredient> RequiredMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building", meta = (ClampMin = "0.0"))
	float ConstructionTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FVector Dimensions = FVector(400.0f, 400.0f, 400.0f);

	/**
	 * Faz 1 entegrasyon borcu (Sistem #7/#9): bos ise `UConstructionComponent::ConfirmPlacement`
	 * taban `ABuildingBase` spawn eder (mevcut satırlarla TAM geriye-uyumlu) — dolu ise (ör.
	 * `AProductionMachine`) o alt sınıf spawn edilir, oyuncu artık üretim makinelerini de
	 * inşaat sistemiyle yerleştirebiliyor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TSubclassOf<ABuildingBase> BuildingClass;

	/**
	 * Yalnızca `BuildingClass` bir `AProductionMachine` (veya alt sınıfı) ise anlamlı —
	 * yerleşince `AProductionMachine::BeginConstruction`'da `AvailableRecipeIDs`'e kopyalanır.
	 * `ABuildingBase` için (BuildingClass boşken) okunmaz.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TArray<FName> ProductionRecipeIDs;

	/**
	 * RequiredMaterials'taki AYNI ItemID'ye sahip satırları toplayarak döner —
	 * FRecipeDefinition::GetAggregatedIngredients ile birebir aynı gerekçe
	 * (tekrarlanan ItemID'de satır-satır kontrol gercek maliyeti dusuk gosterir).
	 */
	TMap<FName, int32> GetAggregatedRequirements() const
	{
		TMap<FName, int32> Result;
		for (const FRecipeIngredient& Req : RequiredMaterials)
		{
			Result.FindOrAdd(Req.ItemID) += Req.Count;
		}
		return Result;
	}
};
