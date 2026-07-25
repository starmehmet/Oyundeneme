#pragma once

#include "CoreMinimal.h"
#include "NPCState.generated.h"

/** Sistem #15 — MIMARI.md'nin `ENPCState`'i birebir. `Eating` şu an hiçbir zaman
 * tetiklenmez (açlık/yemek sistemi 28 sistemde YOK) — enum değeri MIMARI ile bire bir
 * kalsın diye duruyor, gerçek bir açlık sistemi eklenince kullanılabilir (bkz. ADR). */
UENUM(BlueprintType)
enum class ENPCState : uint8
{
	Idle,
	Walking,
	Working,
	Sleeping,
	Hurt,
	Eating
};
