#pragma once

#include "CoreMinimal.h"
#include "ScarcityReason.generated.h"

/** Sistem #10 — Kaynak Simülasyonu'nun "kıtlık alarmı"nın tetiklenme nedeni. */
UENUM(BlueprintType)
enum class EScarcityReason : uint8
{
	None,
	EnergyBrownout,
	FuelCritical,
	Overheating
};
