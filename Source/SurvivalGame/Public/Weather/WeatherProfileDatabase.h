#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Weather/WeatherProfile.h"
#include "WeatherProfileDatabase.generated.h"

class UDataTable;

/**
 * Sistem #11 — Hava durumu profillerinin tek sahibi. DT_WeatherProfiles'ı (üzerinden
 * UWeatherProfileDatabaseSettings) yükler, `EWeatherCondition`→`FWeatherProfile` önbelleği
 * kurar (satır ADINA değil `FWeatherProfile::Condition` alanına göre — bkz. WeatherProfile.h
 * yorumu). Önbellek DataTable satırlarının KOPYASINI tutar (ham işaretçi değil) — UItemDatabase
 * ile aynı desen.
 */
UCLASS()
class SURVIVALGAME_API UWeatherProfileDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	bool FindProfile(EWeatherCondition Condition, FWeatherProfile& OutProfile) const;

	UFUNCTION(BlueprintPure, Category = "Weather")
	TArray<EWeatherCondition> GetAllConditions() const;

	UFUNCTION(BlueprintPure, Category = "Weather")
	int32 GetProfileCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<EWeatherCondition, FWeatherProfile> Cache;
};
