#include "NPC/NPCCharacter.h"
#include "NPC/NPCBrain.h"
#include "Player/HealthComponent.h"
#include "AIController.h"
#include "SurvivalGame.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false; // UNPCManager kare-bolumlemeli tikler (bkz. UNPCBrain)

	BrainComp = CreateDefaultSubobject<UNPCBrain>(TEXT("Brain"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	ANPCCharacter* FindFirstNPC(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}
		for (TActorIterator<ANPCCharacter> It(World); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdNPCAssignTask(
		TEXT("npc_assign_task"),
		TEXT("Sahnedeki ilk NPC'ye oyuncunun su anki konumunu hedefleyen bir gorev atar: npc_assign_task"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				ANPCCharacter* NPC = FindFirstNPC(World);
				const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
				const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
				if (!NPC || !NPC->GetBrain() || !PlayerPawn)
				{
					UE_LOG(LogSurvival, Warning, TEXT("npc_assign_task: sahnede ANPCCharacter veya oyuncu piyonu bulunamadi"));
					return;
				}

				FNPCTaskData Task;
				Task.TaskID = TEXT("dev_task");
				Task.TargetLocation = PlayerPawn->GetActorLocation();
				Task.Priority = 1.0f;
				NPC->GetBrain()->AssignTask(Task);

				UE_LOG(LogSurvival, Log, TEXT("npc_assign_task: hedef=(%.0f,%.0f,%.0f)"),
					Task.TargetLocation.X, Task.TargetLocation.Y, Task.TargetLocation.Z);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdNPCDump(
		TEXT("npc_dump"),
		TEXT("Sahnedeki ilk NPC'nin durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const ANPCCharacter* NPC = FindFirstNPC(World);
				if (!NPC || !NPC->GetBrain())
				{
					UE_LOG(LogSurvival, Warning, TEXT("npc_dump: sahnede ANPCCharacter bulunamadi"));
					return;
				}

				const UNPCBrain* Brain = NPC->GetBrain();
				const UHealthComponent* Health = NPC->GetHealthComponent();

				UE_LOG(LogSurvival, Log,
					TEXT("NPC: durum=%d gorev=%s yorgunluk=%.1f moral=%.2f can=%.1f"),
					static_cast<int32>(Brain->GetCurrentState()), Brain->HasTask() ? TEXT("EVET") : TEXT("hayir"),
					Brain->GetFatigue(), Brain->GetMorale(), Health ? Health->GetCurrentHealth() : -1.0f);
			}));
}
