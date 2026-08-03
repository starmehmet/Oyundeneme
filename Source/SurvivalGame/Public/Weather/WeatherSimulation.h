#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Weather/WeatherTypes.h"
#include "WeatherSimulation.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherStateChanged, FWeatherState, NewState);

/**
 * Sistem #11 — Hava durumunun tek sahibi. MIMARI.md'nin `AWeatherManager`(`AGameStateBase`)
 * + `UWeatherSimulation`(`UGameInstanceSubsystem`) ikilisi YERİNE TEK bir sınıf — Sistem
 * #7/#9/#10'da zaten üç kez uygulanan "Singleton = Subsystem, elle yönetilen Actor/GameState
 * manager yazma" kararının doğal devamı (bkz. ADR).
 *
 * `UTimeKeeper`/`ULogisticsNetwork`/`UResourceSimulation` ile aynı `FTickableGameObject`
 * deseni. `OnWeatherStateChanged`, kıtlık alarmının AKSİNE (edge-triggered) HER Tick'te
 * yayınlanır — hava değerleri (sıcaklık, görüş mesafesi) sürekli-anlamlı bilgidir, bir
 * "durum DEĞİŞİKLİĞİ" olayı değil (tıpkı `UCraftingComponent::OnCraftingProgress`'in her
 * tick yayınlanması gibi — bkz. ADR).
 *
 * DoD'nin "30 sn'de bir değerlendirme" maddesi: her `EvaluationInterval` saniyede bir
 * `RollNextWeather` ağırlıklı-rastgele bir sonraki durumu seçer (mevcut durum
 * `PersistenceMultiplier` ile "atalet" kazanır); seçilen durum mevcuttan FARKLIYSA yeni bir
 * geçiş (`TransitionDuration` boyunca interpolasyon) başlar.
 */
UCLASS()
class SURVIVALGAME_API UWeatherSimulation : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UWeatherSimulation, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintPure, Category = "Weather")
	const FWeatherState& GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetTransitionProgress() const { return TransitionProgress; }

	/** Belirtilen duruma HEMEN bir geçiş başlatır (RollNextWeather'ı atlar) — dev-tool/test için. */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	bool ForceWeather(EWeatherCondition NewCondition);

	const FWeatherState& GetTransitionStartState() const { return TransitionStartState; }
	const FWeatherState& GetTargetState() const { return TargetState; }
	float GetTransitionElapsed() const { return TransitionElapsed; }
	float GetTimeSinceLastEvaluation() const { return TimeSinceLastEvaluation; }

	void RestoreStateForLoad(const FWeatherState& InCurrent, const FWeatherState& InTransStart,
		const FWeatherState& InTarget, float InTransProgress, float InTransElapsed, float InTimeSinceEval);

	UPROPERTY(BlueprintAssignable, Category = "Weather")
	FOnWeatherStateChanged OnWeatherStateChanged;

private:
	void EvaluateWeather();
	void BeginTransitionTo(EWeatherCondition NewCondition);
	EWeatherCondition RollNextWeather() const;

	float TimeSinceLastEvaluation = 0.0f;
	float TransitionElapsed = 0.0f;
	bool bHasEvaluatedOnce = false;

	// EvaluateWeather'in "ayni durum secildi, gecise gerek yok" kisayolunun struct-varsayilanlarindaki
	// (hicbir zaman veritabanindan doldurulmamis) CurrentState'i "zaten oraya gecis yapilmis" sanip
	// BeginTransitionTo'yu HIC cagirmamasini onler (inceleme bulgusu) — ILK degerlendirme HER ZAMAN
	// gercek bir gecis baslatir, RollNextWeather ne donerse donsun.
	bool bHasInitializedState = false;

protected:
	// PIE/MCP dogrulamasi icin okunabilir (GetCurrentState() zaten public getter — Sistem #10
	// EnergyBudget/ThermalBudget ile ayni desen; private uyede BlueprintReadOnly UHT hatasi verir).
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	FWeatherState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	FWeatherState TransitionStartState;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	FWeatherState TargetState;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float TransitionProgress = 1.0f;
};
