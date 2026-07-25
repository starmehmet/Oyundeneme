#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #14 — Kar Birikimi: saf, durum tutmayan birikim/erime, hareket cezası, inşaat-engelleme
 * ve çığ-riski matematiği. Yan etkisiz; birim testleri Private/Tests/SnowSimulationMathTests.cpp.
 */
namespace SurvivalSnow
{
	/** Hava durumu gerçekten kar yağışlıysa (Snowing/Blizzard) yağışa orantılı birikim hızı;
	 * değilse 0. */
	inline float ComputeSnowAccumulationRate(bool bIsSnowyCondition, float Precipitation, float AccumulationCoefficient)
	{
		if (!bIsSnowyCondition)
		{
			return 0.0f;
		}
		return FMath::Max(0.0f, Precipitation) * FMath::Max(0.0f, AccumulationCoefficient);
	}

	/** Donma noktasının ÜSTÜNDEKİ her derece için erime hızı; donma noktasında/altında 0. */
	inline float ComputeSnowMeltRate(float AmbientTemperature, float FreezingTemperature, float MeltCoefficient)
	{
		const float DegreesAboveFreezing = FMath::Max(0.0f, AmbientTemperature - FreezingTemperature);
		return DegreesAboveFreezing * FMath::Max(0.0f, MeltCoefficient);
	}

	/** Net derinlik değişimi (birikim - erime) * DeltaTime — çağıran taraf sonucu mevcut
	 * derinliğe EKLEYİP [0, MaxDepth] aralığında kelepçelemelidir. */
	inline float ComputeSnowDepthDelta(float AccumulationRate, float MeltRate, float DeltaTime)
	{
		return (AccumulationRate - MeltRate) * FMath::Max(0.0f, DeltaTime);
	}

	/** [0,1] hız çarpanı: derinlik arttıkça 1.0'dan MinSpeedMultiplier'a doğru DOĞRUSAL azalır,
	 * MaxPenaltyDepth'te (ve ötesinde) tabana ulaşır. */
	inline float ComputeMovementSpeedMultiplier(float SnowDepth, float MaxPenaltyDepth, float MinSpeedMultiplier)
	{
		const float SafeMaxDepth = FMath::Max(KINDA_SMALL_NUMBER, MaxPenaltyDepth);
		const float Ratio = FMath::Clamp(SnowDepth / SafeMaxDepth, 0.0f, 1.0f);
		return FMath::Lerp(1.0f, FMath::Clamp(MinSpeedMultiplier, 0.0f, 1.0f), Ratio);
	}

	/** Kar derinliği eşiği aşıldıysa inşaat engellenir. */
	inline bool IsConstructionBlocked(float SnowDepth, float MaxConstructionDepth)
	{
		return SnowDepth > MaxConstructionDepth;
	}

	/** Bir yer normalinden dikeyle yaptığı açı (derece) — 0=düz zemin, 90=dikey duvar. */
	inline float ComputeSlopeAngleDegrees(const FVector& GroundNormal)
	{
		const FVector Normal = GroundNormal.GetSafeNormal();
		if (Normal.IsNearlyZero())
		{
			return 0.0f;
		}
		const float CosAngle = FVector::DotProduct(Normal, FVector::UpVector);
		return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.0f, 1.0f)));
	}

	/**
	 * Çığ riski: kar derinliği eşiği aşmalı VE eğim [MinRiskSlopeDegrees, MaxRiskSlopeDegrees]
	 * aralığında olmalı — gerçek çığlar ORTA-DİK yamaçlarda (~tipik 30-45°) oluşur; çok düz
	 * yamaçta kar kaymaz, çok dik yamaçta zaten büyük birikim tutunamaz (sürekli küçük küçük
	 * dökülür) — bkz. ADR.
	 */
	inline bool IsAvalancheRisk(float SnowDepth, float SlopeAngleDegrees, float MinRiskDepth, float MinRiskSlopeDegrees, float MaxRiskSlopeDegrees)
	{
		return SnowDepth >= MinRiskDepth
			&& SlopeAngleDegrees >= MinRiskSlopeDegrees
			&& SlopeAngleDegrees <= MaxRiskSlopeDegrees;
	}
}
