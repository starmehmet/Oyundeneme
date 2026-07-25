#pragma once

#include "CoreMinimal.h"
#include "WeatherCondition.generated.h"

/** Sistem #11 — MIMARI.md ile birebir: 9 hava durumu. */
UENUM(BlueprintType)
enum class EWeatherCondition : uint8
{
	Clear,
	PartlyCloudy,
	Overcast,
	Foggy,
	Rainy,
	Stormy,
	Snowing,
	Blizzard,
	Heatwave
};
