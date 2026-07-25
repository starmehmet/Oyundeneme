#include "Player/HealthComponent.h"
#include "Player/HealthMath.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

bool UHealthComponent::IsDead() const
{
	return SurvivalHealth::IsDead(CurrentHealth);
}

void UHealthComponent::SetCurrentHealthForLoad(float NewHealth)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHealthComponent::TakeDamage(float Amount)
{
	if (IsDead())
	{
		return 0.0f;
	}

	const float Applied = SurvivalHealth::ComputeAppliedDamage(CurrentHealth, Amount);
	if (Applied <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth -= Applied;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	if (IsDead())
	{
		OnDeath.Broadcast();
	}
	return Applied;
}

float UHealthComponent::Heal(float Amount)
{
	if (IsDead())
	{
		return 0.0f; // olu bir aktore iyilesme uygulanmaz (basit karar)
	}

	const float Applied = SurvivalHealth::ComputeAppliedHeal(CurrentHealth, MaxHealth, Amount);
	if (Applied <= 0.0f)
	{
		return 0.0f;
	}

	CurrentHealth += Applied;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	return Applied;
}
