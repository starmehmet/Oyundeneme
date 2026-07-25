#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #2 — Oyuncu Kontrolü: saf, durum tutmayan kamera matematiği.
 * Yan etkisiz; birim testleri Private/Tests/CameraMathTests.cpp.
 */
namespace SurvivalCamera
{
	/**
	 * Fare tekerleği zoom adımını uygular ve [MinArmLength, MaxArmLength] aralığına kelepçeler.
	 * WheelDelta pozitif = yakınlaş (kol kısalır), negatif = uzaklaş (kol uzar).
	 */
	inline float ApplyZoomStep(float CurrentArmLength, float WheelDelta, float StepSize, float MinArmLength, float MaxArmLength)
	{
		const float NewLength = CurrentArmLength - (WheelDelta * StepSize);
		return FMath::Clamp(NewLength, MinArmLength, MaxArmLength);
	}
}
