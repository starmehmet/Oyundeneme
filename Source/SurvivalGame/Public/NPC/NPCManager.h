#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "NPCManager.generated.h"

class UNPCBrain;

/**
 * Sistem #15 — "Kare bölümleme" (`UProductionManager`, Sistem #9, ile BİREBİR aynı desen —
 * `SurvivalProduction::ComputeBatchSize` doğrudan yeniden kullanılır, NPC'lere özel ikinci bir
 * batch-hesaplama fonksiyonu YAZILMADI). Her `Tick`'te TÜM kayıtlı NPC beyinlerini
 * güncellemek yerine bir DİLİM (round-robin) güncellenir; her beyin kendi
 * `LastBrainUpdateTime`'ını tuttuğu için (`UNPCBrain::AdvanceBrain`) seyrek güncellenen bir
 * NPC "yavaş düşünmüyor" — bir sonraki turunda GERÇEK geçen süreyi tek seferde işler.
 */
UCLASS()
class SURVIVALGAME_API UNPCManager : public UWorldSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UNPCManager, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void RegisterBrain(UNPCBrain* Brain);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void UnregisterBrain(UNPCBrain* Brain);

	UFUNCTION(BlueprintPure, Category = "NPC")
	int32 GetBrainCount() const { return Brains.Num(); }

private:
	UPROPERTY()
	TArray<TObjectPtr<UNPCBrain>> Brains;

	double GameTime = 0.0;
	int32 PartitionIndex = 0;

	// CLAUDE.md ornegi: "500 makine / 60 frame" — bir dongude TUM NPC'ler en az bir kez guncellenir.
	static constexpr int32 FramesPerCycle = 60;
};
