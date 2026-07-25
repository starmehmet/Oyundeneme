#include "Production/ProductionRecipeDatabase.h"
#include "Production/ProductionRecipeDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void UProductionRecipeDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UProductionRecipeDatabaseSettings* Settings = GetDefault<UProductionRecipeDatabaseSettings>();
	LoadedTable = Settings->ProductionRecipeTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvivalProduction, Warning,
			TEXT("ProductionRecipeDatabase: DT_ProductionRecipes atanmamis (Project Settings > Game > Production Recipe Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvivalProduction, Log, TEXT("ProductionRecipeDatabase hazir: %d tarif yuklendi"), Cache.Num());
}

void UProductionRecipeDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FProductionRecipe>(TEXT("UProductionRecipeDatabase::RebuildCache"),
		[this](const FName& RowName, const FProductionRecipe& Row)
		{
			Cache.Add(RowName, Row);
		});
}

bool UProductionRecipeDatabase::FindRecipe(FName RecipeID, FProductionRecipe& OutRecipe) const
{
	if (const FProductionRecipe* Found = Cache.Find(RecipeID))
	{
		OutRecipe = *Found;
		return true;
	}
	return false;
}
