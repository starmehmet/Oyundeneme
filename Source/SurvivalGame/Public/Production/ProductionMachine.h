#pragma once

#include "CoreMinimal.h"
#include "Construction/BuildingBase.h"
#include "Production/ProductionState.h"
#include "Inventory/InventoryComponent.h"
#include "ProductionMachine.generated.h"

class UInventoryComponent;
class AStorageNode;
struct FProductionRecipe;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionCompleted, FName, RecipeID);

/**
 * Sistem #9 — `ABuildingBase`'den türer (MIMARI.md'nin öngördüğü hiyerarşi; Sistem #7'de bu
 * genişleme için bilerek `virtual` bırakıldı). Faz 1 entegrasyon borcu (kapatıldı):
 * `UConstructionComponent`/`build_place` akışına `FBuildingDefinition::BuildingClass` üzerinden
 * BAĞLANDI — oyuncu artık bir üretim makinesini gerçek inşaat sistemiyle yerleştirebiliyor;
 * yerleşince `BeginConstruction` override'ı `ProductionRecipeIDs`'i, `BeginPlay` ise
 * `InputBuffer`/`OutputBuffer`'i lojistik ağına (bkz. `ShouldRegisterLogisticsNodes`) kaydeder.
 *
 * Kendi `PrimaryActorTick`'i YOK — `UProductionManager` tarafından kare-bölümlemeli olarak
 * (`AdvanceProduction`) tetiklenir; bkz. o sınıfın yorumu.
 *
 * `AvailableRecipeIDs` yalnızca hangi RecipeID'lerin desteklendiğini tutar — gerçek tarif
 * verisi HER ZAMAN `UProductionRecipeDatabase`'den çözülür (tek-doğruluk-kaynağı, Sistem
 * #5/#6/#7 ile aynı desen; MIMARI'nin İNLİNE `TArray<FProductionRecipe>` taslağından
 * BİLİNÇLİ sapma, bkz. ADR).
 */
UCLASS()
class SURVIVALGAME_API AProductionMachine : public ABuildingBase
{
	GENERATED_BODY()

public:
	AProductionMachine();

	/** Faz 1 entegrasyon borcu: `Super::BeginConstruction` sonrasi `InDefinition.ProductionRecipeIDs`'i
	 * `AvailableRecipeIDs`'e ekler — DataTable veri tek dogruluk kaynagi, koda gomulu liste yok. */
	virtual void BeginConstruction(FName InBuildingID, const FBuildingDefinition& InDefinition, const FIntPoint& InGridCoord) override;

	/**
	 * RecipeID NAME_None ise makineyi Idle'a alır. Aksi halde AvailableRecipeIDs içinde ve
	 * veritabanında bulunmalı — MIMARI'nin iki adımlı SetActiveRecipe+StartProduction'ı TEK
	 * adıma indirgendi (henüz bir seçim UI'ı yok; üretim, kaynaklar elverdiği sürece Tick
	 * üzerinden kendiliğinden başlar/durur — bkz. ADR).
	 */
	UFUNCTION(BlueprintCallable, Category = "Production")
	bool SetActiveRecipe(FName RecipeID);

	UFUNCTION(BlueprintPure, Category = "Production")
	FName GetActiveRecipeID() const { return ActiveRecipeID; }

	UFUNCTION(BlueprintPure, Category = "Production")
	EProductionState GetProductionState() const { return State; }

	/** Normalize edilmiş ilerleme [0,1] — ham (saniye cinsi) Progress DEĞİL (inceleme bulgusu:
	 * eskiden ham değer dönüyordu, ProductionTime!=1 olan her tarifte UI'da anlamsız değerler
	 * gösterirdi; CraftingComponent'in OnCraftingProgress'i normalize etmesiyle aynı desen). */
	/** Sistem #13: `AWindTurbine` bunu tarif-tabanlı olmayan (ruzgar->guc orani) bir anlamla
	 * override eder — bu yuzden virtual, taban sinifin recipe-yorumu tek uygulama degil. */
	UFUNCTION(BlueprintPure, Category = "Production")
	virtual float GetProgress() const;

