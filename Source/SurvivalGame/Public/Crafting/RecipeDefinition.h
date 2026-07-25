#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RecipeDefinition.generated.h"

USTRUCT(BlueprintType)
struct FRecipeIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "1"))
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct FRecipeOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "1"))
	int32 Count = 1;
};

/**
 * Sistem #6 — Zanaat: tek bir tarifin verisi (DT_Recipes satırı). Kimlik, DataTable
 * satır adının kendisi (FName) — ItemDefinition ile aynı tek-doğruluk-kaynağı deseni.
 *
 * WorkstationType şimdilik yalnızca VERİ — CraftingComponent bunu ZORUNLU KILMAZ
 * (istasyon-yakınlık kısıtlaması bu sistemin kapsamında değil, bkz. ADR). Boş string
 * = elde yapılabilir tarif. Gelecekteki üretim/inşaat sistemleri bu alanı tüketebilir.
 */
USTRUCT(BlueprintType)
struct FRecipeDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FText RecipeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<FRecipeIngredient> Ingredients;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<FRecipeOutput> Outputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0.0"))
	float CraftingTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FString WorkstationType;

	/**
	 * Ingredients dizisindeki AYNI ItemID'ye sahip satırları toplayarak döner
	 * (ör. iki ayrı satırda {Wood,5} + {Wood,3} → {Wood,8}). CanCraftRecipe/
	 * StartCrafting/CancelCrafting TUTARLI davransın diye tek yerde hesaplanır —
	 * satır satır bağımsız kontrol, tekrarlanan ItemID'de gerçek maliyeti
	 * olduğundan düşük gösterirdi (inceleme bulgusu).
	 */
	TMap<FName, int32> GetAggregatedIngredients() const
	{
		TMap<FName, int32> Result;
		for (const FRecipeIngredient& Ing : Ingredients)
		{
			Result.FindOrAdd(Ing.ItemID) += Ing.Count;
		}
		return Result;
	}
};
