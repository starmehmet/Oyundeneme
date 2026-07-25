#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCState.h"

/**
 * Sistem #15 — NPC Yapay Zeka: saf, durum tutmayan durum-belirleme, yorgunluk/moral ve
 * konum/uyku-saati kontrol matematiği. Yan etkisiz; birim testleri
 * Private/Tests/NPCMathTests.cpp.
 */
namespace SurvivalNPC
{
	/**
	 * Bu tick'te NPC'nin olması gereken durumu — SABİT ÖNCELİK SIRASI (üstteki alttakini
	 * ezer): Hurt > Sleeping > Eating > Working/Walking (göreve göre) > Idle. `ProductionMath::
	 * DetermineBlockedState`'teki (Sistem #9) aynı "küçük, saf, öncelikli karar fonksiyonu"
	 * deseni.
	 */
	inline ENPCState DetermineNPCState(bool bIsHurt, bool bWantsToSleep, bool bIsEating, bool bHasTask, bool bIsAtTaskLocation)
	{
		if (bIsHurt)
		{
			return ENPCState::Hurt;
		}
		if (bWantsToSleep)
		{
			return ENPCState::Sleeping;
		}
		if (bIsEating)
		{
			return ENPCState::Eating;
		}
		if (bHasTask)
		{
			return bIsAtTaskLocation ? ENPCState::Working : ENPCState::Walking;
		}
		return ENPCState::Idle;
	}

	/** Çalışırken yorgunluk birikir, uyurken azalır; diğer durumlarda (Idle/Walking/Hurt)
	 * nötrdür (basitleştirme — bkz. ADR). Çağıran taraf sonucu Fatigue'a EKLEYİP
	 * [0, MaxFatigue] aralığında kelepçelemelidir. */
	inline float ComputeFatigueDelta(bool bIsWorking, bool bIsSleeping, float WorkFatigueRate, float SleepRecoveryRate, float DeltaTime)
	{
		const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);
		if (bIsSleeping)
		{
			return -FMath::Max(0.0f, SleepRecoveryRate) * SafeDeltaTime;
		}
		if (bIsWorking)
		{
			return FMath::Max(0.0f, WorkFatigueRate) * SafeDeltaTime;
		}
		return 0.0f;
	}

	/** Yorgunluk oranı bir eşiği aşarsa (aşırı çalışma) moral düşer, aksi halde toparlanır.
	 * Çağıran taraf sonucu Morale'e EKLEYİP [0,1] aralığında kelepçelemelidir. */
	inline float ComputeMoraleDelta(float FatigueRatio01, float MoraleRecoveryRate, float MoraleDecayRate, float OverworkFatigueRatio, float DeltaTime)
	{
		const float SafeDeltaTime = FMath::Max(0.0f, DeltaTime);
		const bool bOverworked = FatigueRatio01 > OverworkFatigueRatio;
		return (bOverworked ? -FMath::Max(0.0f, MoraleDecayRate) : FMath::Max(0.0f, MoraleRecoveryRate)) * SafeDeltaTime;
	}

	/** Hedefe kabul yarıçapı içinde mi (görev konumuna "vardı" sayılır). */
	inline bool IsAtLocation(const FVector& Current, const FVector& Target, float AcceptanceRadius)
	{
		return FVector::DistSquared(Current, Target) <= FMath::Square(FMath::Max(0.0f, AcceptanceRadius));
	}

	/** Can, maksimumun bu oranının altında/eşitse yaralı sayılır. */
	inline bool IsHurt(float Health, float MaxHealth, float HurtThresholdFraction)
	{
		return Health <= MaxHealth * FMath::Clamp(HurtThresholdFraction, 0.0f, 1.0f);
	}

	/**
	 * Günün bu saatinde uyku zamanı mı — gece-yarısı SARMALINI doğru ele alır (ör.
	 * SleepStartHour=22, SleepEndHour=6: 22:00-23:59 VE 00:00-05:59 uyku zamanı).
	 */
	inline bool WantsToSleep(int32 HourOfDay, int32 SleepStartHour, int32 SleepEndHour)
	{
		if (SleepStartHour <= SleepEndHour)
		{
			return HourOfDay >= SleepStartHour && HourOfDay < SleepEndHour;
		}
		return HourOfDay >= SleepStartHour || HourOfDay < SleepEndHour;
	}
}
