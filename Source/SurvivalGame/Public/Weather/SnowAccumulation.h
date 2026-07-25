#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "SnowAccumulation.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvalancheRiskChanged, bool, bInRisk);

/**
 * Sistem #14 — Kar derinliğinin (ve ondan türeyen hareket cezası/inşaat-engeli/çığ-riskinin)
 * tek sahibi. MIMARI.md'nin `FSnowAccumulationGrid` (`TMap<FIntPoint, float> SnowDepth`)
 * taslağı YERİNE TEK bir global `CurrentSnowDepth` (Sistem #10'un `FThermalBudget::
 * CurrentTemperature`'ıyla AYNI desen) — bu projede hava durumu VE sıcaklık zaten TAMAMEN
 * global (konumdan bağımsız); per-cell bir ızgara, her hücre HER ZAMAN aynı değere
 * yakınsayacağından hiçbir simülasyon değeri katmadan yalnızca aynı float'ın kopyalarını
 * tutardı (bkz. ADR). Konumsal olan TEK şey çığ riski (yamaç eğimi) — bu, oyuncunun bulunduğu
 * noktada bir yer-normali line-trace'iyle sorgulanır, ayrı bir ızgara GEREKMEZ.
 *
 * `UResourceSimulation`/`UWeatherSimulation` ile aynı desen: `UGameInstanceSubsystem` +
 * `FTickableGameObject`. Hava (Snowing/Blizzard + Precipitation) ve ortam sıcaklığı
 * (`UTemperatureSimulation::GetAmbientTemperature`, Sistem #12'den yeniden kullanılır — paralel
 * bir "kaç derece" kavramı YOK) HER Tick'te birikim/erimeyi besler. Oyuncu hareket cezası da HER
 * Tick uygulanır (tek-oyunculu varsayım, Sistem #12'nin `ApplyPlayerThermalStress`'iyle aynı
 * desen). Çığ riski PAHALI bir line-trace gerektirdiğinden yalnızca `AvalancheCheckInterval`de
 * bir değerlendirilir (Sistem #11'in `EvaluationInterval` deseniyle aynı gerekçe) ve
 * EDGE-TRIGGERED yayınlanır (Sistem #10'un `OnScarcityStateChanged`'ıyla aynı felsefe — bu bir
 * durum DEĞİŞİKLİĞİ olayı, sürekli-anlamlı bir değer değil).
 */
UCLASS()
class SURVIVALGAME_API USnowAccumulation : public UGameInstanceSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(USnowAccumulation, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintPure, Category = "Snow")
	float GetCurrentSnowDepth() const { return CurrentSnowDepth; }

	UFUNCTION(BlueprintPure, Category = "Snow")
	float GetMovementSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Snow")
	bool IsConstructionBlocked() const;

	UFUNCTION(BlueprintPure, Category = "Snow")
	bool IsInAvalancheRisk() const { return bInAvalancheRisk; }

	/** Dev-tool/test: derinliği doğrudan ayarlar (hava/sıcaklık simülasyonunu beklemeden). */
	UFUNCTION(BlueprintCallable, Category = "Snow")
	void SetSnowDepthForTesting(float NewDepth);

	UPROPERTY(BlueprintAssignable, Category = "Snow")
	FOnAvalancheRiskChanged OnAvalancheRiskChanged;

private:
	void ApplyMovementPenalty(UWorld* World);
	void EvaluateAvalancheRisk(UWorld* World);

	float TimeSinceLastAvalancheCheck = 0.0f;

	// Hangi piyon icin BaseWalkSpeed'in onbelleklendigini de tutar — yalnizca bir bool bayrak
	// olsaydi (ilk gordugumde ONBELLEKLE, bir daha asla) piyon degisirse (gelecekteki bir
	// respawn/OpenLevel akisi) YENI piyonun KENDI MaxWalkSpeed'i asla yakalanmaz, ESKI piyonun
	// degeriyle sessizce ezilirdi (inceleme bulgusu — bu projenin "yok edilen/degisen aktore
	// bagli onbellek/kayit bayatlar" hata sinifinin bir baska gorunumu).
	TWeakObjectPtr<ACharacter> BaseWalkSpeedCachedFor;
	float BaseWalkSpeed = 0.0f;

	bool bInAvalancheRisk = false;

protected:
	// PIE/MCP dogrulamasi icin okunabilir (GetCurrentSnowDepth() zaten public getter — Sistem
	// #10/#11/#12 ile ayni desen; private uyede BlueprintReadOnly UHT hatasi verir).
	UPROPERTY(BlueprintReadOnly, Category = "Snow")
	float CurrentSnowDepth = 0.0f;
};
