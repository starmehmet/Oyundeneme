#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Harvesting/HarvestNodeType.h"
#include "HarvestNodeDefinition.generated.h"

/**
 * Sistem #29 — DT_HarvestNodes satiri. Kimlik, DataTable satir adinin kendisidir (FName) —
 * FItemDefinition/FBuildingDefinition ile ayni "tek dogruluk kaynagi" ilkesi, struct icinde
 * ayri bir ID alani YOK.
 */
USTRUCT(BlueprintType)
struct FHarvestNodeDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest")
	EHarvestNodeType NodeType = EHarvestNodeType::Generic;

	/** DT_Items satir adi — UItemDatabase::FindItem ile cozulur, burada KOPYALANMAZ. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest")
	FName YieldItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "1"))
	int32 YieldCountMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "1"))
	int32 YieldCountMax = 1;

	/** Kac hasattan sonra tukenir. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "1"))
	int32 HarvestsBeforeDepletion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0"))
	float RespawnSeconds = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest")
	FText InteractionPrompt;

	/** v1'de YAZILIR ama OKUNMAZ — alet-gereksinimi v1.5'e kadar bilincli ertelendi (bkz. Docs/MIMARI.md #29). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harvest")
	FString RequiredToolTag;
};
