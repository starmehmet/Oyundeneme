#include "NPC/NPCBrain.h"
#include "NPC/NPCMath.h"
#include "NPC/NPCBrainSettings.h"
#include "NPC/NPCManager.h"
#include "NPC/TaskScheduler.h"
#include "Player/HealthComponent.h"
#include "Time/TimeKeeper.h"
#include "SurvivalGame.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UNPCBrain::UNPCBrain()
{
	PrimaryComponentTick.bCanEverTick = false; // UNPCManager kare-bolumlemeli tikler (AdvanceBrain)
}

void UNPCBrain::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UNPCManager* Manager = World->GetSubsystem<UNPCManager>())
		{
			Manager->RegisterBrain(this);
		}
		// Sistem #16 — ayni beyin IKI ayri subsystem'e kaydedilir: UNPCManager kare-bolumlemeli
		// tikler (AdvanceBrain), UTaskScheduler gorev tamamlama/basarisizlik delegate'lerine
		// abone olup is atar. Iki farkli sorumluluk (yasam-dongusu tiklemesi vs. is atama), iki
		// ayri kayit — tek bir subsystem'e her ikisini de yuklemek Tek Sorumluluk'u ihlal ederdi.
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UTaskScheduler* Scheduler = GI->GetSubsystem<UTaskScheduler>())
			{
				Scheduler->RegisterBrain(this);
			}
		}
	}
}

