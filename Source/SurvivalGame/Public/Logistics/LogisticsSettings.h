#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LogisticsSettings.generated.h"

/**
 * Sistem #8 — Lojistik ağı ayarları. Project Settings → Game → Logistics altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Logistics"))
class SURVIVALGAME_API ULogisticsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULogisticsSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Konveyör/drone içeriği gelene kadar TÜM taşımalar bu sabit hızı kullanır (UU/sn). */
	UPROPERTY(EditAnywhere, Config, Category = "Transport", meta = (ClampMin = "1.0"))
	float TransportSpeed = 500.0f;
};
