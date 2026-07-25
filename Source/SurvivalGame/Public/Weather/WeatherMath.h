#pragma once

#include "CoreMinimal.h"
#include "Weather/WeatherTypes.h"

/**
 * Sistem #11 — Hava Durumu: saf, durum tutmayan ağırlıklı-seçim/geçiş-ilerleme/interpolasyon
 * mantığı. Yan etkisiz; birim testleri Private/Tests/WeatherSimulationMathTests.cpp.
 *
 * Rastgelelik gerektiren fonksiyonlar (ör. SelectWeightedIndex) rastgele değeri PARAMETRE
 * olarak alır — çağıran taraf `FMath::FRand()` verir, testler SABİT değerler vererek tam
 * seçim davranışını doğrular (rastgeleliği enjekte etme deseni).
 */
namespace SurvivalWeather
{
	/**
	 * Ağırlıklı rastgele seçim: RandomValue01 [0,1) değerinin Weights toplamı içindeki
	 * konumuna göre hangi index'in seçildiğini döner. Tüm ağırlıklar <=0 ise (tanımsız/boş
	 * liste) güvenlik için 0 döner.
	 */
	inline int32 SelectWeightedIndex(const TArray<float>& Weights, float RandomValue01)
	{
		float Total = 0.0f;
		for (const float W : Weights)
		{
			Total += FMath::Max(0.0f, W);
		}
		if (Total <= 0.0f || Weights.Num() == 0)
		{
			return 0;
		}

		const float Target = FMath::Clamp(RandomValue01, 0.0f, 0.999999f) * Total;
		float Cumulative = 0.0f;
		for (int32 i = 0; i < Weights.Num(); ++i)
		{
			Cumulative += FMath::Max(0.0f, Weights[i]);
			if (Target < Cumulative)
			{
				return i;
			}
		}
		return Weights.Num() - 1; // yuvarlama guvenligi
	}

	/** İlerleme oranı [0,1]. TransitionDuration<=0 anlık geçiş demektir — her zaman tamamlanmış sayılır. */
	inline float ComputeTransitionProgress(float ElapsedTime, float TransitionDuration)
	{
		if (TransitionDuration <= 0.0f)
		{
			return 1.0f;
		}
		return FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);
	}

	/**
	 * İki durum arasında doğrusal interpolasyon. Condition, Alpha>=1 olana kadar From'da kalır.
	 * WindDirection İSTİSNA — düz `FMath::Lerp` İKİ BİRİM VEKTÖR arasında ("ComputeWindDirection"
	 * her zaman birim vektör üretir) doğrusal enterpolasyon yapılırsa neredeyse-ters yönlü çiftlerde
	 * Alpha≈0.5'te vektör SIFIRA yaklaşır/eşitlenir (yön tanımsızlaşır) — bu yüzden birim
	 * vektörler için tasarlanmış `FVector::SlerpNormals` (küresel yol) kullanılır (inceleme bulgusu).
	 */
	inline FWeatherState LerpWeatherState(const FWeatherState& From, const FWeatherState& To, float Alpha)
	{
		const float A = FMath::Clamp(Alpha, 0.0f, 1.0f);

		FWeatherState Result;
		Result.Condition = (A >= 1.0f) ? To.Condition : From.Condition;
		Result.Temperature = FMath::Lerp(From.Temperature, To.Temperature, A);
		Result.Humidity = FMath::Lerp(From.Humidity, To.Humidity, A);
		Result.WindSpeed = FMath::Lerp(From.WindSpeed, To.WindSpeed, A);
		Result.WindDirection = FVector::SlerpNormals(From.WindDirection, To.WindDirection, A);
		Result.VisibilityDistance = FMath::Lerp(From.VisibilityDistance, To.VisibilityDistance, A);
		Result.Precipitation = FMath::Lerp(From.Precipitation, To.Precipitation, A);
		return Result;
	}

	/** RandomAngle01 [0,1) -> yatay düzlemde birim rüzgar yönü vektörü (Z=0). */
	inline FVector ComputeWindDirection(float RandomAngle01)
	{
		const float AngleDegrees = FMath::Clamp(RandomAngle01, 0.0f, 1.0f) * 360.0f;
		const float AngleRadians = FMath::DegreesToRadians(AngleDegrees);
		return FVector(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f);
	}
}