void UNPCBrain::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UNPCManager* Manager = World->GetSubsystem<UNPCManager>())
		{
			Manager->UnregisterBrain(this);
		}
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UTaskScheduler* Scheduler = GI->GetSubsystem<UTaskScheduler>())
			{
				Scheduler->UnregisterBrain(this);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UNPCBrain::AssignTask(const FNPCTaskData& NewTask)
{
	CurrentTask = NewTask;
	bHasTask = true;

	// GERCEKTEN yeni bir gorev — onceki gorevden kalan calisma/yuruyus sureleri gecerli
	// degil, sifirlanir (Sistem #16).
	WorkElapsedTime = 0.0f;
	WalkingElapsedTime = 0.0f;

	// Inceleme bulgusu: zaten Walking durumundaysak EvaluateState bunu bir durum
	// DEGISIKLIGI olarak GORMEZ (Walking->Walking no-op) — OnStateEntered hic tetiklenmez,
	// AIController ESKI hedefe gitmeye sessizce devam eder. Yeni hedefi burada ELLE veriyoruz.
	if (CurrentState == ENPCState::Walking)
	{
		if (AAIController* Controller = GetAIController())
		{
			Controller->MoveToLocation(CurrentTask.TargetLocation);
		}
	}
}

void UNPCBrain::ClearTask()
{
	bHasTask = false;
	CurrentTask = FNPCTaskData();
	WorkElapsedTime = 0.0f;
	WalkingElapsedTime = 0.0f;

	// Inceleme bulgusu: gorev burada temizlendikten sonra CurrentState/AIController bir
	// SONRAKI AdvanceBrain cagrisina kadar (kare-bolumleme yuzunden bir tam UNPCManager
	// dongusu kadar) senkron KALMAZDI — Walking'de basarisiz olan bir gorev sonrasi piyon
	// artik-gecersiz hedefe dogru gorunur sekilde yurumeye devam ederdi. EvaluateState'i
	// HEMEN yeniden cagirmak (bHasTask artik false) durumu ve AIController'i (OnStateExited
	// uzerinden) aninda senkronize eder. Dogrudan disaridan ClearTask() cagrilirsa (ornegin
	// Blueprint) da ayni koruma gecerli olur.
	EvaluateState();
}

void UNPCBrain::CompleteCurrentTask()
{
	const FName TaskID = CurrentTask.TaskID;
	ClearTask();
	OnTaskCompleted.Broadcast(this, TaskID);
}

void UNPCBrain::FailCurrentTask()
{
	const FName TaskID = CurrentTask.TaskID;
	ClearTask();
	OnTaskFailed.Broadcast(this, TaskID);
}

void UNPCBrain::AdvanceBrain(double CurrentGameTime)
{
	const float DeltaTime = FMath::Max(0.0f, static_cast<float>(CurrentGameTime - LastBrainUpdateTime));
	LastBrainUpdateTime = CurrentGameTime;

	EvaluateState();

	const UNPCBrainSettings* Settings = GetDefault<UNPCBrainSettings>();
	const bool bIsWorking = (CurrentState == ENPCState::Working);
	const bool bIsSleeping = (CurrentState == ENPCState::Sleeping);

	const float FatigueDelta = SurvivalNPC::ComputeFatigueDelta(
		bIsWorking, bIsSleeping, Settings->WorkFatigueRate, Settings->SleepRecoveryRate, DeltaTime);
	Fatigue = FMath::Clamp(Fatigue + FatigueDelta, 0.0f, Settings->MaxFatigue);

	const float FatigueRatio = Settings->MaxFatigue > 0.0f ? Fatigue / Settings->MaxFatigue : 0.0f;
	const float MoraleDelta = SurvivalNPC::ComputeMoraleDelta(
		FatigueRatio, Settings->MoraleRecoveryRate, Settings->MoraleDecayRate, Settings->OverworkFatigueRatio, DeltaTime);
	Morale = FMath::Clamp(Morale + MoraleDelta, 0.0f, 1.0f);

	// Sistem #16 — gorev tamamlama/basarisizlik. Kesintide (Hurt/Sleeping) bu sureler
	// ARTMAZ (yalnizca ilgili durumdayken birikir) ama SIFIRLANMAZ da — is kaldigi yerden
	// devam eder (bkz. NPCBrain.h yorumu).
	if (bIsWorking)
	{
		WorkElapsedTime += DeltaTime;
		if (CurrentTask.WorkDuration > 0.0f && WorkElapsedTime >= CurrentTask.WorkDuration)
		{
			CompleteCurrentTask();
		}
	}
	else if (CurrentState == ENPCState::Walking)
	{
		WalkingElapsedTime += DeltaTime;
		if (WalkingElapsedTime >= Settings->MaxWalkingDuration)
		{
			FailCurrentTask();
		}
	}
}

void UNPCBrain::EvaluateState()
{
	const ACharacter* NPC = Cast<ACharacter>(GetOwner());
	const UHealthComponent* Health = NPC ? NPC->FindComponentByClass<UHealthComponent>() : nullptr;
	const UNPCBrainSettings* Settings = GetDefault<UNPCBrainSettings>();

	const bool bIsHurt = Health ? SurvivalNPC::IsHurt(Health->GetCurrentHealth(), Health->GetMaxHealth(), Settings->HurtThresholdFraction) : false;

	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	const bool bWantsToSleep = Clock ? SurvivalNPC::WantsToSleep(Clock->GetHourOfDay(), Settings->SleepStartHour, Settings->SleepEndHour) : false;

	const bool bIsAtTaskLocation = (bHasTask && NPC)
		? SurvivalNPC::IsAtLocation(NPC->GetActorLocation(), CurrentTask.TargetLocation, Settings->TaskAcceptanceRadius)
		: false;

	// Eating henuz hicbir zaman tetiklenmez (aclik/yemek sistemi 28 sistemde YOK, bkz. NPCState.h).
	const ENPCState NewState = SurvivalNPC::DetermineNPCState(bIsHurt, bWantsToSleep, false, bHasTask, bIsAtTaskLocation);

	if (NewState != CurrentState)
	{
		const ENPCState OldState = CurrentState;
		CurrentState = NewState;
		OnStateExited(OldState);
		OnStateEntered(NewState);
		OnStateChanged.Broadcast(OldState, NewState);
	}
}

AAIController* UNPCBrain::GetAIController() const
{
	const ACharacter* NPC = Cast<ACharacter>(GetOwner());
	return NPC ? Cast<AAIController>(NPC->GetController()) : nullptr;
}

void UNPCBrain::OnStateEntered(ENPCState NewState)
{
	if (NewState != ENPCState::Walking || !bHasTask)
	{
		return;
	}

	if (AAIController* Controller = GetAIController())
	{
		// Motorun kendi NAVMESH + hareket-takip sistemi kullanilir — ozel bir yol bulma/graf
		// sistemi YAZILMADI (bkz. NPCBrain.h ADR yorumu). Test haritasinda bir
		// NavMeshBoundsVolume YOKSA bu sessizce basarisiz olur (log'da gorulur).
		Controller->MoveToLocation(CurrentTask.TargetLocation);
	}
}

void UNPCBrain::OnStateExited(ENPCState OldState)
{
	// Inceleme bulgusu: Walking'den (varmadan) baska bir duruma gecerken (gorev iptali,
	// yaralanma, uyku saati) AIController'a HICBIR ZAMAN durdurma sinyali verilmiyordu —
	// piyon mantiksal durum degismis olsa bile eski hedefe dogru fiziksel olarak kaymaya
	// devam ederdi (PIE'de gozlemlenebilir bir desenkronizasyon).
	if (OldState != ENPCState::Walking)
	{
		return;
	}
	if (AAIController* Controller = GetAIController())
	{
		Controller->StopMovement();
	}
}

void UNPCBrain::RestoreStateForLoad(ENPCState InState, float InFatigue, float InMorale,
	bool bInHasTask, const FNPCTaskData& InTask, float InWorkElapsed, float InWalkElapsed)
{
	CurrentState = InState;
	Fatigue = InFatigue;
	Morale = InMorale;
	bHasTask = bInHasTask;
	CurrentTask = InTask;
	WorkElapsedTime = InWorkElapsed;
	WalkingElapsedTime = InWalkElapsed;

	// AssignTask ile ayni kalip: durum gecisi olmadan Walking'e geri yuklenirse
	// OnStateEntered tetiklenmez — hareketi burada ELLE baslatmaliyiz.
	if (CurrentState == ENPCState::Walking && bHasTask)
	{
		if (AAIController* Controller = GetAIController())
		{
			Controller->MoveToLocation(CurrentTask.TargetLocation);
		}
	}
}
