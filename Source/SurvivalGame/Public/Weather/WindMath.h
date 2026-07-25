#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #13 — Rüzgar Simülasyonu: saf, durum tutmayan yerel-hamle (gust) ve rüzgar-yükü/türbin
 * matematiği. Yan etkisiz; birim testleri Private/Tests/WindSimulationMathTests.cpp.
 *
 * Yerel hamle (gust) KAYITLI bir grid (MIMARI'nin `TArray<FVector> LocalGustsPerGrid`'i) DEĞİL —
 * konum+oyun-zamanına göre TAMAMEN durumsuz/deterministik hesaplanır (bir kayıt/temizleme
 * yaşam döngüsü gerektirmez, gerçek rastgelelik de gerekmez — bkz. ADR).
 */
namespace SurvivalWind
{
	/** Bir grid hücresine sabit, deterministik bir faz kaydırması atar (gerçek RNG değil — aynı
	 * hücre her zaman aynı fazı üretir, hücreler arası görünür şekilde farklılaşır). */
	inline float ComputeGustPhaseOffset(int32 GridX, int32 GridY)
	{
		const float Seed = (static_cast<float>(GridX) * 12.9898f) + (static_cast<float>(GridY) * 78.233f);
		return FMath::Fmod(FMath::Abs(Seed), 2.0f * PI);
	}

	/** Dünya konumunu bir gust-grid hücresine böler (Construction'ın `WorldToGridCoord`'una
	 * paralel ama en-yakına yapışma değil, hücre-üyeliği için taban bölme). */
	inline FIntPoint ComputeGustGridCell(const FVector& Position, float CellSize)
	{
		const float SafeCellSize = FMath::Max(1.0f, CellSize);
		return FIntPoint(FMath::FloorToInt(Position.X / SafeCellSize), FMath::FloorToInt(Position.Y / SafeCellSize));
	}

	/** [-1,1] salınım — TemperatureMath'in ComputeTimeOfDayModulation'ıyla aynı kosinüs-tabanlı
	 * desen, burada sinüs (faz kaydırması zaten hücreye göre keyfi, kosinüs/sinüs farkı önemsiz). */
	inline float ComputeGustFactor(float GameTimeSeconds, float PhaseOffset, float GustFrequency)
	{
		return FMath::Sin(GameTimeSeconds * GustFrequency + PhaseOffset);
	}

	/** Küresel rüzgar hızına yerel hamle katkısını ekler, negatife düşürmez. */
	inline float ComputeWindSpeedAt(float GlobalSpeed, float GustFactor, float GustAmplitude)
	{
		return FMath::Max(0.0f, GlobalSpeed + GustFactor * GustAmplitude);
	}

	/** Bir yapıya etkiyen rüzgar yükü (basit dinamik basınç yaklaşımı: 0.5*Cd*v²*Alan).
	 * Şu an yalnızca VERİ olarak hesaplanır — tüketen bir yapısal-bütünlük sistemi henüz yok
	 * (bkz. ADR, Sistem #9'un tüketilmeyen ısı-üretici verisiyle aynı desen). */
	inline float ComputeWindLoad(float WindSpeed, float ExposedArea, float DragCoefficient)
	{
		const float SafeSpeed = FMath::Max(0.0f, WindSpeed);
		return 0.5f * FMath::Max(0.0f, DragCoefficient) * FMath::Square(SafeSpeed) * FMath::Max(0.0f, ExposedArea);
	}

	/**
	 * Bir rüzgar türbininin bu andaki çıktısı: CutInSpeed altında 0, RatedSpeed üstünde/eşit
	 * RatedOutput sabit, arada KÜBİK artış (gerçek türbinlerde çıkış rüzgarın kinetik enerji
	 * akısıyla ~v³ orantılıdır). RatedSpeed<=CutInSpeed gibi hatalı veriye karşı bölme
	 * korumalıdır.
	 */
	inline float ComputeTurbineOutput(float WindSpeed, float CutInSpeed, float RatedSpeed, float RatedOutput)
	{
		if (WindSpeed < CutInSpeed)
		{
			return 0.0f;
		}
		if (WindSpeed >= RatedSpeed)
		{
			return FMath::Max(0.0f, RatedOutput);
		}
		const float Range = FMath::Max(KINDA_SMALL_NUMBER, RatedSpeed - CutInSpeed);
		const float Ratio = FMath::Clamp((WindSpeed - CutInSpeed) / Range, 0.0f, 1.0f);
		return FMath::Max(0.0f, RatedOutput) * FMath::Pow(Ratio, 3.0f);
	}
}
