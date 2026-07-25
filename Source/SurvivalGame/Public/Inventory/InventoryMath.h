#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #4 — Envanter: saf, durum tutmayan yığın/ağırlık mantığı.
 * Yan etkisiz; birim testleri Private/Tests/InventoryMathTests.cpp.
 */
namespace SurvivalInventory
{
	/** Bir slota ne kadarı sığar (taşan kısım hariç kabul edilen miktar). */
	inline int32 ComputeAcceptedIntoStack(int32 CurrentCount, int32 IncomingCount, int32 MaxStackSize)
	{
		const int32 Capacity = FMath::Max(0, MaxStackSize - CurrentCount);
		return FMath::Min(Capacity, FMath::Max(0, IncomingCount));
	}

	/** Sığmayıp taşan miktar (ComputeAcceptedIntoStack'in tamamlayıcısı). */
	inline int32 ComputeStackOverflow(int32 CurrentCount, int32 IncomingCount, int32 MaxStackSize)
	{
		return FMath::Max(0, IncomingCount) - ComputeAcceptedIntoStack(CurrentCount, IncomingCount, MaxStackSize);
	}

	/** Eklenecek ağırlık, taşıma limitini aşar mı? */
	inline bool WouldExceedWeightLimit(float CurrentWeight, float AddedWeight, float MaxWeight)
	{
		return (CurrentWeight + AddedWeight) > (MaxWeight + KINDA_SMALL_NUMBER);
	}

	/** Ağırlık limiti göz önüne alındığında en fazla kaç adet eklenebilir (0..IncomingCount). */
	inline int32 ComputeMaxAffordableCount(float CurrentWeight, float MaxWeight, float UnitWeight, int32 IncomingCount)
	{
		if (UnitWeight <= 0.0f)
		{
			return FMath::Max(0, IncomingCount);
		}
		const float RemainingCapacity = FMath::Max(0.0f, MaxWeight - CurrentWeight);
		// Epsilon BOLUNDUKTEN SONRA, adet uzayinda eklenir — bolmeden ONCE agirlik uzayina
		// eklenirse UnitWeight kucukken (<<1) epsilon adet-uzayinda buyuyup sigmayan
		// birimleri "sigar" gosterir (inceleme bulgusu: RemainingCapacity=0, UnitWeight=0.00002
		// icin yanlislikla 5 donuyordu; olmasi gereken 0).
		const int32 AffordableByWeight = FMath::FloorToInt(RemainingCapacity / UnitWeight + KINDA_SMALL_NUMBER);
		return FMath::Clamp(AffordableByWeight, 0, FMath::Max(0, IncomingCount));
	}
}
