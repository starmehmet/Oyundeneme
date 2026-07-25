#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #12 — Sağlık: saf, durum tutmayan hasar/iyileşme kelepçeleme mantığı. Yan etkisiz;
 * birim testleri Private/Tests/HealthMathTests.cpp.
 *
 * Bu proje için İLK oyuncu can/hasar mekanizması — MIMARI.md'de oyuncu için tanımlı bir
 * Health sistemi YOK (yalnızca NPC'lerin, Sistem #15, kendi Health alanı var); Sistem #12'nin
 * DoD'si ("oyuncu hipotermi/sıcak çarpması hasarı") gerçek bir hasar mekanizması gerektirdiği
 * için burada, operatör onayıyla, minimal ve yeniden-kullanılabilir olarak eklendi (bkz. ADR).
 */
namespace SurvivalHealth
{
	/** Gerçekte uygulanabilecek hasar miktarı — CurrentHealth'i asla negatife düşürmez. */
	inline float ComputeAppliedDamage(float CurrentHealth, float Amount)
	{
		return FMath::Clamp(Amount, 0.0f, FMath::Max(0.0f, CurrentHealth));
	}

	/** Gerçekte uygulanabilecek iyileşme miktarı — CurrentHealth'i asla MaxHealth'i aşacak şekilde artırmaz. */
	inline float ComputeAppliedHeal(float CurrentHealth, float MaxHealth, float Amount)
	{
		return FMath::Clamp(Amount, 0.0f, FMath::Max(0.0f, MaxHealth - CurrentHealth));
	}

	inline bool IsDead(float CurrentHealth)
	{
		return CurrentHealth <= 0.0f;
	}
}
