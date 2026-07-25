#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SnowSimulationSettings.generated.h"

/**
 * Sistem #14 — Kar Birikimi Simülasyonu ayarları. Project Settings → Game → Snow Accumulation
 * altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Snow Accumulation"))
class SURVIVALGAME_API USnowSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USnowSimulationSettings()
	{
		CategoryName = TEXT("Game");
	}

	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float AccumulationCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float MeltCoefficient = 0.1f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow")
	float FreezingTemperature = 0.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float MaxSnowDepth = 100.0f;

	/** Bu derinlikte (ve ötesinde) hareket cezası tabana ulaşır. */
	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float MaxMovementPenaltyDepth = 50.0f;

	/** Tavan derinlikte hız çarpanı (1.0=ceza yok, 0.0=tamamen durur). */
	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSpeedMultiplier = 0.4f;

	/** Bu derinliğin ÜSTÜNDE inşaat engellenir. */
	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float MaxConstructionDepth = 30.0f;

	/** Çığ riski değerlendirmesi arası süre (sn) — her Tick'te DEĞİL, yer-normali line-trace'i
	 * ucuz olmadığı için (bkz. `UWeatherSimulation::EvaluationInterval` ile aynı periyodik
	 * desen). */
	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "1.0"))
	float AvalancheCheckInterval = 5.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow", meta = (ClampMin = "0.0"))
	float AvalancheMinDepth = 40.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow")
	float AvalancheMinSlopeDegrees = 30.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Snow")
	float AvalancheMaxSlopeDegrees = 55.0f;
};
