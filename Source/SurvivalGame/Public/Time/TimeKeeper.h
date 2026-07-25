#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "TimeKeeper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinuteChanged, int32, MinuteOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHourChanged, int32, HourOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, DayNumber);

/**
 * Sistem #1 — Zamanın tek sahibi. Oyun-içi toplam saniyeyi biriktirir, dakika/saat/gün
 * devrilmelerinde delegate yayınlar. Zaman delegate'leri BURADA yaşar; DayNightCycle
 * yalnızca güneş/ışık + sunrise/sunset sahibidir (MIMARI.md'den bilinçli sapma, bkz. ADR).
 *
 * Saat kapısı: Zaman yalnızca StartClock() sonrası akar (bkz. ASurvivalGameMode::StartPlay).
 * Ana menü gibi oynanış-dışı haritalarda saat DURUR; yeni oyun ResetForNewGame() ile başlar.
 *
 * Delegate sözleşmesi: SetTimeOfDay/SetTotalGameSeconds sıçramalarında ARA devrilmeler
 * ATLANIR — her delegate yalnızca son değerle 1 kez ateşlenir (tam 24 saatlik sıçramada hiç).
 * Dinleyiciler sayaç artırmak yerine payload değerini kullanmalıdır.
 *
 * Multiplayer notu: Zaman replike EDİLMEZ — tek oyunculu varsayım. Sistem #21'de saat
 * sahipliği server'a alınıp GameState üzerinden replike edilmeli.
 *
 * Kaydet/yükle: SaveGameManager Get/SetTotalGameSeconds ile durumu taşır.
 */
UCLASS()
class SURVIVALGAME_API UTimeKeeper : public UGameInstanceSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UTimeKeeper, STATGROUP_Tickables);
	}

	// Saat kapısı
	UFUNCTION(BlueprintCallable, Category = "Time")
	void StartClock();

	UFUNCTION(BlueprintCallable, Category = "Time")
	void StopClock();

	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsClockRunning() const { return bClockRunning; }

	/** Yeni oyun: saati ayarlardaki StartHour'a sıfırlar (sessiz — delegate ateşlemez). */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void ResetForNewGame();

	// Sorgular
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetMinuteOfDay() const;

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetHourOfDay() const;

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetDayNumber() const;

	/** Kesirli günün dakikası [0, 1440) — akıcı güneş dönüşü için. */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetMinuteOfDayFloat() const;

	UFUNCTION(BlueprintPure, Category = "Time")
	float GetTimeScale() const { return TimeScale; }

	// Komutlar
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeScale(float NewScale);

	/** Saati ayarlar, gün numarasını korur. Sıçramada ara devrilmeler atlanır (üstteki sözleşme). */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(int32 Hour, int32 Minute);

	// Kaydet/yükle arayüzü (Sistem #17)
	double GetTotalGameSeconds() const { return TotalGameSeconds; }
	void SetTotalGameSeconds(double NewTotal);

	// Olaylar
	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnMinuteChanged OnMinuteChanged;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnHourChanged OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnDayChanged OnDayChanged;

private:
	void ApplyStartTimeFromSettings();
	void BroadcastRollovers();

	double TotalGameSeconds = 0.0;
	float TimeScale = 10.0f;
	bool bClockRunning = false;

	int32 LastBroadcastMinute = -1;
	int32 LastBroadcastHour = -1;
	int32 LastBroadcastDay = -1;
};
