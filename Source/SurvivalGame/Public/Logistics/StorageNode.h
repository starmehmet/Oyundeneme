#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Logistics/StorageNodeType.h"
#include "StorageNode.generated.h"

class UInventoryComponent;
class USceneComponent;

/**
 * Sistem #8 — Lojistik ağına kayıtlı, otomatik (oyuncu-etkileşimsiz) bir envanter erişim
 * noktası. `AContainerActor`'dan (Sistem #4, oyuncu "hepsini topla" etkileşimi) BİLEREK
 * AYRI — farklı bir sorumluluk (makine/ağ tüketimi, oyuncu değil). İkisini birleştirmek
 * (ör. bir konteynerin de ağa kayıtlı olması) gerçek ihtiyaç ortaya çıkınca (Sistem #9
 * üretim makineleri) değerlendirilebilir — şimdiden birleştirmek çalışan, test edilmiş
 * bir sınıfı (Sistem #4) spekülatif bir ihtiyaç için değiştirmek olurdu.
 */
UCLASS()
class SURVIVALGAME_API AStorageNode : public AActor
{
	GENERATED_BODY()

public:
	AStorageNode();

	UFUNCTION(BlueprintPure, Category = "Logistics")
	UInventoryComponent* GetStorage() const { return Storage; }

	UFUNCTION(BlueprintPure, Category = "Logistics")
	EStorageNodeType GetNodeType() const { return NodeType; }

	/** Düğüm TİPİ bu öğeyi kabul edebilir mi (gerçek kapasite kontrolü teslimat anında AddItem'in dönüşüyle yapılır). */
	UFUNCTION(BlueprintPure, Category = "Logistics")
	bool CanAcceptItem(FName ItemID) const;

	/**
	 * Faz 1 entegrasyon borcu (Sistem #9): kendi `CreateDefaultSubobject` envanteri YERİNE
	 * BAŞKA bir aktörün (ör. `AProductionMachine::InputBuffer`) SAHİP OLDUĞU bir envantere
	 * bağlanır — bu düğüm yalnızca lojistik ağına bir GİRİŞ NOKTASI olur, envanterin GERÇEK
	 * sahibi (ve yaşam döngüsü) dış aktörde kalır (tek veri kopyası, çifte-sayım riski yok).
	 * KABUL EDİLEN küçük bulgu (düzeltilmedi): constructor'ın kendi yarattığı varsayılan
	 * `Storage` alt-nesnesi bu çağrıyla üzerine yazılır — sahipsiz ama zararsız kalır (hiçbir
	 * kod ona erişmez, `StartingItems` bu programatik proxy'lerde her zaman boş olduğundan
	 * `BeginPlay`'deki döngü de bir şey yapmaz). Gerçek düzeltme (constructor'ı koşullu hale
	 * getirmek) tüm normal `AStorageNode` kullanımlarını etkileme riski taşırdı.
	 */
	void BindExternalStorage(UInventoryComponent* ExternalStorage) { Storage = ExternalStorage; }

	/** Faz 1 entegrasyon borcu: `AProductionMachine` kendi giriş/çıkış proxy düğümlerini
	 * `EStorageNodeType::MachineInput`/`MachineOutput` ile işaretlemek için kullanır. */
	void SetNodeType(EStorageNodeType NewType) { NodeType = NewType; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Kok bilesen — konum/mesafe hesaplarinin (FindBestDestination, RequestTransport) dogru
	// calismasi icin sart: UInventoryComponent bir USceneComponent DEGIL, kok olamaz. Root
	// atanmazsa GetActorLocation() her zaman (0,0,0) doner (inceleme bulgusu).
	UPROPERTY(VisibleAnywhere, Category = "Logistics")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Logistics")
	TObjectPtr<UInventoryComponent> Storage;

	UPROPERTY(EditAnywhere, Category = "Logistics")
	EStorageNodeType NodeType = EStorageNodeType::Container;

	/** Seviye tasarımcısının düğüme koyduğu başlangıç stoğu (ItemID → Adet). AContainerActor'daki ile aynı desen. */
	UPROPERTY(EditAnywhere, Category = "Logistics")
	TMap<FName, int32> StartingItems;
};
