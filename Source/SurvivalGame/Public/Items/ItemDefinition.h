#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	RawMaterial,
	Component,
	Tool,
	Consumable
};

/**
 * Sistem #5 — Öğe Veritabanı: tek bir öğe türünün verisi (DT_Items satırı).
 * Kimlik, DataTable satır adının kendisidir (FName) — struct içinde ayrı bir
 * ID alanı YOK, tek doğruluk kaynağı ilkesi (iki yerde ID tutmak senkron kayması riski).
 */
USTRUCT(BlueprintType)
struct FItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::RawMaterial;

	/** Birim ağırlık — envanter ağırlık limiti hesabı bunu kullanır. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	/** 1 = yığılamaz (her slot tek adet). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	/** 0 = dayanıklılık takip edilmez (aşınmaz). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0.0"))
	float MaxDurability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TArray<FString> Tags;

	bool IsStackable() const { return MaxStackSize > 1; }
	bool HasDurability() const { return MaxDurability > 0.0f; }
	bool HasTag(const FString& Tag) const { return Tags.Contains(Tag); }
};
