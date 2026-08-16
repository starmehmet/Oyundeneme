#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "SnowVfxSubsystem.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Sistem #14 (Kar) görsel katmanı — gerçek düşen kar taneleri.
 *
 * Proje VFX/Niagara içeriği barındırmadığından (ve Niagara asset'i MCP/C++'tan yazılamadığından),
 * kar Niagara yerine SAF C++ ile üretilir: kameranın etrafındaki bir kutuda binlerce küçük beyaz
 * küre (engine `/Engine/BasicShapes/Sphere` + `BasicShapeMaterial`'dan türetilmiş beyaz dinamik
 * materyal) `UInstancedStaticMeshComponent` ile tutulur. Her tane kendi boyutu/hızı/salınım faziyla
 * (per-tane varyasyon = doğal kar) aşağı savrularak düşer ve kutudan çıkınca tepeden yeniden doğar
 * (kamera hareket ettikçe kutu onu takip eder). Yalnızca hava `Snowing` ya
 * da `Blizzard` iken aktif — `UWeatherSimulation::GetCurrentState()`'ten okunur (Sistem #11 tek
 * doğruluk kaynağı). RNG (FMath::FRand) BURADA meşru: bu gameplay değil kozmetik VFX, determinizm
 * gerekmez (Sistem #14'ün simülasyon çekirdeği hâlâ deterministik).
 */
UCLASS()
class SURVIVALGAME_API USnowVfxSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(USnowVfxSubsystem, STATGROUP_Tickables); }
	// HER ZAMAN tickable — Tick'in kendisi EnsureInitialized ile lazy kurulum yapar. bInitialized
	// dondurmek tavuk-yumurta olurdu (bInitialized yalniz Tick icinde set edilir -> hic tick etmez).
	// Oyun-disi dunyalar DoesSupportWorldType ile zaten eleniyor.
	virtual bool IsTickable() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

	// Yalnizca gercek oyun/PIE dunyalarinda calis (editor preview / inspector dunyalarinda degil).
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

private:
	void EnsureInitialized();
	void SetSnowingActive(bool bNewActive, const FVector& CameraLocation);

	UPROPERTY()
	TObjectPtr<AActor> SnowHost;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> SnowISM;

	// Her tanenin durumu (kutu kamera-goreli takip eder). Per-tane varyasyon = daha gercekci kar:
	// farkli boyut (kucuk/buyuk karisim), farkli dusme hizi, farkli salinim fazi (savrularak duser).
	TArray<FVector> FlakePositions;
	TArray<float> FlakeSpeeds;   // per-tane dusme hizi
	TArray<float> FlakeScales;   // per-tane boyut
	TArray<float> FlakePhases;   // per-tane yatay salinim fazi

	// Salinim (sway) icin zaman birikimi — Date::Now kullanmadan (resume-guvenli), Tick'te DeltaTime toplanir.
	double ElapsedTime = 0.0;

	bool bInitialized = false;
	bool bActive = false;
};
