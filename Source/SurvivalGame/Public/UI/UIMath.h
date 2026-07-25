#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #20 — UI: saf, durum tutmayan giris-yonlendirme mantigi. Yan etkisiz; birim testi
 * Private/Tests/UIMathTests.cpp.
 */
namespace SurvivalUI
{
	/**
	 * Ekran yigini (modal UScreenBase yigini) bosken oyun girisini yakalar (false); yigin
	 * bos DEGILSE UI girisini yakalar (true) — MIMARI'nin "giris yonlendirmesi (UI vs oyun)"
	 * maddesi. Negatif derinlik (cagiran hatasi) guvenlik icin "bos" sayilir.
	 */
	inline bool ShouldCaptureUIInput(int32 ScreenStackDepth)
	{
		return ScreenStackDepth > 0;
	}
}
