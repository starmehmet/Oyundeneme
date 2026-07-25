#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "LogisticsNetwork.generated.h"

class AStorageNode;

/**
 * Sistem #8 — Aktif bir taşıma isteği. Kimlik yok — kuyruktaki konumu (index) yeterli,
 * `UCraftingComponent::FCraftingJob` ile aynı desen. `Source`/`Destination` TWeakObjectPtr —
 * bir düğüm taşıma sürerken yıkılabilir (ör. `build_demolish`), o zaman teslimat sessizce
 * değil LOGLANARAK atlanır (bkz. ULogisticsNetwork::CompleteTransport).
 */
USTRUCT()
struct FTransportRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemID = NAME_None;

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	TWeakObjectPtr<AStorageNode> Source;

	UPROPERTY()
	TWeakObjectPtr<AStorageNode> Destination;

	UPROPERTY()
	float ElapsedTime = 0.0f;

	UPROPERTY()
	float TransportTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransportDelivered, FName, ItemID);

/**
 * Sistem #8 — Lojistik ağının tek sahibi: düğüm kaydı, hedef önbelleği ("rota önbelleği"),
 * taşıma isteği kuyruğu, teslimat. `UTimeKeeper` (Sistem #1) ile aynı `FTickableGameObject`
 * deseni — `UGameInstanceSubsystem`'ler kendiliğinden tick almaz.
 *
 * Malzemeler İSTEK anında kaynaktan HEMEN çıkarılır (`UCraftingComponent::StartCrafting`'deki
 * "hemen tüket" deseniyle aynı, Sistem #6), hedefe yalnızca `TransportTime` dolunca eklenir:
 * "transit'teki" bir öğe iki envanterden de HENÜZ sayılmaz, çifte-sayım riski yok.
 *
 * `AConveyorBelt`/`ATransportDrone` (fiziksel öğe hareketi/spline üzerinde animasyon) bu
 * pasoda YAZILMADI — DoD'de yok, teslimat `ULogisticsSettings::TransportSpeed` sabitiyle
 * mesafe/hız zamanlayıcısı kullanır (`ACraftingStation`/`UInputHandler` kapsam-dışı
 * kararlarıyla aynı sınıf, bkz. ADR).
 *
 * Hedef önbelleği ("rota önbelleği") ÖĞE-bazlı DEĞİL, düz bir "kabul edebilecek düğümler"
 * listesidir — `CanAcceptItem` şu an yalnızca `NodeType`'a bakıyor (hiçbir düğüm belirli bir
 * ItemID'yi reddetmiyor), öğe-bazlı filtreleme eklenene kadar öğe-anahtarlı bir önbellek
 * yanıltıcı olurdu. Ağ değiştiğinde (Register/Unregister) yeniden kurulur — MIMARI.md'nin
 * kendi "Ölçeklenebilirlik Riski" çözümü ("ağ değişiminde rota önceden hesapla") budur.
 */
UCLASS()
class SURVIVALGAME_API ULogisticsNetwork : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ULogisticsNetwork, STATGROUP_Tickables);
	}

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void RegisterNode(AStorageNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Logistics")
	void UnregisterNode(AStorageNode* Node);

	/**
	 * Önbellekten bu ItemID'yi kabul eden, verilen konuma en yakın düğümü döner (yoksa nullptr).
	 * ExcludeNode (tipik olarak taşımanın Source'u) sonuçtan HARİÇ TUTULUR — aksi halde Source
	 * kendisi de bir alıcı tipiyse (ör. varsayılan Container) mesafesi 0 olduğu için her zaman
	 * kendini "en yakın hedef" olarak seçerdi (inceleme bulgusu).
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	AStorageNode* FindBestDestination(FName ItemID, const FVector& FromLocation, const AStorageNode* ExcludeNode = nullptr) const;

	/**
	 * Malzemeleri Source'tan HEMEN çıkarır, kuyruğa ekler. Destination boşsa FindBestDestination
	 * ile çözülür. Kaynak yetersizse VEYA hedef bulunamazsa false döner (hiçbir şey tüketilmez).
	 */
	UFUNCTION(BlueprintCallable, Category = "Logistics")
	bool RequestTransport(FName ItemID, int32 Count, AStorageNode* Source, AStorageNode* Destination = nullptr);

	UFUNCTION(BlueprintPure, Category = "Logistics")
	int32 GetNodeCount() const { return Nodes.Num(); }

	UFUNCTION(BlueprintPure, Category = "Logistics")
	int32 GetActiveTransportCount() const { return ActiveTransports.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "Logistics")
	FOnTransportDelivered OnTransportDelivered;

private:
	void RebuildAcceptorCache();
	void CompleteTransport(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<AStorageNode>> Nodes;

	// Rota onbellegi — bkz. sinif yorumu. Register/Unregister disinda yeniden kurulmaz.
	TArray<TWeakObjectPtr<AStorageNode>> AcceptorCache;

	UPROPERTY()
	TArray<FTransportRequest> ActiveTransports;
};
