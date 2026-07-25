#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "ItemDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #5 — Öğe Veritabanı ayarları (UTimeSettings ile aynı desen — Sistem #1).
 * Project Settings → Game → Item Database altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Item Database"))
class SURVIVALGAME_API UItemDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UItemDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_Items — satır yapısı FItemDefinition olmalı. */
	UPROPERTY(EditAnywhere, Config, Category = "Items")
	TSoftObjectPtr<UDataTable> ItemDefinitionTable;
};
