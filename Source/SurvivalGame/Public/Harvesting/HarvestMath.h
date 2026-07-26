#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #29 — saf, durum tutmayan yeniden-dogma/verim-araligi mantigi. Yan etkisiz;
 * birim testleri Private/Tests/HarvestMathTests.cpp. Gercek rastgele-secim (FMath::RandRange)
 * BILINCLI olarak burada DEGIL, cagiran tarafta (HarvestNode.cpp) kalir — WeatherSimulation'in
 * agirlikli-rastgele secim ayrimiyla ayni disiplin: RNG'nin kendisi test edilmez, yalnizca
 * destekleyici determinist formuller test edilir.
 */
namespace SurvivalHarvest
{
	/**
	 * Min<1 ise 1'e, Max<Min ise Min'e kelepcelenir (bozuk veri girisine karsi guvenli varsayilan).
	 * Inceleme bulgusu (minör): DT_HarvestNodes'ta YieldCountMin=0 gibi bozuk bir satir, editordeki
	 * ClampMin="1" ipucu atlanip (CSV/programatik import) girilirse, RequestedYield=0 uretip
	 * "0 < 0" karsilastirmasinin YANLIS olmasi yuzunden dugumun hicbir sey vermeden normal
	 * sekilde tukenmesine (RemainingHarvests dusmesine) yol aciyordu. Alt sinir >=1 garantisi
	 * bunu koklu sekilde kapatir.
	 */
	inline void NormalizeYieldRange(int32 Min, int32 Max, int32& OutMin, int32& OutMax)
	{
		OutMin = FMath::Max(1, Min);
		OutMax = FMath::Max(OutMin, Max);
	}

	/** Tukenmeden bu yana RespawnSeconds gecti mi? RespawnSeconds<=0 -> her zaman hazir (anlik yeniden-dogma). */
	inline bool IsRespawnReady(double DepletionGameTime, double CurrentGameTime, float RespawnSeconds)
	{
		if (RespawnSeconds <= 0.0f)
		{
			return true;
		}
		return (CurrentGameTime - DepletionGameTime) >= static_cast<double>(RespawnSeconds);
	}
}
