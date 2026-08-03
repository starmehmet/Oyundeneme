#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryMath.h"
#include "Items/ItemDatabase.h"
#include "Player/PlayerCharacter.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	Slots.SetNum(FMath::Max(1, SlotCount));
}

int32 UInventoryComponent::AddItem(FName ItemID, int32 RequestedCount, float SourceDurability)
{
	if (RequestedCount <= 0 || ItemID.IsNone())
	{
		return 0;
	}

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UItemDatabase* DB = GI ? GI->GetSubsystem<UItemDatabase>() : nullptr;
	FItemDefinition Def;
	if (!DB || !DB->FindItem(ItemID, Def))
	{
		UE_LOG(LogSurvival, Warning, TEXT("InventoryComponent::AddItem: bilinmeyen ItemID '%s'"), *ItemID.ToString());
		return 0;
	}

	int32 Remaining = SurvivalInventory::ComputeMaxAffordableCount(CurrentWeight, MaxWeight, Def.Weight, RequestedCount);
	int32 TotalAccepted = 0;

	// Once mevcut yiginlara sigdir (yigilabilir oglerde)
	if (Def.IsStackable())
	{
		int32 SlotIdx;
		while (Remaining > 0 && (SlotIdx = FindSlotWithRoomFor(ItemID, Def.MaxStackSize)) != INDEX_NONE)
		{
			const int32 Accepted = SurvivalInventory::ComputeAcceptedIntoStack(Slots[SlotIdx].Count, Remaining, Def.MaxStackSize);
			if (Accepted <= 0)
			{
				break; // guvenlik: ilerleme yoksa sonsuz donguye girme
			}
			Slots[SlotIdx].Count += Accepted;
			Remaining -= Accepted;
			TotalAccepted += Accepted;
			BroadcastSlot(SlotIdx);
		}
	}

	// Sonra bos slotlara yeni yigin(lar) ac
	int32 EmptyIdx;
	while (Remaining > 0 && (EmptyIdx = FindFirstEmptySlot()) != INDEX_NONE)
	{
		const int32 Accepted = SurvivalInventory::ComputeAcceptedIntoStack(0, Remaining, Def.MaxStackSize);
		if (Accepted <= 0)
		{
			break;
		}
		Slots[EmptyIdx].ItemID = ItemID;
		Slots[EmptyIdx].Count = Accepted;
		Slots[EmptyIdx].Durability = SourceDurability;
		Remaining -= Accepted;
		TotalAccepted += Accepted;
		BroadcastSlot(EmptyIdx);
	}

	if (TotalAccepted > 0)
	{
		CurrentWeight += TotalAccepted * Def.Weight;
	}
	return TotalAccepted;
}

int32 UInventoryComponent::RemoveItem(FName ItemID, int32 RequestedCount)
{
	if (RequestedCount <= 0 || ItemID.IsNone())
	{
		return 0;
	}

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UItemDatabase* DB = GI ? GI->GetSubsystem<UItemDatabase>() : nullptr;
	FItemDefinition Def;
	const bool bKnownWeight = DB && DB->FindItem(ItemID, Def);
	if (!bKnownWeight)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("InventoryComponent::RemoveItem: '%s' veritabaninda yok — agirlik dusulemiyor"), *ItemID.ToString());
	}

	int32 Remaining = RequestedCount;
	int32 TotalRemoved = 0;

	for (int32 i = 0; i < Slots.Num() && Remaining > 0; ++i)
	{
		if (Slots[i].ItemID != ItemID || Slots[i].IsEmpty())
		{
			continue;
		}
		const int32 Taken = FMath::Min(Slots[i].Count, Remaining);
		Slots[i].Count -= Taken;
		Remaining -= Taken;
		TotalRemoved += Taken;
		if (Slots[i].Count <= 0)
		{
			Slots[i] = FInventorySlot();
		}
		BroadcastSlot(i);
	}

	if (TotalRemoved > 0 && bKnownWeight)
	{
		CurrentWeight = FMath::Max(0.0f, CurrentWeight - TotalRemoved * Def.Weight);
	}
	return TotalRemoved;
}

int32 UInventoryComponent::TransferItemTo(UInventoryComponent* Destination, FName ItemID, int32 Count)
{
	if (!Destination || Destination == this || Count <= 0)
	{
		return 0;
	}

	// Slot slot ilerle (tek toplu miktar yerine): her slotun KENDI durability'sini
	// hedefe tasir (inceleme bulgusu — eskiden Durability transferde hep 1.0f'a
	// sifirlaniyordu, asinmis bir alet aktarilinca sessizce "yeni" oluyordu).
	// Guvenli sira: her slot icin ONCE hedefin kabul ettigi miktari olc, SONRA
	// kaynaktan o kadarini cikar — hedef reddederse (dolu/agir) kaynak stogu
	// asla hayali kaybolmaz.
	int32 Remaining = Count;
	int32 TotalTransferred = 0;

	for (int32 i = 0; i < Slots.Num() && Remaining > 0; ++i)
	{
		if (Slots[i].ItemID != ItemID || Slots[i].IsEmpty())
		{
			continue;
		}
		const int32 ToAttempt = FMath::Min(Slots[i].Count, Remaining);
		const int32 Accepted = Destination->AddItem(ItemID, ToAttempt, Slots[i].Durability);
		if (Accepted <= 0)
		{
			continue; // bu slottan aktarilamadi (hedef dolu/agir) — sonraki slotu dene
		}
		RemoveItem(ItemID, Accepted);
		Remaining -= Accepted;
		TotalTransferred += Accepted;
	}
	return TotalTransferred;
}

