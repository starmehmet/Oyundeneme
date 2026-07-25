#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #10 — Kaynak Simülasyonu: saf, durum tutmayan enerji-frekansı/termal/yakıt-kıtlığı
 * mantığı. Yan etkisiz; birim testleri Private/Tests/ResourceSimulationMathTests.cpp.
 */
namespace SurvivalResource
{
	/**
	 * Üretim/tüketim oranından şebeke frekansını türetir. Tüketim yoksa (şebekede yük yok)
	 * nominal 50Hz döner. Basitleştirilmiş model: frekans ASLA 50'yi aşmaz (fazla üretim
	 * "güvenli/istikrarlı" sayılır, gerçek şebekelerdeki aşırı-frekans dengesizliği
	 * modellenmez — MIMARI.md'nin "50Hz = stabil" ifadesiyle tutarlı, bkz. ADR).
	 */
	inline float ComputeFrequency(float TotalProduction, float TotalConsumption)
	{
		if (TotalConsumption <= 0.0f)
		{
			return 50.0f;
		}
		const float Ratio = FMath::Max(0.0f, TotalProduction) / TotalConsumption;
		return FMath::Clamp(50.0f * FMath::Min(1.0f, Ratio), 0.0f, 50.0f);
	}

	/** MIMARI.md: 49.5–50.5 Hz istikrarlı sayılır. */
	inline bool IsFrequencyStable(float Frequency)
	{
		return Frequency >= 49.5f && Frequency <= 50.5f;
	}

	/** MIMARI.md: <48Hz brownout (kıtlık alarmı eşiği — istikrarsız-ama-henüz-brownout-değil bir ara bölge de var). */
	inline bool IsBrownout(float Frequency)
	{
		return Frequency < 48.0f;
	}

	/**
	 * Bu DeltaTime'da sıcaklığa eklenecek/çıkarılacak miktar. Isı üretimi dağıtımı aşarsa
	 * pozitif (ısınma), aksi halde negatif (soğuma) döner — çağıran taraf bunu birikimli
	 * `CurrentTemperature`'a EKLEMELİDİR (bkz. FThermalBudget yorumu — termal atalet modeli).
	 */
	inline float ComputeTemperatureDelta(float HeatProduction, float HeatDissipation, float DeltaTime)
	{
		return (HeatProduction - HeatDissipation) * FMath::Max(0.0f, DeltaTime);
	}

	inline bool IsOverheating(float CurrentTemperature, float MaxSafeTemperature)
	{
		return CurrentTemperature > MaxSafeTemperature;
	}

	/** MaxAmount<=0 (tanımsız/boş rezerv) her zaman false döner — "kıtlık" içi boş bir kavramı olmayan bir rezerv için anlamsız. */
	inline bool IsFuelCritical(float CurrentAmount, float MaxAmount, float CriticalFraction)
	{
		if (MaxAmount <= 0.0f)
		{
			return false;
		}
		return (CurrentAmount / MaxAmount) < CriticalFraction;
	}
}
