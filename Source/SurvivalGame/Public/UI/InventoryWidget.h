#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryComponent.h"
#include "InventoryWidget.generated.h"

/**
 * Sistem #20 — MIMARI'nin `UInventoryWidget`si: yalnizca VERI BAGLAMA katmani (bir envantere
 * abone olup slot degisikliklerini iletir). MIMARI'nin `UInventoryGridPanel` alani (gercek
 * ızgara/ikon UMG duzeni) BILEREK yazilmadi — motorda boyle bir sinif yok (MIMARI'nin kendi
 * ornek/uydurma taslagi) ve gercek bir ızgara duzeni Widget Blueprint icerik calismasi
 * gerektirir (CLAUDE.md: "C++ (cekirdek sistemler) + Blueprint (icerik/gorsellik)"). Gercek
 * gorsel (WBP_Inventory) bu sinifi miras alip `OnBoundSlotChanged`'i dinler.
 */
UCLASS()
class SURVIVALGAME_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Onceki baglanti (varsa) once BIRAKILIR, sonra yeni envanterin MEVCUT tum slotlari icin
	 * bir kez OnBoundSlotChanged tetiklenir (ilk cizim icin). */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBoundInventory(UInventoryComponent* NewInventory);

	/** BoundInventory yok edilmis olabilecegi icin TWeakObjectPtr::Get() guvenle nullptr doner
	 * (inceleme bulgusu — ULogisticsNetwork'un Source/Destination'iyla AYNI desen). */
	UFUNCTION(BlueprintPure, Category = "UI")
	UInventoryComponent* GetBoundInventory() const { return BoundInventory.Get(); }

protected:
	// AContainerActor gibi widget'tan BAGIMSIZ yasam donguslu bir aktore ait olabilir —
	// TObjectPtr (guclu referans) DEGIL, ULogisticsNetwork'un Source/Destination'iyla AYNI
	// TWeakObjectPtr deseni (inceleme bulgusu).
	TWeakObjectPtr<UInventoryComponent> BoundInventory;

	/** Baglı envanterde bir slot degisince (veya yeniden baglaninca mevcut tum slotlar icin)
	 * cagirilir — gercek ızgara/ikon cizimi Blueprint icerigine birakilir. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnBoundSlotChanged(int32 SlotIndex, const FInventorySlot& NewSlot);

	// UUserWidget
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleInventoryChanged(int32 SlotIndex, const FInventorySlot& NewSlot);
};
