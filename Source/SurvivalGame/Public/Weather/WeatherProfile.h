#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Weather/WeatherCondition.h"
#include "WeatherProfile.generated.h"

/**
 * Sistem #11 — Bir hava durumunun (DT_WeatherProfiles satırı) fiziksel tanımı VE görülme
 * olasılığı. Item/Recipe/Building/ProductionRecipe'den FARKLI olarak, gerçek arama anahtarı
 * DataTable satır ADI DEĞİL — açıkça tutulan `Condition` alanıdır (bkz. ADR): satır adları
 * yalnızca insan-okunur etiketler (ör. "Clear", "Firtina1"), `UWeatherProfileDatabase` her
 * zaman `Condition` enum'una göre önbellekler — çağıranlar zaten string değil enum ile çalışıyor.
 */
USTRUCT(BlueprintType)
struct FWeatherProfile : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	EWeatherCondition Condition = EWeatherCondition::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float Temperature = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Humidity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0"))
	float WindSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0"))
	float VisibilityDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Precipitation = 0.0f;

	/** Bu durumun genel iklimsel görülme sıklığı (diğer profillere göre ORANSAL — mutlak bir anlamı yok). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (ClampMin = "0.0"))
	float BaseWeight = 1.0f;
};
