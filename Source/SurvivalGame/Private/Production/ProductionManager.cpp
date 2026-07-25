#include "Production/ProductionManager.h"
#include "Production/ProductionMath.h"
#include "Production/ProductionMachine.h"
#include "World/WorldPartitionHelper.h"
#include "Inventory/InventoryComponent.h"
#include "Logistics/StorageNode.h"
#include "SurvivalGame.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

void UProductionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalProduction, Log, TEXT("ProductionManager hazir (kare basina en fazla %d/%d dilim)"),
		1, FramesPerCycle);
}

bool UProductionManager::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UProductionManager::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UProductionManager::RegisterMachine(AProductionMachine* Machine)
{
	if (!Machine || Machines.Contains(Machine))
	{
		return;
	}
	Machine->ResetProductionClock(GameTime);
	Machines.Add(Machine);
	UE_LOG(LogSurvivalProduction, Log, TEXT("Makine kaydedildi (toplam %d)"), Machines.Num());
}

void UProductionManager::UnregisterMachine(AProductionMachine* Machine)
{
	if (Machines.Remove(Machine) > 0)
	{
		// PartitionIndex, kalan liste boyutuna gore Tick'te zaten modulo ile sarilir — burada
		// ekstra duzeltme gerekmez.
		UE_LOG(LogSurvivalProduction, Log, TEXT("Makine kaydi silindi (toplam %d)"), Machines.Num());
	}
}

void UProductionManager::Tick(float DeltaTime)
{
	GameTime += DeltaTime;

	// Sistem #18: bosaltilmis bir dunya-bolumlendirme hucresindeki makineler ILERLETILMEZ —
	// YENI bir "duraklat" semantigi DEGIL: her makine kendi LastProductionUpdateTime'ini
	// tuttugundan (bkz. AdvanceProduction yorumu), hucre yeniden yuklenince bir sonraki
	// cagri GERCEK gecen sureyi otomatik telafi eder, hicbir sey kaybolmaz.
	const UWorldPartitionHelper* Partition = GetWorld() ? GetWorld()->GetSubsystem<UWorldPartitionHelper>() : nullptr;

	const int32 BatchSize = SurvivalProduction::ComputeBatchSize(Machines.Num(), FramesPerCycle);
	for (int32 i = 0; i < BatchSize; ++i)
	{
		if (Machines.Num() == 0)
		{
			break;
		}
		PartitionIndex %= Machines.Num();
		if (AProductionMachine* Machine = Machines[PartitionIndex])
		{
			if (!Partition || Partition->IsPositionLoaded(Machine->GetActorLocation()))
			{
				Machine->AdvanceProduction(GameTime);
			}
		}
		++PartitionIndex;
	}
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi, craft_start ile ayni desen) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdProductionSetRecipe(
		TEXT("production_set_recipe"),
		TEXT("Sahnedeki ilk uretim makinesine tarif ata: production_set_recipe <RecipeID>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || Args.Num() < 1)
				{
					UE_LOG(LogSurvivalProduction, Warning, TEXT("Kullanim: production_set_recipe <RecipeID>"));
					return;
				}

				AProductionMachine* FoundMachine = nullptr;
				for (TActorIterator<AProductionMachine> It(World); It; ++It)
				{
					FoundMachine = *It;
					break;
				}
				if (!FoundMachine)
				{
					UE_LOG(LogSurvivalProduction, Warning, TEXT("production_set_recipe: sahnede AProductionMachine bulunamadi"));
					return;
				}

				const bool bSet = FoundMachine->SetActiveRecipe(FName(*Args[0]));
				UE_LOG(LogSurvivalProduction, Log, TEXT("production_set_recipe '%s': %s"),
					*Args[0], bSet ? TEXT("basarili") : TEXT("basarisiz (bilinmeyen tarif)"));
			}));

	FString DumpSlots(const UInventoryComponent* Inventory)
	{
		if (!Inventory)
		{
			return TEXT("(yok)");
		}
		FString Result;
		for (const FInventorySlot& Slot : Inventory->GetSlots())
		{
			if (Slot.IsEmpty())
			{
				continue;
			}
			Result += FString::Printf(TEXT("%s x%d "), *Slot.ItemID.ToString(), Slot.Count);
		}
		return Result.IsEmpty() ? TEXT("(bos)") : Result;
	}

	// Faz 1 entegrasyon borcu: makinenin durumunu VE lojistik-agi kayit durumunu (InputLogisticsNode/
	// OutputLogisticsNode) tek yerde gosterir — PIE'de ucdan uca (insaat->lojistik->uretim) akisin
	// dogrulanmasi icin.
	FAutoConsoleCommandWithWorldAndArgs GCmdProductionDump(
		TEXT("production_dump"),
		TEXT("Sahnedeki ilk uretim makinesinin durumunu (arabellekler + lojistik kaydi dahil) logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}

				AProductionMachine* FoundMachine = nullptr;
				for (TActorIterator<AProductionMachine> It(World); It; ++It)
				{
					FoundMachine = *It;
					break;
				}
				if (!FoundMachine)
				{
					UE_LOG(LogSurvivalProduction, Warning, TEXT("production_dump: sahnede AProductionMachine bulunamadi"));
					return;
				}

				const AStorageNode* InNode = FoundMachine->GetInputLogisticsNode();
				const AStorageNode* OutNode = FoundMachine->GetOutputLogisticsNode();

				UE_LOG(LogSurvivalProduction, Log,
					TEXT("Makine: durum=%d ilerleme=%.2f enerji=%.1f girdi=[%s] cikti=[%s] lojistik-giris-dugumu=%s lojistik-cikis-dugumu=%s"),
					static_cast<int32>(FoundMachine->GetProductionState()),
					FoundMachine->GetProgress(),
					FoundMachine->GetCurrentEnergy(),
					*DumpSlots(FoundMachine->GetInputBuffer()),
					*DumpSlots(FoundMachine->GetOutputBuffer()),
					InNode ? TEXT("kayitli") : TEXT("YOK"),
					OutNode ? TEXT("kayitli") : TEXT("YOK"));
			}));
}
