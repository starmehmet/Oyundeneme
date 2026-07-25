#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/**
 * Sistem #12 — Minimal, yeniden kullanılabilir can/hasar bileşeni. Bu projedeki İLK oyuncu
 * can sistemi (bkz. HealthMath.h yorumu / ADR) — Sistem #12'nin (Sıcaklık) hipotermi/sıcak
 * çarpması hasarı ihtiyacı için eklendi, ama gelecekteki sistemler (savaş, açlık, düşme
 * hasarı) de kullanabilir diye `APlayerCharacter`'a özel değil, herhangi bir aktöre eklenebilir
 * genel bir `UActorComponent` olarak tasarlandı.
 *
 * `TakeDamage`/`Heal`, `UInventoryComponent::AddItem` ile aynı sözleşmeyi takip eder:
 * GERÇEKTE uygulanan miktarı döner, çağıran taraf isterse bunu kullanır.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	/** Gerçekte uygulanan hasarı döner (0 olabilir — zaten ölüyse veya Amount<=0 ise). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float Amount);

	/** Gerçekte uygulanan iyileşmeyi döner. Ölü bir aktöre iyileşme uygulanmaz (basit karar). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	/** Sistem #17 — kayıttan yükleme için: `TakeDamage`/`Heal`'in GÖRECELİ sözleşmesinden
	 * BİLEREK farklı, MUTLAK bir değer atar (kayıtta hasar/iyileşme "olayı" yok, yalnızca
	 * son değer var). [0, MaxHealth] aralığında kelepçelenir, `OnHealthChanged` yayınlar
	 * (yüklemeden sonra HUD gibi dinleyiciler senkron kalsın diye) ama `OnDeath` YAYINLAMAZ
	 * (bir "yükleme" bir "ölüm olayı" değildir). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealthForLoad(float NewHealth);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;
};
