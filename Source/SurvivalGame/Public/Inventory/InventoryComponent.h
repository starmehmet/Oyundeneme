#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

/**
 * Sistem #4 — Envanter slotu. Kimlik DT_Items satır adı (FName) — UItemDatabase
 * üzerinden çözülür, slot kendisi tanım kopyası TUTMAZ (tek doğruluk kaynağı).
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 0;

	/** [0,1] — yalnızca dayanıklılığı olan öğeler için anlamlı (bkz. FItemDefinition::HasDurability). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float Durability = 1.0f;

	bool IsEmpty() const { return ItemID.IsNone() || Count <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, const FInventorySlot&, NewSlot);

/**
 * Sistem #4 — Sabit sayıda slot + ağırlık limitli envanter. Miktarlar DT_Items'tan
 * (UItemDatabase) çözülen FItemDefinition::Weight/MaxStackSize'a göre yığılır/kelepçelenir.
 * AddItem/RemoveItem GERÇEKTEN kabul/çıkarılan miktarı döner — çağıran taraf isteğin
 * tamamının karşılanmadığını (dolu envanter, ağırlık limiti) bu farktan anlar.
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	/**
	 * Gerçekte eklenen miktarı döner (Count'tan az olabilir — dolu/ağırlık limiti).
	 * SourceDurability yalnızca YENİ açılan slot(lar)a uygulanır — mevcut bir yığına
	 * eklenirken slotun durability'sine dokunulmaz (yığılabilir ögeler zaten dayanıklılık
	 * takip etmez varsayımıyla çalışır, bkz. FItemDefinition::HasDurability).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(FName ItemID, int32 Count, float SourceDurability = 1.0f);

	/** Gerçekte çıkarılan miktarı döner (Count'tan az olabilir — yetersiz stok). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemID, int32 Count);

	/** Bu envanterden Destination'a aktarır; gerçekte aktarılan miktarı döner. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 TransferItemTo(UInventoryComponent* Destination, FName ItemID, int32 Count);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 MinCount = 1) const { return GetItemCount(ItemID) >= MinCount; }

	/**
	 * Mutasyonsuz kapasite tahmini (Sistem #9 üretim makinelerinin çıktı-bloke durumunu
	 * gerçekten AddItem çağırmadan ÖNCEDEN değerlendirebilmesi için). YALNIZCA ağırlık
	 * bütçesine bakar — AddItem'in tam slot/yığın mantığını (mevcut yığınlar, boş slot
	 * sayısı) BİREBİR simüle etmez; nadir bir kenar durumda ("ağırlık uygun ama slotlar
	 * dolu") bu true dönüp gerçek AddItem yine de kısmi kabul edebilir — çağıranlar zaten
	 * AddItem'in dönüş değerini kontrol edip eksik kabulü loglamalı (StartCrafting/
	 * ConfirmPlacement/RequestTransport'taki desen).
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasRoomFor(FName ItemID, int32 Count) const;

	/**
	 * HasRoomFor'un çoklu-öğe hâli — TOPLAM ağırlığı TEK seferde kontrol eder. Aynı kontrolü
	 * her öğe türü için AYRI AYRI çağırmak (HasRoomFor tek tek), her çağrıyı AYNI (henüz
	 * değişmemiş) CurrentWeight'e karşı ölçer — birden fazla farklı öğe türü içeren bir
	 * teslimatta her tür TEK BAŞINA sığar görünüp TOPLAM ağırlık aşılabilir (Sistem #9
	 * inceleme bulgusu: AProductionMachine'in çok-satırlı çıktılarında yaşandı).
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasRoomForBatch(const TMap<FName, int32>& Items) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const { return CurrentWeight; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const { return MaxWeight; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventorySlot>& GetSlots() const { return Slots; }

	/**
	 * Sistem #17 — kaydedilmiş TAM slot dizisini olduğu gibi yükler ve `CurrentWeight`'i
	 * `UItemDatabase`'den YENİDEN hesaplar. `AddItem` döngüsünden BİLEREK FARKLI: `AddItem`
	 * yığınlama/boş-slot arama mantığından geçer (kaydedilenden FARKLI bir slot dağılımı
	 * üretebilir, DB'ye satır-satır sorgu atar); bu, kaydedilen düzeni BİREBİR geri yükler.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RestoreSlots(const TArray<FInventorySlot>& SavedSlots);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

protected:
	virtual void BeginPlay() override;

private:
	int32 FindSlotWithRoomFor(FName ItemID, int32 MaxStackSize) const;
	int32 FindFirstEmptySlot() const;
	void BroadcastSlot(int32 SlotIndex);

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 SlotCount = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "0.0"))
	float MaxWeight = 50.0f;

	// BlueprintReadOnly kullanilamaz — private uye, zaten GetSlots() ile public okunuyor
	UPROPERTY()
	TArray<FInventorySlot> Slots;

protected:
	// PIE/MCP dogrulamasi icin okunabilir (GetCurrentWeight() zaten public getter —
	// bu yalnizca MCP'nin fonksiyon degil property okuyabilmesi icin, bkz. Sistem #3 InteractionCount deseni)
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float CurrentWeight = 0.0f;
};
