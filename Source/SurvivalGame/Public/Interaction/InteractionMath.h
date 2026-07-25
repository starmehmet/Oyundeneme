#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #3 — Etkileşim: saf, durum tutmayan karar mantığı.
 * Yan etkisiz; birim testleri Private/Tests/InteractionMathTests.cpp.
 */
namespace SurvivalInteraction
{
	/**
	 * Trace kısma (throttle): iz sürme yalnızca her TraceInterval karede bir yapılır
	 * (MIMARI.md Sistem #2 ölçek riski azaltımı — ekran gösterimi önbellekli sonucu kullanır).
	 * FrameCounter 0'dan artar; Interval <= 1 her kare demektir.
	 */
	inline bool ShouldTraceThisFrame(uint64 FrameCounter, int32 TraceInterval)
	{
		if (TraceInterval <= 1)
		{
			return true;
		}
		return (FrameCounter % static_cast<uint64>(TraceInterval)) == 0;
	}

	/**
	 * Prompt debounce: hızlı odak değişimlerinde UI titremesini önler (min 0.2 sn kuralı).
	 * Odak DEĞİŞMEDİYSE güncelleme gerekmez; değiştiyse son güncellemeden bu yana
	 * MinInterval geçmiş olmalı. İlk güncelleme (LastUpdateTime < 0) her zaman geçer.
	 */
	inline bool ShouldUpdatePrompt(double NowSeconds, double LastUpdateTimeSeconds, double MinIntervalSeconds, bool bFocusChanged)
	{
		if (!bFocusChanged)
		{
			return false;
		}
		if (LastUpdateTimeSeconds < 0.0)
		{
			return true;
		}
		return (NowSeconds - LastUpdateTimeSeconds) >= MinIntervalSeconds;
	}

	/**
	 * Hedef, etkileşim menzilinde mi? Menzil, BAKIŞ noktasından değil PAWN konumundan
	 * ölçülür — üçüncü şahısta kamera geride olduğundan kamera-mesafesi yanıltıcıdır.
	 */
	inline bool IsWithinRange(const FVector& PawnLocation, const FVector& TargetLocation, float MaxDistance)
	{
		return FVector::DistSquared(PawnLocation, TargetLocation) <= FMath::Square(MaxDistance);
	}
}
