#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "ProductionManager.generated.h"

class AProductionMachine;

/**
 * Sistem #9 — "Kare bölümleme" (frame partitioning, CLAUDE.md: "500 makine / 60 frame; sınırsız
 * per-frame O(n) döngü ekleme"). Her `Tick`'te TÜM kayıtlı makineleri güncellemek yerine,
 * `SurvivalProduction::ComputeBatchSize` ile hesaplanan bir DİLİM (round-robin) güncellenir —
 * `FramesPerCycle` kare içinde her makine en az bir kez güncellenmiş olur.
 *
 * Bu, güncellemeler arası GERÇEKTEN geçen süreyi kaybetmeden yapılır: her makine kendi
 * `LastProductionUpdateTime`'ını tutar (`AProductionMachine::AdvanceProduction`), bu yüzden
 * seyrek güncellenen bir makine "yavaş üretir" DEĞİL — bir sonraki turunda o ana kadar
 * biriken GERÇEK süreyi tek seferde işler (üretim matematiği kare-oranından bağımsız kalır,
 * yalnızca CPU maliyeti dağıtılır).
 */
UCLASS()
class SURVIVALGAME_API UProductionManager : public UWorldSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(UProductionManager, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintCallable, Category = "Production")
	void RegisterMachine(AProductionMachine* Machine);

	UFUNCTION(BlueprintCallable, Category = "Production")
	void UnregisterMachine(AProductionMachine* Machine);

	UFUNCTION(BlueprintPure, Category = "Production")
	int32 GetMachineCount() const { return Machines.Num(); }

private:
	UPROPERTY()
	TArray<TObjectPtr<AProductionMachine>> Machines;

	double GameTime = 0.0;
	int32 PartitionIndex = 0;

	// CLAUDE.md ornegi: "500 makine / 60 frame" — bir dongude TUM makineler en az bir kez guncellenir.
	static constexpr int32 FramesPerCycle = 60;
};
