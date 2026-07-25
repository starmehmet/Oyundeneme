#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Crafting/RecipeDefinition.h"
#include "RecipeDatabase.generated.h"

class UDataTable;

/**
 * Sistem #6 — Tarif tanımlarının tek sahibi. UItemDatabase ile birebir aynı desen:
 * DT_Recipes'i (URecipeDatabaseSettings üzerinden) yükler, FName→FRecipeDefinition
 * önbelleği kurar (satırların KOPYASI, ham işaretçi değil).
 */
UCLASS()
class SURVIVALGAME_API URecipeDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool FindRecipe(FName RecipeID, FRecipeDefinition& OutRecipe) const;

	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool IsValidRecipeID(FName RecipeID) const { return Cache.Contains(RecipeID); }

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FName> FindRecipesByWorkstation(const FString& WorkstationType) const;

	UFUNCTION(BlueprintPure, Category = "Crafting")
	int32 GetRecipeCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<FName, FRecipeDefinition> Cache;
};
