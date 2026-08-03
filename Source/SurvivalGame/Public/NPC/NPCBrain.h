#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPC/NPCState.h"
#include "NPC/NPCTaskData.h"
#include "NPCBrain.generated.h"

class UNPCBrain;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCStateChanged, ENPCState, OldState, ENPCState, NewState);

/** Sistem #16 — `UTaskScheduler`'ın HANGİ beynin tamamladığını/başarısız olduğunu bilmesi için
 * beyin işaretçisi de taşır (aynı işleve birden çok beyin bağlanacağından, yalnızca TaskID
 * yeterli olmazdı). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCTaskCompleted, UNPCBrain*, Brain, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCTaskFailed, UNPCBrain*, Brain, FName, TaskID);

/**
 * Sistem #15 — Bir NPC'nin durum makinesi + yorgunluk/moral takibi. Sahibinin (`ANPCCharacter`)
 * `UHealthComponent`'ini (Sistem #12'den YENİDEN KULLANILIR — NPC'ye özel ikinci bir can alanı
 * YOK) ve global `UTimeKeeper`'ı (Sistem #1, uyku saati için) okur.
 *
 * Kendi `PrimaryComponentTick`'i YOK — `UNPCManager` tarafından kare-bölümlemeli olarak
 * (`AdvanceBrain`) tetiklenir; `AProductionMachine`/`UProductionManager` (Sistem #9) ile
 * BİREBİR aynı desen (CLAUDE.md: "yeni her sürekli-çalışan sistem frame bölümlemeli" — bu
 * kural şimdiye kadarki her "çoklu-örnek" sistemde uygulandı, NPC'ler de aynı sınıf).
 *
 * Yürüme (Walking durumu), motorun kendi NAVMESH + `AAIController::MoveToLocation`'ı ile
 * yapılır — ÖZEL bir yol bulma/graf sistemi YAZILMADI (motorda zaten var olanı yeniden
 * kullan kuralı, bkz. ADR). Test haritasında bir `NavMeshBoundsVolume` GEREKİR, yoksa
 * `MoveToLocation` sessizce başarısız olur (log'da görülür).
 */
UCLASS(ClassGroup = (NPC), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UNPCBrain : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCBrain();

	/** `UNPCManager` tarafından çağrılır — `AProductionMachine::AdvanceProduction` ile aynı
	 * "gerçek geçen süre" muhasebesi (bkz. o sınıfın yorumu, Sistem #9). */
	void AdvanceBrain(double CurrentGameTime);

	void ResetBrainClock(double CurrentGameTime) { LastBrainUpdateTime = CurrentGameTime; }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void AssignTask(const FNPCTaskData& NewTask);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void ClearTask();

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool HasTask() const { return bHasTask; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	ENPCState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	float GetFatigue() const { return Fatigue; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	float GetMorale() const { return Morale; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	int32 GetSkillLevel() const { return SkillLevel; }

	const FNPCTaskData& GetCurrentTask() const { return CurrentTask; }
	float GetWorkElapsedTime() const { return WorkElapsedTime; }
	float GetWalkingElapsedTime() const { return WalkingElapsedTime; }

	void RestoreStateForLoad(ENPCState InState, float InFatigue, float InMorale,
		bool bInHasTask, const FNPCTaskData& InTask, float InWorkElapsed, float InWalkElapsed);

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCStateChanged OnStateChanged;

	/** Sistem #16 — `CurrentTask.WorkDuration` saniyesi kadar Working durumunda kalınca ateşlenir. */
	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCTaskCompleted OnTaskCompleted;

	/** Sistem #16 — `MaxWalkingDuration`'dan uzun süre hedefe VARAMADAN Walking'de kalınca ateşlenir. */
	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCTaskFailed OnTaskFailed;

private:
	void EvaluateState();
	void OnStateEntered(ENPCState NewState);
	void OnStateExited(ENPCState OldState);
	class AAIController* GetAIController() const;
	void CompleteCurrentTask();
	void FailCurrentTask();

	double LastBrainUpdateTime = 0.0;

	// Sistem #16 — CurrentTask'in gorev-tamamlama/basarisizlik zamanlayicilari. KESINTIDE
	// (Hurt/Sleeping'e gecince) SIFIRLANMAZ — "gorev durumunu devam ettir" DoD'si: is yarida
	// kesilip devam edilirse KALDIGI yerden surer, bastan baslamaz. Yalnizca AssignTask
	// (GERCEKTEN yeni bir gorev) sifirlar.
	float WorkElapsedTime = 0.0f;
	float WalkingElapsedTime = 0.0f;

protected:
	// AProductionMachine/UHealthComponent ile ayni desen (bkz. o siniflarin BeginPlay/EndPlay'i)
	// — inceleme bulgusu: public'te bulunursa disaridan (ornegin gelecekteki bir task-scheduler)
	// yanlislikla dogrudan cagrilip UNPCManager'dan sessizce kaydi silinebilirdi.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// PIE/MCP dogrulamasi icin okunabilir (getter'lar zaten var — Sistem #9/#10/#11/#12 ile
	// ayni desen; private uyede BlueprintReadOnly UHT hatasi verir).
	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	ENPCState CurrentState = ENPCState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	float Fatigue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	float Morale = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	FNPCTaskData CurrentTask;

	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	bool bHasTask = false;

public:
	/** Sistem #16 — `UTaskScheduler`'ın beceri-eşleştirmesinde kullanılır (bkz.
	 * `SurvivalTaskScheduler::IsNPCQualified`). Bina/BP seviyesinde tasarımcı tarafından atanır. */
	UPROPERTY(EditAnywhere, Category = "NPC")
	int32 SkillLevel = 1;
};
