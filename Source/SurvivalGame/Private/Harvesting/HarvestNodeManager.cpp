#include "Harvesting/HarvestNodeManager.h"
#include "Harvesting/HarvestNode.h"
#include "Harvesting/HarvestMath.h"
#include "Production/ProductionMath.h"
#include "World/WorldPartitionHelper.h"
#include "SurvivalGame.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

void UHarvestNodeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvival, Log, TEXT("HarvestNodeManager hazir (kare basina en fazla %d/%d dilim)"), 1, FramesPerCycle);
}

bool UHarvestNodeManager::IsTickable() const
{
	if (IsTemplate() || DepletedNodes.Num() == 0)
	{
		return false;
	}
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UHarvestNodeManager::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UHarvestNodeManager::RegisterDepletedNode(AHarvestNode* Node)
{
	if (!Node || DepletedNodes.Contains(Node))
	{
		return;
	}
	DepletedNodes.Add(Node);
	UE_LOG(LogSurvival, Verbose,
		TEXT("HarvestNodeManager: dugum tukendi, yeniden-dogma kuyruguna eklendi (toplam %d)"), DepletedNodes.Num());
}

void UHarvestNodeManager::UnregisterDepletedNode(AHarvestNode* Node)
{
	if (DepletedNodes.Remove(Node) > 0)
	{
		UE_LOG(LogSurvival, Verbose, TEXT("HarvestNodeManager: dugum kuyruktan cikarildi (toplam %d)"), DepletedNodes.Num());
	}
}

void UHarvestNodeManager::Tick(float DeltaTime)
{
	GameTime += DeltaTime;

	// Sistem #18: bosaltilmis bir dunya-bolumlendirme hucresindeki dugumler ILERLETILMEZ —
	// ProductionManager/NPCManager ile ayni disiplin: mutlak DepletionGameTime damgasi
	// sayesinde hicbir ilerleme kaybolmaz, hucre yeniden yuklenince tek seferde telafi olur.
	const UWorldPartitionHelper* Partition = GetWorld() ? GetWorld()->GetSubsystem<UWorldPartitionHelper>() : nullptr;

	const int32 BatchSize = SurvivalProduction::ComputeBatchSize(DepletedNodes.Num(), FramesPerCycle);
	for (int32 i = 0; i < BatchSize; ++i)
	{
		if (DepletedNodes.Num() == 0)
		{
			break;
		}
		PartitionIndex %= DepletedNodes.Num();
		AHarvestNode* Node = DepletedNodes[PartitionIndex];
		if (!Node)
		{
			DepletedNodes.RemoveAt(PartitionIndex);
			continue; // silinen elemanin yerine kayan eleman bu turda PartitionIndex'te — atlanmaz
		}

		if (Partition && !Partition->IsPositionLoaded(Node->GetActorLocation()))
		{
			++PartitionIndex;
			continue;
		}

		if (SurvivalHarvest::IsRespawnReady(Node->GetDepletionGameTime(), GameTime, Node->GetRespawnSeconds()))
		{
			Node->Respawn();
			DepletedNodes.RemoveAt(PartitionIndex); // Respawn() DepletedNodes'a dokunmaz — kaldirma burada
			continue; // dizi kaydigi icin PartitionIndex'i arttirma
		}

		++PartitionIndex;
	}
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (ProductionManager.cpp ile ayni desen) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdHarvestDump(
		TEXT("harvest_dump"),
		TEXT("Sahnedeki tum AHarvestNode'larin durumunu (tukenme/sarj) logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}

				int32 Total = 0;
				int32 Depleted = 0;
				for (TActorIterator<AHarvestNode> It(World); It; ++It)
				{
					++Total;
					if (It->IsDepleted())
					{
						++Depleted;
					}
					UE_LOG(LogSurvival, Log, TEXT("HarvestNode '%s' (NodeID=%s): tukenmis=%s kalan-sarj=%d"),
						*It->GetName(), *It->GetNodeID().ToString(),
						It->IsDepleted() ? TEXT("evet") : TEXT("hayir"), It->GetRemainingHarvests());
				}
				UE_LOG(LogSurvival, Log, TEXT("harvest_dump: %d dugum tarandi, %d tukenmis"), Total, Depleted);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdHarvestForceRegenerate(
		TEXT("harvest_force_regenerate"),
		TEXT("Sahnedeki TUM tukenmis AHarvestNode'lari zorla yeniden dogurur (PIE test kisayolu)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}

				UHarvestNodeManager* Manager = World->GetSubsystem<UHarvestNodeManager>();

				int32 Regenerated = 0;
				for (TActorIterator<AHarvestNode> It(World); It; ++It)
				{
					if (It->IsDepleted())
					{
						It->Respawn();
						if (Manager)
						{
							Manager->UnregisterDepletedNode(*It);
						}
						++Regenerated;
					}
				}
				UE_LOG(LogSurvival, Log, TEXT("harvest_force_regenerate: %d dugum yeniden dogduruldu"), Regenerated);
			}));
}
