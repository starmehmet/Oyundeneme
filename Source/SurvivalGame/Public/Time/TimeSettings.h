#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TimeSettings.generated.h"

/**
 * Sistem #1 — Zaman ayarları. Project Settings → Game → Survival Time altında görünür,
 * değerler Config/DefaultGame.ini'ye yazılır. Denge değerleri koda gömülmez (CLAUDE.md kuralı).
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Survival Time"))
class SURVIVALGAME_API UTimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTimeSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** 1 gerçek saniye = TimeScale oyun saniyesi. 10 → 1 gerçek dk = 10 oyun dk. */
	UPROPERTY(EditAnywhere, Config, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float TimeScale = 10.0f;

	/** Yeni oyunun başladığı saat (gün 0). */
	UPROPERTY(EditAnywhere, Config, Category = "Time", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartHour = 8;

	/** Gündoğumu dakikası. 360 = 06:00. */
	UPROPERTY(EditAnywhere, Config, Category = "Sun", meta = (ClampMin = "1", ClampMax = "1438"))
	int32 SunriseMinute = 360;

	/** Günbatımı dakikası. 1080 = 18:00. SunriseMinute'ten büyük olmalı. */
	UPROPERTY(EditAnywhere, Config, Category = "Sun", meta = (ClampMin = "2", ClampMax = "1439"))
	int32 SunsetMinute = 1080;

	/** Gündoğumu/batımı ışık geçiş penceresi (oyun dakikası). */
	UPROPERTY(EditAnywhere, Config, Category = "Sun", meta = (ClampMin = "0", ClampMax = "240"))
	float TransitionMinutes = 60.0f;

	/** Directional light gündüz şiddeti (lux). */
	UPROPERTY(EditAnywhere, Config, Category = "Sun", meta = (ClampMin = "0.0"))
	float DayIntensityLux = 8.0f;

	/** Directional light gece şiddeti (lux) — ay ışığı taklidi. */
	UPROPERTY(EditAnywhere, Config, Category = "Sun", meta = (ClampMin = "0.0"))
	float NightIntensityLux = 0.05f;
};