bool UInventoryComponent::HasRoomFor(FName ItemID, int32 Count) const
{
	if (Count <= 0 || ItemID.IsNone())
	{
		return true;
	}

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UItemDatabase* DB = GI ? GI->GetSubsystem<UItemDatabase>() : nullptr;
	FItemDefinition Def;
	if (!DB || !DB->FindItem(ItemID, Def))
	{
		return false; // bilinmeyen oge — guvenlik icin "sigmaz"
	}

	return !SurvivalInventory::WouldExceedWeightLimit(CurrentWeight, Count * Def.Weight, MaxWeight);
}

bool UInventoryComponent::HasRoomForBatch(const TMap<FName, int32>& Items) const
{
	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UItemDatabase* DB = GI ? GI->GetSubsystem<UItemDatabase>() : nullptr;
	if (!DB)
	{
		return false;
	}

	float TotalAddedWeight = 0.0f;
	for (const TPair<FName, int32>& Pair : Items)
	{
		if (Pair.Value <= 0 || Pair.Key.IsNone())
		{
			continue;
		}
		FItemDefinition Def;
		if (!DB->FindItem(Pair.Key, Def))
		{
			return false; // bilinmeyen oge — guvenlik icin "sigmaz"
		}
		TotalAddedWeight += Pair.Value * Def.Weight;
	}

	return !SurvivalInventory::WouldExceedWeightLimit(CurrentWeight, TotalAddedWeight, MaxWeight);
}

void UInventoryComponent::RestoreSlots(const TArray<FInventorySlot>& SavedSlots)
{
	Slots = SavedSlots;
	Slots.SetNum(FMath::Max(1, SlotCount)); // slot sayisi kaydedildikten sonra degistiyse (nadir) fazlasi atilir/eksigi boslanir

	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UItemDatabase* DB = GI ? GI->GetSubsystem<UItemDatabase>() : nullptr;

	CurrentWeight = 0.0f;
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		// Inceleme bulgusu: bos slotlar da BroadcastSlot ALMALI (RemoveItem'daki desenle
		// AYNI, bkz. o fonksiyon) — aksi halde bir onceki oturumda dolu olan ama kaydedilen
		// durumda bos olan bir slot icin UI/dinleyiciler HICBIR ZAMAN haberdar edilmez,
		// eski (artik yanlis) icerigi gostermeye devam eder.
		if (Slots[i].IsEmpty())
		{
			BroadcastSlot(i);
			continue;
		}
		FItemDefinition Def;
		if (DB && DB->FindItem(Slots[i].ItemID, Def))
		{
			CurrentWeight += Slots[i].Count * Def.Weight;
		}
		else
		{
			UE_LOG(LogSurvival, Warning, TEXT("RestoreSlots: bilinmeyen ItemID '%s' — agirlik hesaba katilmadi"), *Slots[i].ItemID.ToString());
		}
		BroadcastSlot(i);
	}
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Total = 0;
	for (const FInventorySlot& Slot : Slots)
	{
		if (Slot.ItemID == ItemID)
		{
			Total += Slot.Count;
		}
	}
	return Total;
}

int32 UInventoryComponent::FindSlotWithRoomFor(FName ItemID, int32 MaxStackSize) const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].ItemID == ItemID && Slots[i].Count < MaxStackSize)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UInventoryComponent::BroadcastSlot(int32 SlotIndex)
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		OnInventoryChanged.Broadcast(SlotIndex, Slots[SlotIndex]);
	}
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (craft_start/production_set_recipe ile ayni desen) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdGiveItem(
		TEXT("give_item"),
		TEXT("Oyuncunun envanterine oge ekler (dev-cheat): give_item <ItemID> [Adet]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || Args.Num() < 1)
				{
					UE_LOG(LogSurvival, Warning, TEXT("Kullanim: give_item <ItemID> [Adet]"));
					return;
				}

				const APlayerController* PC = World->GetFirstPlayerController();
				APlayerCharacter* Player = PC ? Cast<APlayerCharacter>(PC->GetPawn()) : nullptr;
				UInventoryComponent* Inventory = Player ? Player->GetInventoryComponent() : nullptr;
				if (!Inventory)
				{
					UE_LOG(LogSurvival, Warning, TEXT("give_item: oyuncu/envanter bulunamadi"));
					return;
				}

				const FName ItemID(*Args[0]);
				const int32 RequestedCount = Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 1;
				const int32 Added = Inventory->AddItem(ItemID, RequestedCount);
				UE_LOG(LogSurvival, Log, TEXT("give_item: '%s' x%d istendi, %d eklendi"),
					*Args[0], RequestedCount, Added);
			}));
}
