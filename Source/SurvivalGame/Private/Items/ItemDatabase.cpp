#include "Items/ItemDatabase.h"
#include "Items/ItemDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void UItemDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UItemDatabaseSettings* Settings = GetDefault<UItemDatabaseSettings>();
	LoadedTable = Settings->ItemDefinitionTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("ItemDatabase: DT_Items atanmamis (Project Settings > Game > Item Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvival, Log, TEXT("ItemDatabase hazir: %d ogem yuklendi"), Cache.Num());
}

void UItemDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FItemDefinition>(TEXT("UItemDatabase::RebuildCache"),
		[this](const FName& RowName, const FItemDefinition& Row)
		{
			Cache.Add(RowName, Row);
		});
}

bool UItemDatabase::FindItem(FName ItemID, FItemDefinition& OutDefinition) const
{
	if (const FItemDefinition* Found = Cache.Find(ItemID))
	{
		OutDefinition = *Found;
		return true;
	}
	return false;
}

TArray<FName> UItemDatabase::FindItemsByTag(const FString& Tag) const
{
	TArray<FName> Result;
	for (const TPair<FName, FItemDefinition>& Pair : Cache)
	{
		if (Pair.Value.HasTag(Tag))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

TArray<FName> UItemDatabase::FindItemsByCategory(EItemCategory Category) const
{
	TArray<FName> Result;
	for (const TPair<FName, FItemDefinition>& Pair : Cache)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}
