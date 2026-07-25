#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Construction/BuildingDefinition.h"
#include "BuildingDatabase.generated.h"

class UDataTable;

/**
 * Sistem #7 — Bina tanımlarının tek sahibi. DT_Buildings'i (UBuildingDatabaseSettings
 * üzerinden) yükler, FName→FBuildingDefinition önbelleği kurar. Önbellek DataTable
 * satırlarının KOPYASINI tutar (ham işaretçi değil) — UItemDatabase ile aynı desen.
 */
UCLASS()
class SURVIVALGAME_API UBuildingDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool FindBuilding(FName BuildingID, FBuildingDefinition& OutDefinition) const;

	UFUNCTION(BlueprintPure, Category = "Construction")
	bool IsValidBuildingID(FName BuildingID) const { return Cache.Contains(BuildingID); }

	UFUNCTION(BlueprintPure, Category = "Construction")
	int32 GetBuildingCount() const { return Cache.Num(); }

private:
	void RebuildCache();

	UPROPERTY()
	TObjectPtr<UDataTable> LoadedTable;

	TMap<FName, FBuildingDefinition> Cache;
};
