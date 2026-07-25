#include "NPC/NPCManager.h"
#include "NPC/NPCBrain.h"
#include "Production/ProductionMath.h"
#include "World/WorldPartitionHelper.h"
#include "SurvivalGame.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UNPCManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvival, Log, TEXT("NPCManager hazir (kare basina en fazla %d/%d dilim)"), 1, FramesPerCycle);
}

bool UNPCManager::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UNPCManager::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UNPCManager::RegisterBrain(UNPCBrain* Brain)
{
	if (!Brain || Brains.Contains(Brain))
	{
		return;
	}
	Brain->ResetBrainClock(GameTime);
	Brains.Add(Brain);
	UE_LOG(LogSurvival, Log, TEXT("NPC beyni kaydedildi (toplam %d)"), Brains.Num());
}

void UNPCManager::UnregisterBrain(UNPCBrain* Brain)
{
	if (Brains.Remove(Brain) > 0)
	{
		UE_LOG(LogSurvival, Log, TEXT("NPC beyni kaydi silindi (toplam %d)"), Brains.Num());
	}
}

void UNPCManager::Tick(float DeltaTime)
{
	GameTime += DeltaTime;

	// Sistem #18: bosaltilmis bir dunya-bolumlendirme hucresindeki NPC'ler ILERLETILMEZ —
	// UProductionManager::Tick ile ayni gerekce (bkz. o sinifin yorumu): gercek gecen sure
	// hicbir zaman kaybolmaz, yalnizca hucre bosken erteliniyor.
	const UWorldPartitionHelper* Partition = GetWorld() ? GetWorld()->GetSubsystem<UWorldPartitionHelper>() : nullptr;

	const int32 BatchSize = SurvivalProduction::ComputeBatchSize(Brains.Num(), FramesPerCycle);
	for (int32 i = 0; i < BatchSize; ++i)
	{
		if (Brains.Num() == 0)
		{
			break;
		}
		PartitionIndex %= Brains.Num();
		if (UNPCBrain* Brain = Brains[PartitionIndex])
		{
			const AActor* Owner = Brain->GetOwner();
			if (!Partition || !Owner || Partition->IsPositionLoaded(Owner->GetActorLocation()))
			{
				Brain->AdvanceBrain(GameTime);
			}
		}
		++PartitionIndex;
	}
}
