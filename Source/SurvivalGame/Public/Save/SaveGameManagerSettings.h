#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SaveGameManagerSettings.generated.h"

/**
 * Sistem #17 — Kaydet/Yükle ayarları. Project Settings → Game → Save Game Manager altında
 * görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Save Game Manager"))
class SURVIVALGAME_API USaveGameManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USaveGameManagerSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Otomatik kaydetme periyodu (sn). 0 veya altı = otomatik kaydetme KAPALI. */
	UPROPERTY(EditAnywhere, Config, Category = "Save", meta = (ClampMin = "0.0"))
	float AutosaveInterval = 300.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Save")
	FString AutosaveSlotName = TEXT("AutoSave");
};
