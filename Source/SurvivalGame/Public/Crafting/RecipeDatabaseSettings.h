#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "RecipeDatabaseSettings.generated.h"

class UDataTable;

/**
 * Sistem #6 — Zanaat ayarları (UTimeSettings/UItemDatabaseSettings ile aynı desen).
 * Project Settings → Game → Recipe Database altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Recipe Database"))
class SURVIVALGAME_API URecipeDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URecipeDatabaseSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** DT_Recipes — satır yapısı FRecipeDefinition olmalı. */
	UPROPERTY(EditAnywhere, Config, Category = "Recipes")
	TSoftObjectPtr<UDataTable> RecipeTable;
};
