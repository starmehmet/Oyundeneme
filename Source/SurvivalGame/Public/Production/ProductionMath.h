#pragma once

#include "CoreMinimal.h"
#include "Production/ProductionState.h"

/**
 * Sistem #9 — Üretim Makineleri: saf, durum tutmayan ilerleme/bloke-durumu/kare-bölümleme
 * mantığı. Yan etkisiz; birim testleri Private/Tests/ProductionMathTests.cpp.
 */
namespace SurvivalProduction
{
	/** İlerleme oranı [0,1]. ProductionTime<=0 anlık üretim demektir — her zaman tamamlanmış sayılır. */
	inline float ComputeProductionProgress(float ElapsedTime, float ProductionTime)
	{
		if (ProductionTime <= 0.0f)
		{
			return 1.0f;
		}
		return FMath::Clamp(ElapsedTime / ProductionTime, 0.0f, 1.0f);
	}

	inline bool IsProductionComplete(float ElapsedTime, float ProductionTime)
	{
		return ElapsedTime >= ProductionTime;
	}

	/** Bu DeltaTime'da tüketilen enerji miktarı. Negatif hız/süre 0'a kelepçelenir. */
	inline float ComputeEnergyConsumed(float EnergyPerSecond, float DeltaTime)
	{
		return FMath::Max(0.0f, EnergyPerSecond) * FMath::Max(0.0f, DeltaTime);
	}

	/**
	 * Girdi/çıktı/yakıt durumuna göre makinenin ANLIK durumunu belirler. Öncelik sırası
	 * (yakıt > girdi > çıktı) BİLİNÇLİ bir seçim — yakıt en temel engelleyicidir: girdi ve
	 * çıktı için yer olsa bile enerji yoksa hiçbir şey çalışmaz.
	 */
	inline EProductionState DetermineBlockedState(bool bHasInputs, bool bHasOutputRoom, bool bHasFuel)
	{
		if (!bHasFuel)
		{
			return EProductionState::Blocked_NoFuel;
		}
		if (!bHasInputs)
		{
			return EProductionState::Blocked_NoInput;
		}
		if (!bHasOutputRoom)
		{
			return EProductionState::Blocked_NoOutput;
		}
		return EProductionState::Running;
	}

	/**
	 * "Frame bölümleme": TotalCount öğeyi FramesPerCycle kare içinde en az bir kez güncellemek
	 * için, bu karede kaç tanesinin işleneceği (CLAUDE.md örneği: 500 makine / 60 frame).
	 * Her zaman en az 1 döner (TotalCount>0 iken) — küçük listelerde bile ilerleme durmaz.
	 */
	inline int32 ComputeBatchSize(int32 TotalCount, int32 FramesPerCycle)
	{
		if (TotalCount <= 0)
		{
			return 0;
		}
		return FMath::Max(1, FMath::CeilToInt(static_cast<float>(TotalCount) / FMath::Max(1, FramesPerCycle)));
	}
}
