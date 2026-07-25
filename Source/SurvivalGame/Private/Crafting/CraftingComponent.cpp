#include "Crafting/CraftingComponent.h"
#include "Crafting/CraftingMath.h"
#include "Crafting/RecipeDatabase.h"
#include "Inventory/InventoryComponent.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = true; // ilerlemeyi takip etmek icin tick gerekli
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerInventory = GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!OwnerInventory)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("CraftingComponent: sahibinde UInventoryComponent yok — zanaat calismayacak"));
	}
}

bool UCraftingComponent::CanCraftRecipe(FName RecipeID) const
{
	if (!OwnerInventory || RecipeID.IsNone())
	{
		return false;
	}
	if (!SurvivalCrafting::CanEnqueue(ActiveJobs.Num(), MaxQueueSize))
	{
		return false;
	}

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const URecipeDatabase* DB = GI ? GI->GetSubsystem<URecipeDatabase>() : nullptr;
	FRecipeDefinition Recipe;
	if (!DB || !DB->FindRecipe(RecipeID, Recipe))
	{
		return false;
	}

	// Toplu (aggregated) miktarlar kullanilir — ayni ItemID iki ayri satirda
	// gecerse (or. {Wood,5}+{Wood,3}) satir-satir bagimsiz kontrol gercek
	// maliyeti oldugundan dusuk gosterirdi (inceleme bulgusu).
	for (const TPair<FName, int32>& Req : Recipe.GetAggregatedIngredients())
	{
		if (!OwnerInventory->HasItem(Req.Key, Req.Value))
		{
			return false;
		}
	}
	return true;
}

bool UCraftingComponent::StartCrafting(FName RecipeID)
{
	if (!CanCraftRecipe(RecipeID))
	{
		return false;
	}

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const URecipeDatabase* DB = GI ? GI->GetSubsystem<URecipeDatabase>() : nullptr;
	FRecipeDefinition Recipe;
	if (!DB || !DB->FindRecipe(RecipeID, Recipe))
	{
		return false; // CanCraftRecipe zaten dogruladi — savunmaci kontrol
	}

	// Atomik tuketim: herhangi bir kalem eksik cikarsa (RemoveItem'in dondugu
	// miktar istenenden azsa) o ana kadar tuketilen her sey GERI EKLENIR ve
	// StartCrafting basarisiz doner — kismi tuketim asla kalici olmaz
	// (inceleme bulgusu: eskiden RemoveItem'in donusu hic kontrol edilmiyordu).
	TArray<TPair<FName, int32>> Consumed;
	for (const TPair<FName, int32>& Req : Recipe.GetAggregatedIngredients())
	{
		const int32 Removed = OwnerInventory->RemoveItem(Req.Key, Req.Value);
		Consumed.Add(TPair<FName, int32>(Req.Key, Removed));
		if (Removed < Req.Value)
		{
			for (const TPair<FName, int32>& C : Consumed)
			{
				if (C.Value > 0)
				{
					OwnerInventory->AddItem(C.Key, C.Value);
				}
			}
			UE_LOG(LogSurvival, Warning,
				TEXT("StartCrafting: '%s' icin '%s' yetersiz (istenen %d, cikan %d) — islem geri alindi"),
				*RecipeID.ToString(), *Req.Key.ToString(), Req.Value, Removed);
			return false;
		}
	}

	FCraftingJob NewJob;
	NewJob.RecipeID = RecipeID;
	NewJob.ElapsedTime = 0.0f;
	ActiveJobs.Add(NewJob);

	UE_LOG(LogSurvival, Log, TEXT("Zanaat basladi: '%s' (kuyrukta %d is)"), *RecipeID.ToString(), ActiveJobs.Num());
	return true;
}

