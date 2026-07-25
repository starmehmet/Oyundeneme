#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TemperatureSimulationSettings.generated.h"

/**
 * Sistem #12 — Sıcaklık Simülasyonu ayarları. Project Settings → Game → Temperature Simulation
 * altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Temperature Simulation"))
class SURVIVALGAME_API UTemperatureSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTemperatureSimulationSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Gün/gece salınımının genliği (°C) — ortam sıcaklığına ±bu kadar eklenir. */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature", meta = (ClampMin = "0.0"))
	float DayNightAmplitude = 5.0f;

	/** Vücut sıcaklığının çevreye ne kadar hızlı sürükleneceği (bkz. ComputeBodyTemperatureDelta). */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature", meta = (ClampMin = "0.0"))
	float AdaptationRate = 0.05f;

	/**
	 * [ComfortMinTemp, ComfortMaxTemp] aralığındaki ortam sıcaklığı vücut sıcaklığını
	 * ETKİLEMEZ (giysi/metabolizma bu aralığı telafi eder varsayımı) — bkz.
	 * ComputeTargetBodyTemperature. İnceleme bulgusu: bu aralık olmadan DT_WeatherProfiles'daki
	 * TÜM sıradan hava profilleri (Clear dahil) 35°C altında olduğundan oyuncu her havada
	 * hipotermiden ölüyordu.
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature")
	float ComfortMinTemp = 10.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Temperature")
	float ComfortMaxTemp = 30.0f;

	/** Bu değerin altında hipotermi hasarı başlar (°C). */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature")
	float SafeMinBodyTemp = 35.0f;

	/** Bu değerin üstünde sıcak çarpması hasarı başlar (°C). */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature")
	float SafeMaxBodyTemp = 39.0f;

	/** Güvenli aralık dışında, her derece için saniyede uygulanan hasar. */
	UPROPERTY(EditAnywhere, Config, Category = "Temperature", meta = (ClampMin = "0.0"))
	float DamagePerSecondPerDegree = 2.0f;
};
