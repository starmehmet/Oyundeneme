#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/ItemDefinition.h"
#include "ItemDatabase.generated.h"

class UDataTable;

/**
 * Sistem #5 — Öğe tanımlarının tek sahibi. DT_Items'ı (UItemDatabaseSettings üzerinden)
 * yükler, FName→FItemDefinition önbelleği kurar. Önbellek DataTable satırlarının
 * KOPYASINI tutar (ham işaretçi değil) — tablo yeniden yüklense/GC'lense bile güvenli.
 */
UCLASS()
class SURVIVALGAME_API UItemDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Items")
	bool FindItem(FName ItemID, FItemDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Items")
	bool IsValidItemID(FName ItemID) const { return Cache.Contains(ItemID); }

	UFUNCTION(BlueprintCallable, Category = "Items")
	TArray<FName> FindItemsByTag(const FString& Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Items")
	TArray<FName> FindItemsByCategory(EItemCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Items")
	int32 GetItemCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<FName, FItemDefinition> Cache;
};
