#pragma once

#include "CoreMinimal.h"
#include "SoundCategory.generated.h"

/**
 * Sistem #19 — Ses kategorisi (MIMARI.md ile birebir). Master, diger tum kategorilerin
 * ustune carpimsal olarak binen genel ana ses seviyesidir (bkz. AudioMath.h ComputeEffectiveVolume).
 */
UENUM(BlueprintType)
enum class ESoundCategory : uint8
{
	Master,
	Music,
	Ambient,
	SFX,
	UI,
	Voice
};
