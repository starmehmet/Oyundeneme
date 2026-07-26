#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "HarvestNodeDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #29 — Hasat Dugumleri ayarlari (UItemDatabaseSettings ile ayni desen — Sistem #5).
 * Project Settings -> Game -> Harvest Node Database altinda gorunur.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Harvest Node Database"))
class SURVIVALGAME_API UHarvestNodeDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHarvestNodeDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_HarvestNodes — satir yapisi FHarvestNodeDefinition olmali. */
	UPROPERTY(EditAnywhere, Config, Category = "Harvest")
	TSoftObjectPtr<UDataTable> HarvestNodeTable;
};
