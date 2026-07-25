#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WeatherSimulationSettings.generated.h"

/**
 * Sistem #11 — Hava Durumu Simülasyonu ayarları. Project Settings → Game → Weather Simulation
 * altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Weather Simulation"))
class SURVIVALGAME_API UWeatherSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UWeatherSimulationSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Yeni bir hava durumu "atılıp atılmayacağının" değerlendirildiği periyot (sn). DoD: 30sn. */
	UPROPERTY(EditAnywhere, Config, Category = "Weather", meta = (ClampMin = "1.0"))
	float EvaluationInterval = 30.0f;

	/** Mevcut ve hedef hava durumu arasındaki geçişin (interpolasyonun) süresi (sn). */
	UPROPERTY(EditAnywhere, Config, Category = "Weather", meta = (ClampMin = "0.0"))
	float TransitionDuration = 10.0f;

	/** Değerlendirmede MEVCUT hava durumunun ağırlığı bu katsayıyla çarpılır — hava durumunun "atalet"i. */
	UPROPERTY(EditAnywhere, Config, Category = "Weather", meta = (ClampMin = "1.0"))
	float PersistenceMultiplier = 3.0f;
};
