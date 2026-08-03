#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Production/ResourceTypes.h"
#include "Production/ScarcityReason.h"
#include "ResourceSimulation.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScarcityStateChanged, bool, bInScarcity, EScarcityReason, Reason);

/**
 * Sistem #10 — Global enerji/termal/yakıt ekonomisinin tek sahibi. `ULogisticsNetwork`
 * (Sistem #8) ile aynı desen: `UGameInstanceSubsystem` + `FTickableGameObject`.
 *
 * Üretici/tüketici KAYDI `AActor*` anahtarlıdır (MIMARI.md'nin imzasıyla aynı) ama BİLEREK
 * `TWeakObjectPtr` olarak saklanır ve her `Tick`'te önce BAYATLAMIŞ (yok edilmiş ama
 * kaydı silinmemiş) girişler temizlenir — Sistem #7/#8/#9'un incelemelerinde tekrar tekrar
 * çıkan "aktör yok edildi ama kaydı kalıcı olarak kaldı" hata sınıfına karşı BAŞTAN savunmacı
 * (bu sefer inceleme bulgusunu BEKLEMEDEN uygulandı).
 *
 * Enerji Frequency'si (bkz. `FEnergyBudget`) ANLIK türetilir; termal `CurrentTemperature`
 * BİRİKİMLİDİR (gerçek termal atalet modeli — bkz. `FThermalBudget` yorumu). Bu BİLİNÇLİ bir
 * asimetri, tutarsızlık değil.
 *
 * Isı DAĞITIMI için ayrı bir "soğutucu" kaydı YOK — sabit `UResourceSimulationSettings::
 * BaselineHeatDissipation` kullanılır (henüz radyatör/soğutucu içeriği yok); enerji ile
 * simetrik tam üretici/tüketici kaydı YALNIZCA gerçek ihtiyaç (soğutucu bina) ortaya çıkınca
 * eklenir (bkz. ADR).
 *
 * Yakıt rezervleri (`FuelReserves`) enerji üretici/tüketici muhasebesinden BAĞIMSIZDIR —
 * ikisini birbirine bağlamak (yakıt tüketip enerji üretmek) gerçek bir üretici/jeneratör
 * içeriği gerektirir, bu pasoda yok (bkz. ADR).
 */
UCLASS()
class SURVIVALGAME_API UResourceSimulation : public UGameInstanceSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UResourceSimulation, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void RegisterEnergyProducer(AActor* Producer, float OutputPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void UnregisterEnergyProducer(AActor* Producer);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void RegisterEnergyConsumer(AActor* Consumer, float ConsumptionPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void UnregisterEnergyConsumer(AActor* Consumer);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void RegisterHeatProducer(AActor* Producer, float HeatPerSecond);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void UnregisterHeatProducer(AActor* Producer);

	/**
	 * FuelID önceden yoksa yeni bir rezerv açar (MaxAmount = ilk eklenen miktar veya verilen
	 * MaxAmount). GERÇEKTE kabul edilen miktarı döner (`UInventoryComponent::AddItem` ile aynı
	 * desen) — rezerv zaten doluysa (ve bu çağrıda MaxAmount büyütülmediyse) istenen miktardan
	 * AZ dönebilir; çağıran taraf bunu kontrol etmeli (inceleme bulgusu: eskiden void'di,
	 * tavana ulaşan fazlalık sessizce kaybolurdu, hiçbir sinyal yoktu).
	 */
	UFUNCTION(BlueprintCallable, Category = "Resource")
	float AddFuel(FName FuelID, float Amount, float MaxAmount = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetFuelConsumptionRate(FName FuelID, float ConsumptionRate);

	UFUNCTION(BlueprintPure, Category = "Resource")
	const FEnergyBudget& GetEnergyBudget() const { return EnergyBudget; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	const FThermalBudget& GetThermalBudget() const { return ThermalBudget; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	bool GetFuelReserve(FName FuelID, FFuelReserve& OutReserve) const;

	UFUNCTION(BlueprintPure, Category = "Resource")
	bool IsInScarcity() const { return bInScarcity; }

	UFUNCTION(BlueprintPure, Category = "Resource")
	EScarcityReason GetScarcityReason() const { return CurrentScarcityReason; }

	const TMap<FName, FFuelReserve>& GetFuelReserves() const { return FuelReserves; }

	void RestoreStateForLoad(const TMap<FName, FFuelReserve>& InFuelReserves, float InThermalTemp);

	UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnScarcityStateChanged OnScarcityStateChanged;

private:
	void PruneStaleRegistrations();
	void RecomputeEnergyBudget();
	void RecomputeThermalBudget(float DeltaTime);
	void DepleteFuelReserves(float DeltaTime);
	void EvaluateScarcity();

	UPROPERTY()
	TMap<FName, FFuelReserve> FuelReserves;

	TMap<TWeakObjectPtr<AActor>, float> EnergyProducers;
	TMap<TWeakObjectPtr<AActor>, float> EnergyConsumers;
	TMap<TWeakObjectPtr<AActor>, float> HeatProducers;

	bool bInScarcity = false;
	EScarcityReason CurrentScarcityReason = EScarcityReason::None;

protected:
	// PIE/MCP dogrulamasi icin okunabilir (GetEnergyBudget()/GetThermalBudget() zaten public
	// getter — bu yalnizca MCP'nin fonksiyon degil property okuyabilmesi icin, Sistem #3
	// InteractionCount / Sistem #4 CurrentWeight ile ayni desen; private uyede BlueprintReadOnly
	// UHT hatasi verir, bu yuzden protected).
	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	FEnergyBudget EnergyBudget;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	FThermalBudget ThermalBudget;
};
