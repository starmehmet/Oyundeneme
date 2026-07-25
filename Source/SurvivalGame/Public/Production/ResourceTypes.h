#pragma once

#include "CoreMinimal.h"
#include "Production/ResourceSimulationMath.h"
#include "ResourceTypes.generated.h"

/**
 * Sistem #10 — Global enerji ağının anlık durumu. `Frequency` gerçek bir AC frekansı DEĞİL,
 * MIMARI.md'nin stilize ettiği bir "şebeke istikrarı" göstergesi — üretim/tüketim dengesinden
 * ANLIK olarak türetilir (elektrik şebekelerinde frekans yüke neredeyse anında tepki verir,
 * termal sistemlerdeki gibi "atalet"i yoktur — bkz. FThermalBudget karşılaştırması).
 */
USTRUCT(BlueprintType)
struct FEnergyBudget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float TotalProduction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float TotalConsumption = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float CurrentBalance = 0.0f;

	/** 50 = nominal/sağlıklı, üretim tüketimi karşılayamadıkça düşer. Asla 50'yi AŞMAZ (bkz. ComputeFrequency). */
	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float Frequency = 50.0f;

	bool IsFrequencyStable() const { return SurvivalResource::IsFrequencyStable(Frequency); }
};

/**
 * Sistem #10 — Global termal bütçe. `CurrentTemperature`, Frequency'nin AKSİNE anlık değil
 * BİRİKEN bir değerdir — gerçek termal kütle/atalet modellenir (ısı üretim/dağıtım dengesi
 * zamanla sıcaklığı SÜRÜKLER, anında sıçratmaz). Soyut birim; ortam sıcaklığı henüz yok
 * (Sistem #12 — Sıcaklık), 0 = "nötr/ortam" varsayımı.
 */
USTRUCT(BlueprintType)
struct FThermalBudget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float TotalHeatProduction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float TotalHeatDissipation = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float CurrentTemperature = 0.0f;
};

/** Sistem #10 — Tek bir yakıt türünün stoğu. `ConsumptionRate`, `UResourceSimulation::Tick`
 * tarafından GERÇEKTEN tüketilir (pasif bir "bilgi" alanı değil) — bkz. sınıf yorumu. */
USTRUCT(BlueprintType)
struct FFuelReserve
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float CurrentAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float MaxAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resource")
	float ConsumptionRate = 0.0f;
};
