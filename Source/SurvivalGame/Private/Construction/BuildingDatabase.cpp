#include "Construction/BuildingDatabase.h"
#include "Construction/BuildingDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void UBuildingDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UBuildingDatabaseSettings* Settings = GetDefault<UBuildingDatabaseSettings>();
	LoadedTable = Settings->BuildingDefinitionTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("BuildingDatabase: DT_Buildings atanmamis (Project Settings > Game > Building Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvival, Log, TEXT("BuildingDatabase hazir: %d bina yuklendi"), Cache.Num());
}

void UBuildingDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FBuildingDefinition>(TEXT("UBuildingDatabase::RebuildCache"),
		[this](const FName& RowName, const FBuildingDefinition& Row)
		{
			Cache.Add(RowName, Row);
		});
}

bool UBuildingDatabase::FindBuilding(FName BuildingID, FBuildingDefinition& OutDefinition) const
{
	if (const FBuildingDefinition* Found = Cache.Find(BuildingID))
	{
		OutDefinition = *Found;
		return true;
	}
	return false;
}
