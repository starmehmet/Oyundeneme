#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AudioManagerSettings.generated.h"

/**
 * Sistem #19 — Ses Yoneticisi ayarlari. Project Settings -> Game -> Audio Manager altinda
 * gorunur. Kategori varsayilanlari TEK TEK float alan olarak tutulur (UDeveloperSettings
 * icinde TMap<Enum,float> yerine) — Project Settings editor UI'da her kategori kendi
 * satirinda acikca gorunur ve WindSimulationSettings/TemperatureSimulationSettings ile ayni
 * "duz alanlar" bicemini korur.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Audio Manager"))
class SURVIVALGAME_API UAudioManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAudioManagerSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** 3D sesler icin dinleyiciden maksimum duyulabilir mesafe (UU). <=0 sinirsiz demektir. */
	UPROPERTY(EditAnywhere, Config, Category = "Audio", meta = (ClampMin = "0.0"))
	float MaxAudibleDistance = 6000.0f;

	/** Muzik gecisleri (crossfade) icin varsayilan sure (sn). */
	UPROPERTY(EditAnywhere, Config, Category = "Audio", meta = (ClampMin = "0.0"))
	float DefaultMusicCrossfadeDuration = 2.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultMasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultMusicVolume = 0.7f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultAmbientVolume = 0.8f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultSFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultUIVolume = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Audio|Varsayilan Seviyeler", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultVoiceVolume = 1.0f;
};
