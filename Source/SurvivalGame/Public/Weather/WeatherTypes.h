#pragma once

#include "CoreMinimal.h"
#include "Weather/WeatherCondition.h"
#include "WeatherTypes.generated.h"

/** Sistem #11 — Şu anki (veya iki durum arasında ara-değeri alınmış) hava durumu anlık görüntüsü. */
USTRUCT(BlueprintType)
struct FWeatherState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	EWeatherCondition Condition = EWeatherCondition::Clear;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float Temperature = 20.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float Humidity = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float WindSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	FVector WindDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float VisibilityDistance = 10000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float Precipitation = 0.0f;
};
