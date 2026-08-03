#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NPC/TaskDefinition.h"
#include "TaskScheduler.generated.h"

class UNPCBrain;

/**
 * Sistem #16 — MIMARI.md'nin `UTaskScheduler`'ı. `FTickableGameObject` DEĞİLDİR — tamamen
 * OLAY-GÜDÜMLÜ (`EnqueueTask` + her beynin `OnTaskCompleted`/`OnTaskFailed` delegate'lerine
 * abone olunur), sürekli bir Tick'e ihtiyaç yok (bkz. `UWindSimulation`'ın aynı gerekçesi,
 * Sistem #13).
 *
 * MIMARI'nin `TMap<ANPCCharacter*, UNPCTask*>` yerine `TMap<TWeakObjectPtr<UNPCBrain>,
 * FTaskDefinition>` kullanılır — atanan görevin TAM tanımını (yeniden kuyruğa almak için
 * gerekli) tutar, `UNPCTask` polymorphic nesnesi hiç YAZILMADI (bkz. `FNPCTaskData` ADR'si,
 * Sistem #15).
 *
 * Kuyruk MIMARI'nin "heap" imasının AKSİNE düz bir `TArray` + doğrusal tarama
 * (`SurvivalTaskScheduler::FindBestEligibleTaskIndex`) — beceri-eşleştirme VE backoff
 * filtrelemesi bir öncelik-heap'iyle temiz bileşmez (koşullu atlamalar heap düzenini bozar);
 * beklenen ölçekte (bir yerleşimdeki onlarca görev/NPC) doğrusal tarama zaten ucuz (bkz. ADR).
 */
UCLASS()
class SURVIVALGAME_API UTaskScheduler : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Task")
	void EnqueueTask(const FTaskDefinition& TaskDef);

	/** `UNPCBrain::BeginPlay`'den çağrılır (`UNPCManager::RegisterBrain` ile aynı desen). */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void RegisterBrain(UNPCBrain* Brain);

	UFUNCTION(BlueprintCallable, Category = "Task")
	void UnregisterBrain(UNPCBrain* Brain);

	UFUNCTION(BlueprintPure, Category = "Task")
	int32 GetPendingTaskCount() const { return PendingTasks.Num(); }

	UFUNCTION(BlueprintPure, Category = "Task")
	int32 GetActiveAssignmentCount() const { return ActiveAssignments.Num(); }

	const TArray<FTaskDefinition>& GetPendingTasks() const { return PendingTasks; }

	void RestorePendingTasksForLoad(const TArray<FTaskDefinition>& InTasks);

private:
	UFUNCTION()
	void HandleTaskCompleted(UNPCBrain* Brain, FName TaskID);

	UFUNCTION()
	void HandleTaskFailed(UNPCBrain* Brain, FName TaskID);

	void TryAssignBestTaskTo(UNPCBrain* Brain);
	void TryAssignToAnyIdleBrain();
	void PruneStaleBrains();
	double GetCurrentGameTime() const;

	UPROPERTY()
	TArray<FTaskDefinition> PendingTasks;

	// Sistem #10/#12/#13 ile ayni desen: aktor/bilesen-anahtarli kayitlar TWeakObjectPtr
	// olarak, UPROPERTY OLMADAN saklanir (ResourceSimulation::EnergyProducers ile birebir).
	TArray<TWeakObjectPtr<UNPCBrain>> RegisteredBrains;
	TMap<TWeakObjectPtr<UNPCBrain>, FTaskDefinition> ActiveAssignments;
};
