#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DayNightCycle.generated.h"

class ADirectionalLight;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSunrise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSunset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayNightStateChanged, bool, bIsDaytime);

/**
 * Sistem #1 — Güneş ve ışık sahibi. TimeKeeper'dan zamanı OKUR (poll — init sırası
 * bağımsızlığı için delegate yerine tercih edildi), directional light'ı döndürür,
 * şiddeti gün/gece arası geçirir, gün/gece olaylarını yayınlar.
 * Sahnedeki güneş: "Sun" tag'li directional light; yoksa bulunan ilki.
 *
 * Olay sözleşmesi:
 * - OnDayNightStateChanged: İLK değerlendirmede mevcut durumla ve sonraki her geçişte
 *   ateşlenir — durum-güdümlü dinleyiciler (spawner vb.) BUNU kullanmalı.
 * - OnSunrise/OnSunset: yalnızca canlı kenar geçişlerinde ateşlenir. Zaman sıçramalarında
 *   (time_sethour, save yükleme) ve level geçişlerinde ARADA KALAN kenarlar ATLANABİLİR;
 *   kenar saymayın, gerekiyorsa IsDaytimeNow() ile durumu poll edin.
 */
UCLASS()
class SURVIVALGAME_API UDayNightCycle : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// UWorldSubsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UDayNightCycle, STATGROUP_Tickables);
	}

	/** Sahne güneşini elle ata — otomatik bulunanı geçersiz kılar. */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void RegisterSunLight(ADirectionalLight* InSunLight);

	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsDaytimeNow() const;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnSunrise OnSunrise;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnSunset OnSunset;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnDayNightStateChanged OnDayNightStateChanged;

private:
	void UpdateSun(double MinuteOfDayF);

	/** Sunrise < Sunset garantili okunmuş ayarlar; bozuksa bir kez uyarır ve onarır. */
	void GetSanitizedSunTimes(double& OutSunrise, double& OutSunset) const;

	TWeakObjectPtr<ADirectionalLight> SunLight;
	TOptional<bool> bWasDaytime;
	/** Işığın sahnedeki yaw'ı — rotasyon MUTLAK yazılır (oku-değiştir-yaz YASAK:
	    GetActorRotation pitch'i [-90,+90]'a normalize eder, geri okuma yörüngeyi bozar). */
	double SunYaw = 0.0;
	bool bWarnedNoSun = false;
	mutable bool bWarnedBadSunTimes = false;
};
