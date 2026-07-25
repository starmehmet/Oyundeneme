#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "WorldPartitionHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCellLoadStateChanged, FIntPoint, CellCoord, bool, bIsLoaded);

/**
 * Sistem #18 — MIMARI.md'nin `UWorldPartitionHelper`'ı; motorun GERÇEK World Partition
 * özelliği (harita-dönüştürme, HLOD, editör-öncelikli akış) YERİNE MIMARI'nin taslağına
 * sadık, hafif bir özel sistem (bkz. ADR — motorun native World Partition'ı bu projenin
 * mevcut tek-seviyeli, düz haritasını kökten değiştirmeyi gerektirirdi, ölçek/risk
 * dengesizliği çok büyük).
 *
 * DÜRÜST kapsam: "boşaltma" GERÇEK spawn/destroy DEĞİL — aktörler `SetActorHiddenInGame`+
 * `SetActorEnableCollision` ile gizlenir/collision'ı kapatılır, ASLA yok edilmez. Bu, DoD'nin
 * "hücre durumunu koru" maddesini OTOMATİK karşılar (hiçbir şey yok edilmediği için
 * geri-yükleyecek bir durum yok) ama "bellek kullanımını optimize et" maddesini TAM
 * karşılamaz — bellek ayrımı aynı kalır, yalnızca render/collision/simülasyon MALİYETİ
 * azalır. Gerçek bellek-azaltan spawn/destroy akışı, her sistemin kendi kaydet/geri-yükle
 * uç noktasını (Sistem #17'nin şu an KAPSAM DIŞI bıraktığı) gerektirir — bkz. ADR.
 *
 * `UProductionManager`/`UNPCManager` (Sistem #9/#15), boşaltılmış hücrelerdeki makine/NPC'leri
 * `AdvanceProduction`/`AdvanceBrain` ile İLERLETMEYİ ATLAR — küçük, additive bir kontrol
 * (bkz. o sınıfların `Tick`'i). Bu YENİ bir "duraklat" semantiği İCAT ETMEZ: her nesne zaten
 * kendi `LastUpdateTime`'ını tuttuğundan (Sistem #9'un "gerçek geçen süre asla kaybolmaz"
 * garantisi), hücre yeniden yüklenince bir SONRAKİ ilerletme GERÇEK geçen süreyi otomatik
 * telafi eder.
 */
UCLASS()
class SURVIVALGAME_API UWorldPartitionHelper : public UWorldSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UWorldPartitionHelper, STATGROUP_Tickables);
	}

	/** Bu konumun ait olduğu hücre YÜKLÜ mü — henüz taranmamış/bilinmeyen bir konum için
	 * (güvenli varsayılan) `true` döner, yoksa yeni içerik sessizce donardı. */
	UFUNCTION(BlueprintPure, Category = "WorldPartition")
	bool IsPositionLoaded(const FVector& Position) const;

	UFUNCTION(BlueprintPure, Category = "WorldPartition")
	int32 GetLoadedCellCount() const;

	UFUNCTION(BlueprintPure, Category = "WorldPartition")
	int32 GetTotalCellCount() const { return Cells.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "WorldPartition")
	FOnCellLoadStateChanged OnCellLoadStateChanged;

private:
	struct FWorldPartitionCell
	{
		TArray<TWeakObjectPtr<AActor>> ContainedActors;
		bool bIsLoaded = true;
	};

	void RebuildCellRegistry();
	void EvaluateCellLoading();
	void SetCellLoaded(const FIntPoint& CellCoord, FWorldPartitionCell& Cell, bool bLoaded);
	AActor* FindPlayerPawn() const;

	// TWeakObjectPtr-degerli konteynerlar UPROPERTY DEGIL — ResourceSimulation::EnergyProducers
	// (Sistem #10) ile ayni desen.
	TMap<FIntPoint, FWorldPartitionCell> Cells;

	float TimeSinceLastEvaluation = 0.0f;
};
