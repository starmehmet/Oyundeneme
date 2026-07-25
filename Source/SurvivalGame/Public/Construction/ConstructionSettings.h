#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ConstructionSettings.generated.h"

/**
 * Sistem #7 — İnşaat ızgarasının fiziksel parametresi. Bina VERİTABANI'ndan (DT_Buildings,
 * bkz. UBuildingDatabaseSettings) BİLEREK AYRI — GridSize dünyanın bir özelliği, bina
 * kataloğu değil; biri değişince diğerine dokunulmaz.
 * Project Settings → Game → Construction altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Construction"))
class SURVIVALGAME_API UConstructionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UConstructionSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Izgara hucre boyutu (Unreal Unit). DoD: 400 UU. */
	UPROPERTY(EditAnywhere, Config, Category = "Grid", meta = (ClampMin = "1.0"))
	float GridSize = 400.0f;
};
