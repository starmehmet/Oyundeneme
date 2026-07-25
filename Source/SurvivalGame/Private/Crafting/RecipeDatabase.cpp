#include "Crafting/RecipeDatabase.h"
#include "Crafting/RecipeDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void URecipeDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const URecipeDatabaseSettings* Settings = GetDefault<URecipeDatabaseSettings>();
	LoadedTable = Settings->RecipeTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("RecipeDatabase: DT_Recipes atanmamis (Project Settings > Game > Recipe Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvival, Log, TEXT("RecipeDatabase hazir: %d tarif yuklendi"), Cache.Num());
}

void URecipeDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FRecipeDefinition>(TEXT("URecipeDatabase::RebuildCache"),
		[this](const FName& RowName, const FRecipeDefinition& Row)
		{
			Cache.Add(RowName, Row);
		});
}

bool URecipeDatabase::FindRecipe(FName RecipeID, FRecipeDefinition& OutRecipe) const
{
	if (const FRecipeDefinition* Found = Cache.Find(RecipeID))
	{
		OutRecipe = *Found;
		return true;
	}
	return false;
}

TArray<FName> URecipeDatabase::FindRecipesByWorkstation(const FString& WorkstationType) const
{
	TArray<FName> Result;
	for (const TPair<FName, FRecipeDefinition>& Pair : Cache)
	{
		if (Pair.Value.WorkstationType == WorkstationType)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}
