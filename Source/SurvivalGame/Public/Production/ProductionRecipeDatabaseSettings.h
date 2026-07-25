#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "ProductionRecipeDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #9 — Üretim Tarifleri Veritabanı ayarları (UItemDatabaseSettings ile aynı desen).
 * Project Settings → Game → Production Recipe Database altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Production Recipe Database"))
class SURVIVALGAME_API UProductionRecipeDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UProductionRecipeDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_ProductionRecipes — satır yapısı FProductionRecipe olmalı. */
	UPROPERTY(EditAnywhere, Config, Category = "Production")
	TSoftObjectPtr<UDataTable> ProductionRecipeTable;
};
