#include "NPC/TaskScheduler.h"
#include "NPC/TaskSchedulerMath.h"
#include "NPC/TaskSchedulerSettings.h"
#include "NPC/NPCBrain.h"
#include "NPC/NPCCharacter.h"
#include "Production/ProductionMachine.h"
#include "Time/TimeKeeper.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"

void UTaskScheduler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvival, Log, TEXT("TaskScheduler hazir"));
}

double UTaskScheduler::GetCurrentGameTime() const
{
	const UGameInstance* GI = GetGameInstance();
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	return Clock ? Clock->GetTotalGameSeconds() : 0.0;
}

void UTaskScheduler::PruneStaleBrains()
{
	for (auto It = RegisteredBrains.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void UTaskScheduler::EnqueueTask(const FTaskDefinition& TaskDef)
{
	PendingTasks.Add(TaskDef);
	UE_LOG(LogSurvival, Log, TEXT("Gorev kuyruga eklendi: '%s' (oncelik=%.1f, beceri>=%d)"),
		*TaskDef.TaskID.ToString(), TaskDef.Priority, TaskDef.RequiredSkillLevel);
	TryAssignToAnyIdleBrain();
}

void UTaskScheduler::RegisterBrain(UNPCBrain* Brain)
{
	if (!Brain || RegisteredBrains.Contains(Brain))
	{
		return;
	}
	RegisteredBrains.Add(Brain);
	Brain->OnTaskCompleted.AddDynamic(this, &UTaskScheduler::HandleTaskCompleted);
	Brain->OnTaskFailed.AddDynamic(this, &UTaskScheduler::HandleTaskFailed);
	TryAssignBestTaskTo(Brain);
}

void UTaskScheduler::UnregisterBrain(UNPCBrain* Brain)
{
	if (Brain)
	{
		Brain->OnTaskCompleted.RemoveDynamic(this, &UTaskScheduler::HandleTaskCompleted);
		Brain->OnTaskFailed.RemoveDynamic(this, &UTaskScheduler::HandleTaskFailed);
	}

	// Inceleme bulgusu: bu beyin (NPC yok edildigi icin) aktif bir gorev tasiyorsa, o gorev
	// HandleTaskFailed'daki ile AYNI sekilde (backoff'la) yeniden kuyruga alinmali —
	// aksi halde NPC oldugunde/yok edildiginde is sessizce kaybolurdu, hicbir baska NPC'ye
	// asla teklif edilmezdi (bkz. HandleTaskFailed, ayni desen).
	if (const FTaskDefinition* Def = ActiveAssignments.Find(Brain))
	{
		FTaskDefinition Requeued = *Def;
		const UTaskSchedulerSettings* Settings = GetDefault<UTaskSchedulerSettings>();
		Requeued.AvailableAfterGameTime = SurvivalTaskScheduler::ComputeBackoffAvailableTime(GetCurrentGameTime(), Settings->TaskFailureBackoffDuration);
		PendingTasks.Add(Requeued);
		UE_LOG(LogSurvival, Log, TEXT("UnregisterBrain: NPC yok edildi, aktif gorevi '%s' yeniden kuyruga alindi"), *Requeued.TaskID.ToString());
	}

	RegisteredBrains.RemoveSingleSwap(Brain);
	ActiveAssignments.Remove(Brain);
}

void UTaskScheduler::TryAssignBestTaskTo(UNPCBrain* Brain)
{
	if (!Brain || Brain->HasTask())
	{
		return;
	}

	const double CurrentGameTime = GetCurrentGameTime();
	const int32 BestIndex = SurvivalTaskScheduler::FindBestEligibleTaskIndex(PendingTasks, Brain->GetSkillLevel(), CurrentGameTime);
	if (BestIndex == INDEX_NONE)
	{
		return;
	}

	FTaskDefinition TaskDef = PendingTasks[BestIndex];
	PendingTasks.RemoveAt(BestIndex);

	FNPCTaskData BrainTask;
	BrainTask.TaskID = TaskDef.TaskID;
	BrainTask.TargetLocation = TaskDef.TargetMachine.IsValid() ? TaskDef.TargetMachine->GetActorLocation() : TaskDef.TargetLocation;
	BrainTask.Priority = TaskDef.Priority;
	BrainTask.WorkDuration = TaskDef.WorkDuration;
	Brain->AssignTask(BrainTask);

	ActiveAssignments.Add(Brain, TaskDef);
	UE_LOG(LogSurvival, Log, TEXT("Gorev atandi: '%s'"), *TaskDef.TaskID.ToString());
}

void UTaskScheduler::TryAssignToAnyIdleBrain()
{
	PruneStaleBrains();
	for (const TWeakObjectPtr<UNPCBrain>& WeakBrain : RegisteredBrains)
	{
		if (UNPCBrain* Brain = WeakBrain.Get())
		{
			if (!Brain->HasTask())
			{
				TryAssignBestTaskTo(Brain);
			}
		}
	}
}

void UTaskScheduler::HandleTaskCompleted(UNPCBrain* Brain, FName TaskID)
{
	ActiveAssignments.Remove(Brain);
	UE_LOG(LogSurvival, Log, TEXT("Gorev tamamlandi: '%s'"), *TaskID.ToString());
	TryAssignBestTaskTo(Brain);
}

void UTaskScheduler::HandleTaskFailed(UNPCBrain* Brain, FName TaskID)
{
	const UTaskSchedulerSettings* Settings = GetDefault<UTaskSchedulerSettings>();
	if (const FTaskDefinition* Def = ActiveAssignments.Find(Brain))
	{
		FTaskDefinition Requeued = *Def;
		Requeued.AvailableAfterGameTime = SurvivalTaskScheduler::ComputeBackoffAvailableTime(GetCurrentGameTime(), Settings->TaskFailureBackoffDuration);
		PendingTasks.Add(Requeued);
		UE_LOG(LogSurvival, Warning, TEXT("Gorev basarisiz: '%s' — %.0fsn backoff sonrasi yeniden kuyrukta"),
			*TaskID.ToString(), Settings->TaskFailureBackoffDuration);
	}
	ActiveAssignments.Remove(Brain);
	TryAssignBestTaskTo(Brain);
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdTaskEnqueue(
		TEXT("task_enqueue"),
		TEXT("Oyuncunun konumunu hedefleyen bir gorev kuyruga ekler: task_enqueue [Oncelik] [GerekliBeceri] [CalismaSuresi]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UTaskScheduler* Scheduler = GI ? GI->GetSubsystem<UTaskScheduler>() : nullptr;
				const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
				const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
				if (!Scheduler || !PlayerPawn)
				{
					UE_LOG(LogSurvival, Warning, TEXT("task_enqueue: TaskScheduler veya oyuncu piyonu bulunamadi"));
					return;
				}

				FTaskDefinition TaskDef;
				TaskDef.TaskID = FName(*FString::Printf(TEXT("dev_task_%d"), FMath::RandRange(1000, 9999)));
				TaskDef.TaskName = FText::FromString(TEXT("Dev Gorevi"));
				TaskDef.TargetLocation = PlayerPawn->GetActorLocation();
				TaskDef.Priority = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 1.0f;
				TaskDef.RequiredSkillLevel = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 0;
				TaskDef.WorkDuration = Args.Num() > 2 ? FCString::Atof(*Args[2]) : 5.0f;

				Scheduler->EnqueueTask(TaskDef);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdTaskDump(
		TEXT("task_dump"),
		TEXT("Gorev planlayicinin durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const UTaskScheduler* Scheduler = GI ? GI->GetSubsystem<UTaskScheduler>() : nullptr;
				if (!Scheduler)
				{
					return;
				}
				UE_LOG(LogSurvival, Log, TEXT("GorevPlanlayici: bekleyen=%d aktif-atama=%d"),
					Scheduler->GetPendingTaskCount(), Scheduler->GetActiveAssignmentCount());
			}));
}
