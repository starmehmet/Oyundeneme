#pragma once

#include "CoreMinimal.h"
#include "ThermalZone.generated.h"

/**
 * Sistem #12 — Kayıtlı bir termal bölge (ör. bir bina). MIMARI.md'nin `AActor*` anahtarlı
 * `TMap<AActor*, FThermalZone>` taslağı `TWeakObjectPtr` ile uygulanır (Sistem #10'un
 * kayıt-registry desenindeki gibi kendiliğinden temizlenir — bkz. UTemperatureSimulation).
 */
USTRUCT(BlueprintType)
struct FThermalZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature", meta = (ClampMin = "0.0"))
	float Radius = 500.0f;

	/** Bölgenin dış ortamdan bağımsız temel sıcaklığı (ör. bir mağaranın doğal sıcaklığı). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	float BaseTemperature = 20.0f;

	/** Ek ısı kaynağı katkısı (ör. bir ocak/ısıtıcı) — pozitif ısıtır, negatif soğutur. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	float HeatSourcePower = 0.0f;

	/** [0,1] — 0: yalıtım yok (tamamen dış ortama bağlı), 1: tam yalıtılmış (yalnızca BaseTemperature+HeatSourcePower). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InsulationFactor = 0.5f;
};