	float GetRawProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "Production")
	float GetCurrentEnergy() const { return CurrentEnergy; }

	UFUNCTION(BlueprintPure, Category = "Production")
	UInventoryComponent* GetInputBuffer() const { return InputBuffer; }

	UFUNCTION(BlueprintPure, Category = "Production")
	UInventoryComponent* GetOutputBuffer() const { return OutputBuffer; }

	/** Faz 1 entegrasyon borcu: lojistik agina kayitli giris/cikis proxy dugumleri (PIE/dev
	 * dogrulamasi icin — ShouldRegisterLogisticsNodes()==false ise nullptr doner, ör. AWindTurbine). */
	UFUNCTION(BlueprintPure, Category = "Production")
	AStorageNode* GetInputLogisticsNode() const { return InputLogisticsNode; }

	UFUNCTION(BlueprintPure, Category = "Production")
	AStorageNode* GetOutputLogisticsNode() const { return OutputLogisticsNode; }

	/** Faz 1 entegrasyon borcu: Input/OutputBuffer'i Sistem #8'in lojistik agina iki AStorageNode
	 * proxy'si (MachineInput/MachineOutput) ile kaydeder mi? `AWindTurbine` (InputBuffer/OutputBuffer'i
	 * hic KULLANMAZ) false doner — kullanilmayan bos arabellekler agi kirletmesin (bkz. ADR). */
	virtual bool ShouldRegisterLogisticsNodes() const { return true; }

	/**
	 * `UProductionManager` tarafından çağrılır — bu makinenin EN SON güncellendiği andan
	 * `CurrentGameTime`'a kadar GERÇEKTEN geçen süreyi hesaplayıp `Tick_Production`'a iletir.
	 * Kare-bölümleme yüzünden bu çağrılar arası (per-frame DEĞİL) olabilir; "kayıp" zaman
	 * olmaması için ilerleme her zaman GERÇEK geçen süre kadar ilerler.
	 *
	 * Sistem #13: `AWindTurbine` bunu TAMAMEN farklı (recipe'siz, rüzgar->enerji) bir mantıkla
	 * override eder — bu yüzden virtual. `UProductionManager::Tick`, makineleri HER ZAMAN
	 * `AProductionMachine*` taban-sınıf işaretçisi üzerinden çağırır (bkz. `Machines` dizisi);
	 * virtual OLMASAYDI bu çağrı her zaman taban sınıfın recipe-mantığına giderdi (turbine
	 * sessizce hiçbir zaman enerji üretmezdi, hiçbir hata/log olmadan) — inceleme bulgusu.
	 */
	virtual void AdvanceProduction(double CurrentGameTime);

	/**
	 * `UProductionManager::RegisterMachine` tarafından kayıt anında çağrılır — saati GERÇEK
	 * kayıt anına sabitler. Aksi halde (varsayılan `LastProductionUpdateTime=0.0` ile) ilk
	 * `AdvanceProduction` çağrısı oyunun O ANA KADAR geçen TÜM süresini "kayıp zaman" sayıp
	 * devasa, yanlış bir tek-seferlik üretim sıçramasına yol açardı.
	 */
	void ResetProductionClock(double CurrentGameTime) { LastProductionUpdateTime = CurrentGameTime; }

	void RestoreStateForLoad(FName InRecipeID, float InProgress, float InEnergy, EProductionState InState,
		const TArray<FInventorySlot>& InInput, const TArray<FInventorySlot>& InOutput);

	UPROPERTY(BlueprintAssignable, Category = "Production")
	FOnProductionCompleted OnProductionCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Tick_Production(float DeltaTime);
	bool HasSufficientInputs(const FProductionRecipe& Recipe) const;
	bool HasSufficientOutputRoom(const FProductionRecipe& Recipe) const;
	void CompleteProductionCycle(const FProductionRecipe& Recipe);

	UPROPERTY(VisibleAnywhere, Category = "Production")
	TObjectPtr<UInventoryComponent> InputBuffer;

	UPROPERTY(VisibleAnywhere, Category = "Production")
	TObjectPtr<UInventoryComponent> OutputBuffer;

	// Faz 1 entegrasyon borcu: InputBuffer/OutputBuffer'i lojistik agina KAYDEDEN proxy'ler —
	// kendi envanterlerini YARATMAZLAR, AStorageNode::BindExternalStorage ile GERCEK arabellege
	// baglanirlar (tek veri kopyasi). BeginPlay'de spawn edilir, EndPlay'de yok edilir.
	UPROPERTY()
	TObjectPtr<AStorageNode> InputLogisticsNode;

	UPROPERTY()
	TObjectPtr<AStorageNode> OutputLogisticsNode;

	/** Bu makine modelinin destekleyebileceği RecipeID'ler (bina/BP tanımı, seviye tasarımı). */
	UPROPERTY(EditAnywhere, Category = "Production")
	TArray<FName> AvailableRecipeIDs;

	UPROPERTY(EditAnywhere, Category = "Production", meta = (ClampMin = "0.0"))
	float MaxEnergy = 100.0f;

	/** Seviye tasarımcısının InputBuffer'a koyduğu başlangıç stoğu (AStorageNode ile aynı desen). */
	UPROPERTY(EditAnywhere, Category = "Production")
	TMap<FName, int32> StartingInputItems;

	UPROPERTY(BlueprintReadOnly, Category = "Production")
	FName ActiveRecipeID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Production")
	float Progress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Production")
	float CurrentEnergy = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Production")
	EProductionState State = EProductionState::Idle;

	double LastProductionUpdateTime = 0.0;
};
