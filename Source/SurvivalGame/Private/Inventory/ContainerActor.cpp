#include "Inventory/ContainerActor.h"
#include "Inventory/InventoryComponent.h"
#include "Player/PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "SurvivalGame.h"

AContainerActor::AContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	// ECC_Interaction varsayilani Ignore — etkilesilebilirler ACIKCA Block ile kaydolur
	Mesh->SetCollisionResponseToChannel(ECC_Interaction, ECR_Block);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AContainerActor::BeginPlay()
{
	Super::BeginPlay();
	if (!Inventory)
	{
		return;
	}
	for (const TPair<FName, int32>& Pair : StartingItems)
	{
		Inventory->AddItem(Pair.Key, Pair.Value);
	}
}

bool AContainerActor::CanInteract_Implementation(APlayerCharacter* Interactor) const
{
	if (!Inventory)
	{
		return false;
	}
	for (const FInventorySlot& Slot : Inventory->GetSlots())
	{
		if (!Slot.IsEmpty())
		{
			return true;
		}
	}
	return false;
}

FText AContainerActor::GetInteractionPrompt_Implementation() const
{
	return NSLOCTEXT("Inventory", "LootAllPrompt", "Hepsini Topla");
}

float AContainerActor::GetInteractionDistance_Implementation() const
{
	return InteractionDistance;
}

void AContainerActor::OnInteract_Implementation(APlayerCharacter* Interactor)
{
	if (!Interactor || !Inventory)
	{
		return;
	}
	UInventoryComponent* PlayerInv = Interactor->GetInventoryComponent();
	if (!PlayerInv)
	{
		return;
	}

	// Aktarim sirasinda Inventory->Slots degisecegi icin once anlik kopyasini al
	const TArray<FInventorySlot> SlotsCopy = Inventory->GetSlots();
	for (const FInventorySlot& Slot : SlotsCopy)
	{
		if (!Slot.IsEmpty())
		{
			Inventory->TransferItemTo(PlayerInv, Slot.ItemID, Slot.Count);
		}
	}
}
