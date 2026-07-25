#include "Weather/WeatherProfileDatabase.h"
#include "Weather/WeatherProfileDatabaseSettings.h"
#include "SurvivalGame.h"
#include "Engine/DataTable.h"

void UWeatherProfileDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UWeatherProfileDatabaseSettings* Settings = GetDefault<UWeatherProfileDatabaseSettings>();
	LoadedTable = Settings->WeatherProfileTable.LoadSynchronous();

	if (!LoadedTable)
	{
		UE_LOG(LogSurvivalWeather, Warning,
			TEXT("WeatherProfileDatabase: DT_WeatherProfiles atanmamis (Project Settings > Game > Weather Profile Database) — veritabani bos."));
		return;
	}

	RebuildCache();
	UE_LOG(LogSurvivalWeather, Log, TEXT("WeatherProfileDatabase hazir: %d profil yuklendi"), Cache.Num());
}

void UWeatherProfileDatabase::RebuildCache()
{
	Cache.Reset();
	if (!LoadedTable)
	{
		return;
	}

	LoadedTable->ForeachRow<FWeatherProfile>(TEXT("UWeatherProfileDatabase::RebuildCache"),
		[this](const FName& RowName, const FWeatherProfile& Row)
		{
			Cache.Add(Row.Condition, Row);
		});
}

bool UWeatherProfileDatabase::FindProfile(EWeatherCondition Condition, FWeatherProfile& OutProfile) const
{
	if (const FWeatherProfile* Found = Cache.Find(Condition))
	{
		OutProfile = *Found;
		return true;
	}
	return false;
}

TArray<EWeatherCondition> UWeatherProfileDatabase::GetAllConditions() const
{
	TArray<EWeatherCondition> Result;
	Cache.GetKeys(Result);
	return Result;
}
