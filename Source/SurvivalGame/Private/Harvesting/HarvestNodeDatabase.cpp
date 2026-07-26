#include "Harvesting/HarvestNodeDatabase.h"
#include "Harvesting/HarvestNodeDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void UHarvestNodeDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UHarvestNodeDatabaseSettings* Settings = GetDefault<UHarvestNodeDatabaseSettings>();
	LoadedTable = Settings->HarvestNodeTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("HarvestNodeDatabase: DT_HarvestNodes atanmamis (Project Settings > Game > Harvest Node Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvival, Log, TEXT("HarvestNodeDatabase hazir: %d dugum tanimi yuklendi"), Cache.Num());
}

void UHarvestNodeDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FHarvestNodeDefinition>(TEXT("UHarvestNodeDatabase::RebuildCache"),
		[this](const FName& RowName, const FHarvestNodeDefinition& Row)
		{
			Cache.Add(RowName, Row);
		});
}

bool UHarvestNodeDatabase::FindNodeDefinition(FName NodeID, FHarvestNodeDefinition& OutDefinition) const
{
	if (const FHarvestNodeDefinition* Found = Cache.Find(NodeID))
	{
		OutDefinition = *Found;
		return true;
	}
	return false;
}
