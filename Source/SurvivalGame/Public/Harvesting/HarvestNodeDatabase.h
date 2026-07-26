#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Harvesting/HarvestNodeDefinition.h"
#include "HarvestNodeDatabase.generated.h"

class UDataTable;

/**
 * Sistem #29 — Hasat dugumu tanimlarinin tek sahibi (UItemDatabase ile birebir ayni desen —
 * Sistem #5). DT_HarvestNodes'u (UHarvestNodeDatabaseSettings uzerinden) yukler,
 * FName->FHarvestNodeDefinition onbellegi kurar. Onbellek DataTable satirlarinin KOPYASINI
 * tutar (ham isaretci degil) — tablo yeniden yuklense/GC'lense bile guvenli.
 */
UCLASS()
class SURVIVALGAME_API UHarvestNodeDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Harvest")
	bool FindNodeDefinition(FName NodeID, FHarvestNodeDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Harvest")
	bool IsValidNodeID(FName NodeID) const { return Cache.Contains(NodeID); }

	UFUNCTION(BlueprintPure, Category = "Harvest")
	int32 GetNodeDefinitionCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<FName, FHarvestNodeDefinition> Cache;
};
