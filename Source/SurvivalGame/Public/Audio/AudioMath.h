#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #19 — Ses: saf, durum tutmayan ses-karisimi/budama mantigi. Yan etkisiz; birim
 * testleri Private/Tests/AudioMathTests.cpp.
 */
namespace SurvivalAudio
{
	/**
	 * Nihai efektif ses seviyesi = Taban * Kategori * Master (uc katman carpimsal karisim —
	 * MIMARI'nin "kategori basina ses ayari" istegi). Her girdi [0,1] araligina once
	 * kelepcelenir (negatif/asiri buyuk deger UI'dan sizarsa bile guvenli), sonuc da [0,1]'e
	 * kelepcelenir.
	 */
	inline float ComputeEffectiveVolume(float BaseVolume, float CategoryVolume, float MasterVolume)
	{
		const float ClampedBase = FMath::Clamp(BaseVolume, 0.0f, 1.0f);
		const float ClampedCategory = FMath::Clamp(CategoryVolume, 0.0f, 1.0f);
		const float ClampedMaster = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
		return FMath::Clamp(ClampedBase * ClampedCategory * ClampedMaster, 0.0f, 1.0f);
	}

	/**
	 * Uzak 3D sesleri budamak icin: dinleyiciden MaxAudibleDistance'tan daha uzaktaki sesler
	 * calinmaz (MIMARI "ses budamasi"). MaxAudibleDistance<=0 SINIRSIZ menzil demektir (asla
	 * budanmaz) — 2D/UI sesleri zaten cagiran tarafta bIs3D ile bu kontrolun disinda tutulur,
	 * bu fonksiyon yalnizca konumlandirilmis (3D) sesler icin anlamlidir.
	 */
	inline bool ShouldCullSound(float DistanceToListener, float MaxAudibleDistance)
	{
		if (MaxAudibleDistance <= 0.0f)
		{
			return false;
		}
		return DistanceToListener > MaxAudibleDistance;
	}
}
