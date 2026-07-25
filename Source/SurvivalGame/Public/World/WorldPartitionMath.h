#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #18 — Dünya Bölümlendirme: saf, durum tutmayan hücre-eşleme ve histerezisli
 * yükle/boşalt karar matematiği. Yan etkisiz; birim testleri
 * Private/Tests/WorldPartitionMathTests.cpp.
 */
namespace SurvivalWorldPartition
{
	/** Dünya konumunu bir hücre koordinatına böler — Construction/Wind'in grid-hücre
	 * fonksiyonlarıyla aynı taban-bölme deseni. */
	inline FIntPoint GetCellForPosition(const FVector& Position, float CellSize)
	{
		const float SafeCellSize = FMath::Max(1.0f, CellSize);
		return FIntPoint(FMath::FloorToInt(Position.X / SafeCellSize), FMath::FloorToInt(Position.Y / SafeCellSize));
	}

	/** Bir hücrenin dünya-uzayı merkezi (uzaklık hesabı için). */
	inline FVector GetCellCenter(const FIntPoint& CellCoord, float CellSize)
	{
		const float HalfCell = CellSize * 0.5f;
		return FVector(CellCoord.X * CellSize + HalfCell, CellCoord.Y * CellSize + HalfCell, 0.0f);
	}

	/**
	 * Histerezisli yükle/boşalt kararı — `LoadRadius < UnloadRadius` OLMALI: bir hücre
	 * `LoadRadius` içine girince yüklenir, yalnızca `UnloadRadius`'u AŞINCA boşaltılır.
	 * Aradaki "kararlı bölge" sınırda duran bir oyuncunun hücreyi sürekli yükleyip
	 * boşaltmasını (titreşim) önler.
	 */
	inline bool ShouldCellBeLoaded(float DistanceToPlayer, float LoadRadius)
	{
		return DistanceToPlayer <= LoadRadius;
	}

	inline bool ShouldCellBeUnloaded(float DistanceToPlayer, float UnloadRadius)
	{
		return DistanceToPlayer > UnloadRadius;
	}
}
