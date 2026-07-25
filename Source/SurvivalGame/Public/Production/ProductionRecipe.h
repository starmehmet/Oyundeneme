#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Crafting/RecipeDefinition.h"
#include "ProductionRecipe.generated.h"

/**
 * Sistem #9 — Bir üretim makinesi tarifi (DT_ProductionRecipes satırı). Kimlik, DataTable
 * satır adının kendisi (FName) — Item/Recipe/BuildingDefinition ile aynı tek-doğruluk-
 * kaynağı deseni.
 *
 * MIMARI.md taslağı `AProductionMachine::AvailableRecipes`'i bina üzerinde tutulan İNLİNE
 * bir `TArray<FProductionRecipe>` olarak gösteriyor — BİLEREK DataTable+Subsystem deseniyle
 * DEĞİŞTİRİLDİ (Item/Recipe/BuildingDatabase ile aynı): tarif verisi kodda/aktörde
 * DUPLICATE edilmez, `AProductionMachine` yalnızca hangi RecipeID'leri desteklediğini
 * (`TArray<FName>`) tutar, gerçek veriyi her zaman `UProductionRecipeDatabase`'den çözer
 * (CLAUDE.md: "Veri tabanlı içerik... asla koda gömülmez").
 *
 * Inputs/Outputs, Crafting'in `FRecipeIngredient`/`FRecipeOutput`'unu yeniden kullanır
 * (Sistem #7'nin `FBuildingDefinition::RequiredMaterials`'ta yaptığı gibi) — ItemID+Count
 * çifti zaten tam olarak istenen şey.
 */
USTRUCT(BlueprintType)
struct FProductionRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
	FText RecipeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
	TArray<FRecipeIngredient> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
	TArray<FRecipeOutput> Outputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (ClampMin = "0.0"))
	float ProductionTime = 1.0f;

	/** Saniyede tüketilen enerji. 0 = yakıtsız çalışır (Blocked_NoFuel asla tetiklenmez). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production", meta = (ClampMin = "0.0"))
	float EnergyPerSecond = 0.0f;

	/**
	 * Inputs dizisindeki AYNI ItemID'ye sahip satırları toplayarak döner —
	 * `FRecipeDefinition::GetAggregatedIngredients`/`FBuildingDefinition::GetAggregatedRequirements`
	 * ile birebir aynı gerekçe (tekrarlanan ItemID'de satır-satır kontrol gerçek maliyeti
	 * olduğundan düşük gösterir — Sistem #6/#7 inceleme bulgusu, burada baştan uygulanıyor).
	 */
	TMap<FName, int32> GetAggregatedInputs() const
	{
		TMap<FName, int32> Result;
		for (const FRecipeIngredient& In : Inputs)
		{
			Result.FindOrAdd(In.ItemID) += In.Count;
		}
		return Result;
	}

	/** GetAggregatedInputs ile aynı gerekçe, Outputs için — çıktı-alan kontrolü tekrarlanan
	 * ItemID'leri TEK toplam olarak değerlendirmeli (bkz. AProductionMachine::HasSufficientOutputRoom). */
	TMap<FName, int32> GetAggregatedOutputs() const
	{
		TMap<FName, int32> Result;
		for (const FRecipeOutput& Out : Outputs)
		{
			Result.FindOrAdd(Out.ItemID) += Out.Count;
		}
		return Result;
	}
};
