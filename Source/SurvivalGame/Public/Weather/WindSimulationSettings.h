#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WindSimulationSettings.generated.h"

/**
 * Sistem #13 — Rüzgar Simülasyonu ayarları. Project Settings → Game → Wind Simulation altında
 * görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Wind Simulation"))
class SURVIVALGAME_API UWindSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UWindSimulationSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Yerel hamle (gust) hücrelerinin boyutu (UU) — bundan daha yakın iki konum aynı gust
	 * fazını paylaşır. Construction'ın 400 UU ızgarasından bilerek daha büyük (gust dokusu
	 * bina-ölçeğinde değil, bölge-ölçeğinde değişir). */
	UPROPERTY(EditAnywhere, Config, Category = "Wind", meta = (ClampMin = "1.0"))
	float GustCellSize = 2000.0f;

	/** Küresel rüzgar hızına eklenip çıkarılabilecek maksimum yerel hamle (±). */
	UPROPERTY(EditAnywhere, Config, Category = "Wind", meta = (ClampMin = "0.0"))
	float GustAmplitude = 5.0f;

	/** Gust salınımının açısal hızı (rad/sn) — büyüdükçe hamle daha hızlı dalgalanır. */
	UPROPERTY(EditAnywhere, Config, Category = "Wind", meta = (ClampMin = "0.0"))
	float GustFrequency = 0.5f;

	/** `GetWindLoadOnStructure` için varsayılan sürükleme katsayısı (tipik bir bina/yüzey). */
	UPROPERTY(EditAnywhere, Config, Category = "Wind", meta = (ClampMin = "0.0"))
	float DefaultDragCoefficient = 1.2f;
};
