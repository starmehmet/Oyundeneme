#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Save/SaveDataTypes.h"
#include "SaveGameManager.generated.h"

class APlayerCharacter;

/**
 * Sistem #17 — MIMARI.md'nin `USaveGameManager`'ı. MIMARI'nin `TMap<FString, FSaveSlot>
 * SaveSlots` (bellek-içi yuva-kayıt-defteri) İCAT EDİLMEDİ — platformun kendi
 * `ISaveGameSystem`'i (`UGameplayStatics::DoesSaveGameExist` vb.) zaten diskteki yuvaları
 * takip ediyor, aynı bilgiyi ikinci kez bellekte tutmak tekrar olurdu (bkz. ADR).
 *
 * `FTickableGameObject` — otomatik kaydetme periyodunu (`UWeatherSimulation::
 * EvaluationInterval`/`USnowAccumulation::AvalancheCheckInterval` ile aynı periyodik-kontrol
 * deseni) VE gerçek (duvar-saati) oynama süresini biriktirmek için.
 *
 * Kapsam (SaveVersion=2): Zaman+Oyuncu (v1) + Insaat/Uretim/Lojistik/Hava/Kar/Kaynak/
 * Hasat/NPC/Gorev/Ses (v2). Kaydetme/yukleme SENKRON'dur — gercek ihtiyac (500+ bina)
 * ortaya cikinca `UGameplayStatics::AsyncSaveGameToSlot` deseni eklenebilir.
 */
UCLASS()
class SURVIVALGAME_API USaveGameManager : public UGameInstanceSubsystem, public FTickableGameObject
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(USaveGameManager, STATGROUP_Tickables);
	}

	/** Canlı sistemlerden anlık durumu toplayıp sıkıştırıp diske yazar. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(const FString& SlotName);

	/**
	 * Diskten okuyup açıp canlı sistemlere uygular. Kayıt YOKSA/BOZUKSA (decompress/cast
	 * başarısız) `false` döner, HİÇBİR ŞEYİ değiştirmez (DoD: "kayıt bozulmasını idare et").
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave(const FString& SlotName);

	/**
	 * En son BAŞARIYLA yazılan yuvaya (otomatik veya elle, hangisi bu oturumda en son
	 * olduysa) döner. Bu oturumda hiç kaydetme olmadıysa `Settings->AutosaveSlotName`'i
	 * dener (önceki bir oturumdan kalmış olabilir). Hiçbiri yoksa `false` döner, HİÇBİR
	 * ŞEYİ değiştirmez (`LoadGame` ile aynı sözleşme).
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool RevertToLastSave();

	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveExist(const FString& SlotName) const;

	UFUNCTION(BlueprintPure, Category = "Save")
	float GetTotalPlayTimeSeconds() const { return TotalPlayTimeSeconds; }

private:
	FGameSaveData BuildSaveDataFromLiveSystems() const;
	bool ApplySaveDataToLiveSystems(const FGameSaveData& Data);
	APlayerCharacter* FindPlayerCharacter() const;

	float TotalPlayTimeSeconds = 0.0f;
	float TimeSinceLastAutosave = 0.0f;
	FString LastSavedSlotName;
};
