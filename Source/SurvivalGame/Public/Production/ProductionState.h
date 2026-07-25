#pragma once

#include "CoreMinimal.h"
#include "ProductionState.generated.h"

/** Sistem #9 — Bir üretim makinesinin şu anki durumu. */
UENUM(BlueprintType)
enum class EProductionState : uint8
{
	Idle,
	Running,
	Blocked_NoInput,
	Blocked_NoOutput,
	Blocked_NoFuel
};
