#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WindSimulation.generated.h"

/**
 * Sistem #13 — Herhangi bir konumdaki rüzgar vektörünün tek sahibi. `UWeatherSimulation`/
 * `UResourceSimulation`/`UTemperatureSimulation`'ın AKSİNE `FTickableGameObject` DEĞİLDİR —
 * biriktirilen/ilerletilen hiçbir durumu yok: küresel yön/hız her sorguda doğrudan
 * `UWeatherSimulation::GetCurrentState()`'ten okunur (Sistem #11'in zaten sahip olduğu
 * `WindSpeed`/`WindDirection` alanları — paralel bir "küresel rüzgar" kavramı YOK, tek
 * doğruluk kaynağı), yerel hamle (gust) ise konum+oyun-zamanına göre TAMAMEN durumsuz/
 * deterministik hesaplanır (bkz. `WindMath.h` ADR notu). Bu yüzden Tick'e ya da bir
 * kayıt/temizleme yaşam döngüsüne ihtiyaç yok.
 */
UCLASS()
class SURVIVALGAME_API UWindSimulation : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Belirtilen konumda TAM rüzgar vektörü (küresel yön×hız + yerel hamle). */
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetWindVectorAt(const FVector& Position) const;

	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetWindSpeedAt(const FVector& Position) const;

	/**
	 * Bir konumdaki/alandaki yapıya etkiyen rüzgar yükü (basınç) — bkz.
	 * `SurvivalWind::ComputeWindLoad`. Şu an yalnızca VERİ olarak hesaplanır; `ABuildingBase`'in
	 * gerçek bir yapısal-bütünlük/kararlılık alanı henüz yok (28 sistemde ayrı bir "Yapısal
	 * Bütünlük" sistemi de yok) — tüketen bir sistem ortaya çıkınca kullanılabilir (bkz. ADR).
	 */
	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetWindLoadOnStructure(const FVector& Position, float ExposedArea) const;

	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetGlobalWindSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetGlobalWindDirection() const;
};
