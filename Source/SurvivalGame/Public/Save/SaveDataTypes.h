#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/InventoryComponent.h"
#include "SaveDataTypes.generated.h"

/**
 * Sistem #17 — MIMARI.md'nin `FGameSaveData`'sı; `FSaveDataVersion` ayrı bir struct YERİNE
 * düz bir `int32` (tek alanlık bir struct'ın kendisi gereksiz bir katmandı, bkz. ADR).
 *
 * DÜRÜST kapsam notu: bu ilk pasoda YALNIZCA Zaman (Sistem #1) + Oyuncu (konum/can/vücut
 * sıcaklığı/envanter) kaydediliyor — İnşaat/Üretim/Lojistik/Kaynak/Hava/NPC/Görev durumu
 * BU PASODA kaydedilmiyor (her biri kendi kaydet/yükle arayüzünü henüz açığa çıkarmadı;
 * `UTimeKeeper::GetTotalGameSeconds/SetTotalGameSeconds` gibi TEK sistem bu iş için ÖNCEDEN
 * hazırlanmıştı, bkz. o sınıfın Sistem #1 yorumu). MIMARI'nin "tam oyun durumu" DoD'si
 * gerçek anlamda HER sistemin kendi kaydet/yükle uç noktasını eklemesini gerektirir — bu,
 * mekanizmanın (sıkıştırma/sürüm/bozulma/yuva yönetimi) doğru kurulduğu bu pasodan SONRA,
 * gerçek ihtiyaç ortaya çıktıkça (her sistem kendi ADR'siyle) genişletilebilir bir seam.
 */
USTRUCT()
struct FGameSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SaveVersion = 1;

	/** Gerçek (duvar-saati) oynama süresi — `UTimeKeeper`'ın ölçekli/duraklatılabilir
	 * oyun-içi zamanından BAĞIMSIZ, `USaveGameManager`'ın kendi Tick'inde biriktirilir. */
	UPROPERTY()
	float TotalPlayTimeSeconds = 0.0f;

	/** `UTimeKeeper::GetTotalGameSeconds()`/`SetTotalGameSeconds()` — Sistem #1'in bu iş için
	 * ÖNCEDEN hazırlanmış arayüzü. */
	UPROPERTY()
	double TotalGameSeconds = 0.0;

	UPROPERTY()
	FVector PlayerPosition = FVector::ZeroVector;

	UPROPERTY()
	float PlayerHealth = 100.0f;

	UPROPERTY()
	float PlayerBodyTemperature = 37.0f;

	/** `FInventorySlot` (Sistem #4) DOĞRUDAN yeniden kullanılır — paralel bir kayıt-özel
	 * envanter-satırı türü İCAT EDİLMEDİ. */
	UPROPERTY()
	TArray<FInventorySlot> PlayerInventory;
};

/**
 * `UGameplayStatics::SaveGameToMemory`/`LoadGameFromMemory`'nin çalıştığı gerçek `USaveGame`
 * sarmalayıcısı — motorun kendi property-serileştirmesi bu sınıfın TÜM `UPROPERTY` alanlarını
 * otomatik olarak (elle `FMemoryWriter` yazmadan) ele alır; `USaveGameManager` yalnızca ham
 * baytları sıkıştırıp diske yazar (bkz. `SaveDataSerializer.h`).
 */
UCLASS()
class SURVIVALGAME_API USurvivalSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameSaveData Payload;
};
