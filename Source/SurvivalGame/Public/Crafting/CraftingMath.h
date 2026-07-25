#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #6 — Zanaat: saf, durum tutmayan ilerleme/kuyruk mantığı.
 * Yan etkisiz; birim testleri Private/Tests/CraftingMathTests.cpp.
 */
namespace SurvivalCrafting
{
	/** İlerleme oranı [0,1]. CraftingTime<=0 anlık tarif demektir — her zaman tamamlanmış sayılır. */
	inline float ComputeCraftProgress(float ElapsedTime, float CraftingTime)
	{
		if (CraftingTime <= 0.0f)
		{
			return 1.0f;
		}
		return FMath::Clamp(ElapsedTime / CraftingTime, 0.0f, 1.0f);
	}

	inline bool IsCraftComplete(float ElapsedTime, float CraftingTime)
	{
		return ElapsedTime >= CraftingTime;
	}

	/** Kuyruğa yeni iş eklenebilir mi (kapasite doldu mu)? */
	inline bool CanEnqueue(int32 CurrentQueueSize, int32 MaxQueueSize)
	{
		return CurrentQueueSize < MaxQueueSize;
	}
}
