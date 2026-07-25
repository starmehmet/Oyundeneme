#include "UI/InventoryWidget.h"
#include "SurvivalGame.h"

void UInventoryWidget::SetBoundInventory(UInventoryComponent* NewInventory)
{
	if (BoundInventory.IsValid())
	{
		BoundInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}

	BoundInventory = NewInventory;

	if (!BoundInventory.IsValid())
	{
		UE_LOG(LogSurvivalUI, Verbose, TEXT("SetBoundInventory: NewInventory==null, baglanti kaldirildi"));
		return;
	}

	BoundInventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);

	const TArray<FInventorySlot>& Slots = BoundInventory->GetSlots();
	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		OnBoundSlotChanged(SlotIndex, Slots[SlotIndex]);
	}

	UE_LOG(LogSurvivalUI, Log, TEXT("SetBoundInventory: %d slot ile baglanti kuruldu"), Slots.Num());
}

void UInventoryWidget::HandleInventoryChanged(int32 SlotIndex, const FInventorySlot& NewSlot)
{
	OnBoundSlotChanged(SlotIndex, NewSlot);
}

void UInventoryWidget::NativeDestruct()
{
	// Inceleme bulgusu (major): widget HideWidget/PopScreen ile viewport'tan kaldirilinca
	// (motor kaynagindan dogrulandi: RemoveFromParent bunu SENKRON tetikler) envanterin
	// OnInventoryChanged abonelik listesinde kalici bir "olu" giris birakmamak icin baglanti
	// burada da BIRAKILIR — SetBoundInventory'nin tekrar cagrilmasina bagli kalinmaz.
	if (BoundInventory.IsValid())
	{
		BoundInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}
	Super::NativeDestruct();
}
