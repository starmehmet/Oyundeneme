#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "BuildingDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #7 — Bina Veritabanı ayarları (UItemDatabaseSettings ile aynı desen — Sistem #5).
 * Project Settings → Game → Building Database altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Building Database"))
class SURVIVALGAME_API UBuildingDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBuildingDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_Buildings — satır yapısı FBuildingDefinition olmalı. */
	UPROPERTY(EditAnywhere, Config, Category = "Buildings")
	TSoftObjectPtr<UDataTable> BuildingDefinitionTable;
};
