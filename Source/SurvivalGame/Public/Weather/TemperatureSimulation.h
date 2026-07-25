#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Weather/ThermalZone.h"
#include "TemperatureSimulation.generated.h"

class AActor;
class APlayerCharacter;

/**
 * Sistem #12 — Ortam/vücut sıcaklığının tek sahibi. `UResourceSimulation`/`UWeatherSimulation`
 * ile aynı desen: `UGameInstanceSubsystem` + `FTickableGameObject`. MIMARI.md'nin
 * `GetInteriorTemperature(Building, DeltaTime)` imzası burada `GetEnvironmentTemperatureAt(Position)`
 * olarak sadeleştirildi — ayrı, kalıcı bir termal-kütle modeli YOK, iç sıcaklık her sorguda
 * anlık hesaplanır (bkz. `SurvivalTemperature::ComputeInteriorTemperature`, ADR).
 *
 * Bölge kaydı `TWeakObjectPtr` ile Sistem #10/#11'deki kendiliğinden-temizlenen registry
 * desenini tekrarlar (`PruneStaleRegistrations`, her Tick başında).
 *
 * Oyuncunun termal stresi her Tick'te doğrudan bu sınıf tarafından uygulanır — projenin
 * "tek-oyunculu varsayım" kuralına göre ayrı bir oyuncu-kaydı YOK, `GetFirstPlayerController`
 * ile bulunur (Sistem #10'un konsol komutlarındaki `FindPlayerPawn` deseniyle aynı fikir).
 */
UCLASS()
class SURVIVALGAME_API UTemperatureSimulation : public UGameInstanceSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UTemperatureSimulation, STATGROUP_Tickables);
	}

	/** Hava durumu sıcaklığı + gün/gece salınımı — konumdan bağımsız (global hava, bkz. UWeatherSimulation). */
	UFUNCTION(BlueprintPure, Category = "Temperature")
	float GetAmbientTemperature() const;

	/** Kayıtlı bir bölge içindeyse o bölgenin iç sıcaklığı, değilse saf ortam sıcaklığı. İlk-eşleşen bölge kullanılır. */
	UFUNCTION(BlueprintPure, Category = "Temperature")
	float GetEnvironmentTemperatureAt(const FVector& Position) const;

	UFUNCTION(BlueprintCallable, Category = "Temperature")
	void RegisterThermalZone(AActor* ZoneActor, const FThermalZone& ZoneData);

	UFUNCTION(BlueprintCallable, Category = "Temperature")
	void UnregisterThermalZone(AActor* ZoneActor);

	/** Bir oyuncunun vücut sıcaklığını çevreye doğru sürükler, güvenli aralık dışındaysa hasar uygular. */
	void ApplyPlayerThermalStress(APlayerCharacter* Player, float DeltaTime);

private:
	void PruneStaleRegistrations();

	TMap<TWeakObjectPtr<AActor>, FThermalZone> ThermalZones;

protected:
	// PIE/MCP dogrulamasi icin okunabilir (getter'lar zaten var — Sistem #10/#11 ile ayni
	// desen; private uyede BlueprintReadOnly UHT hatasi verir).
	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float LastAmbientTemperature = 20.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float LastPlayerEnvironmentTemperature = 20.0f;
};
