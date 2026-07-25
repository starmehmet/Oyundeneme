#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Production/ProductionRecipe.h"
#include "ProductionRecipeDatabase.generated.h"

class UDataTable;

/**
 * Sistem #9 — Üretim tariflerinin tek sahibi. DT_ProductionRecipes'i (üzerinden
 * UProductionRecipeDatabaseSettings) yükler, FName→FProductionRecipe önbelleği kurar.
 * Önbellek DataTable satırlarının KOPYASINI tutar (ham işaretçi değil) — UItemDatabase
 * ile aynı desen.
 */
UCLASS()
class SURVIVALGAME_API UProductionRecipeDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Production")
	bool FindRecipe(FName RecipeID, FProductionRecipe& OutRecipe) const;

	UFUNCTION(BlueprintPure, Category = "Production")
	bool IsValidRecipeID(FName RecipeID) const { return Cache.Contains(RecipeID); }

	UFUNCTION(BlueprintPure, Category = "Production")
	int32 GetRecipeCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<FName, FProductionRecipe> Cache;
};
