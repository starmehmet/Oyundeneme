#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ResourceSimulationSettings.generated.h"

/**
 * Sistem #10 — Kaynak Simülasyonu ayarları. Project Settings → Game → Resource Simulation
 * altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Resource Simulation"))
class SURVIVALGAME_API UResourceSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UResourceSimulationSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Kayıtlı ısı üreticisi olmasa bile şebekenin sahip olduğu sabit soğutma kapasitesi. */
	UPROPERTY(EditAnywhere, Config, Category = "Thermal", meta = (ClampMin = "0.0"))
	float BaselineHeatDissipation = 10.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Thermal", meta = (ClampMin = "0.0"))
	float MaxSafeTemperature = 100.0f;

	/** Bir yakıt rezervi bu oranın ALTINA düşünce "kritik" sayılır (0.1 = %10). */
	UPROPERTY(EditAnywhere, Config, Category = "Fuel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FuelCriticalFraction = 0.1f;
};
