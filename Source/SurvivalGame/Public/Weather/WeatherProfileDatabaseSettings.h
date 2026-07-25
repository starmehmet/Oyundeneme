#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "WeatherProfileDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #11 — Hava Durumu Profilleri Veritabanı ayarları (UItemDatabaseSettings ile aynı desen).
 * Project Settings → Game → Weather Profile Database altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Weather Profile Database"))
class SURVIVALGAME_API UWeatherProfileDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UWeatherProfileDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_WeatherProfiles — satır yapısı FWeatherProfile olmalı, 9 hava durumunun HEPSİ için birer satır önerilir. */
	UPROPERTY(EditAnywhere, Config, Category = "Weather")
	TSoftObjectPtr<UDataTable> WeatherProfileTable;
};