bool UCraftingComponent::CancelCrafting(int32 JobIndex)
{
	if (!ActiveJobs.IsValidIndex(JobIndex) || !OwnerInventory)
	{
		return false;
	}

	const FName RecipeID = ActiveJobs[JobIndex].RecipeID;
	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const URecipeDatabase* DB = GI ? GI->GetSubsystem<URecipeDatabase>() : nullptr;
	FRecipeDefinition Recipe;
	if (DB && DB->FindRecipe(RecipeID, Recipe))
	{
		// AddItem'in donusu (gercekte kabul edilen miktar) KONTROL EDILIR —
		// eskiden sessizce yok sayiliyordu; envanter dolu/agirsa iade eksik
		// kalabilir, bu artik en azindan LOGLANIR (inceleme bulgusu — Sistem #4'te
		// TransferItemTo'da duzeltilen ayni sinif hata CancelCrafting'e sizmisti).
		for (const TPair<FName, int32>& Req : Recipe.GetAggregatedIngredients())
		{
			const int32 Accepted = OwnerInventory->AddItem(Req.Key, Req.Value);
			if (Accepted < Req.Value)
			{
				UE_LOG(LogSurvival, Warning,
					TEXT("CancelCrafting: '%s' icin '%s' tam iade edilemedi (istenen %d, kabul %d) — envanter dolu/agir olabilir"),
					*RecipeID.ToString(), *Req.Key.ToString(), Req.Value, Accepted);
			}
		}
	}

	ActiveJobs.RemoveAt(JobIndex);
	return true;
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const URecipeDatabase* DB = GI ? GI->GetSubsystem<URecipeDatabase>() : nullptr;

	// Geriye dogru: CompleteJob RemoveAt yapacagi icin ileri index kaymasini onler
	for (int32 i = ActiveJobs.Num() - 1; i >= 0; --i)
	{
		ActiveJobs[i].ElapsedTime += DeltaTime;

		FRecipeDefinition Recipe;
		const bool bFound = DB && DB->FindRecipe(ActiveJobs[i].RecipeID, Recipe);
		const float CraftingTime = bFound ? Recipe.CraftingTime : 0.0f;

		OnCraftingProgress.Broadcast(i, SurvivalCrafting::ComputeCraftProgress(ActiveJobs[i].ElapsedTime, CraftingTime));

		if (SurvivalCrafting::IsCraftComplete(ActiveJobs[i].ElapsedTime, CraftingTime))
		{
			CompleteJob(i);
		}
	}
}

void UCraftingComponent::CompleteJob(int32 JobIndex)
{
	if (!ActiveJobs.IsValidIndex(JobIndex))
	{
		return;
	}

	const FName RecipeID = ActiveJobs[JobIndex].RecipeID;
	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const URecipeDatabase* DB = GI ? GI->GetSubsystem<URecipeDatabase>() : nullptr;
	FRecipeDefinition Recipe;
	bool bSuccess = false;

	if (DB && DB->FindRecipe(RecipeID, Recipe) && OwnerInventory)
	{
		for (const FRecipeOutput& Out : Recipe.Outputs)
		{
			OwnerInventory->AddItem(Out.ItemID, Out.Count);
		}
		bSuccess = true;
	}

	ActiveJobs.RemoveAt(JobIndex);
	OnCraftingCompleted.Broadcast(RecipeID, bSuccess);

	UE_LOG(LogSurvival, Log, TEXT("Zanaat tamamlandi: '%s' (basarili: %s)"),
		*RecipeID.ToString(), bSuccess ? TEXT("evet") : TEXT("hayir"));
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdCraftStart(
		TEXT("craft_start"),
		TEXT("Zanaat baslat: craft_start <RecipeID>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || Args.Num() < 1)
				{
					UE_LOG(LogSurvival, Warning, TEXT("Kullanim: craft_start <RecipeID>"));
					return;
				}
				const APlayerController* PC = World->GetFirstPlayerController();
				APawn* Pawn = PC ? PC->GetPawn() : nullptr;
				UCraftingComponent* Crafting = Pawn ? Pawn->FindComponentByClass<UCraftingComponent>() : nullptr;
				if (!Crafting)
				{
					UE_LOG(LogSurvival, Warning, TEXT("craft_start: CraftingComponent bulunamadi"));
					return;
				}
				const bool bStarted = Crafting->StartCrafting(FName(*Args[0]));
				UE_LOG(LogSurvival, Log, TEXT("craft_start '%s': %s"),
					*Args[0], bStarted ? TEXT("basladi") : TEXT("basarisiz (malzeme/kuyruk)"));
			}));
}
