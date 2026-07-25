#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #7 — İnşaat: saf, durum tutmayan ızgara koordinat dönüşümü.
 * Yan etkisiz; birim testleri Private/Tests/ConstructionMathTests.cpp.
 */
namespace SurvivalConstruction
{
	/** Dünya konumunu en yakın ızgara hücresine yuvarlar (X/Y düzleminde). GridSize<=0 ise (0,0) döner. */
	inline FIntPoint WorldToGridCoord(const FVector& WorldPosition, float GridSize)
	{
		if (GridSize <= 0.0f)
		{
			return FIntPoint(0, 0);
		}
		return FIntPoint(
			FMath::RoundToInt(WorldPosition.X / GridSize),
			FMath::RoundToInt(WorldPosition.Y / GridSize));
	}

	/** Bir ızgara hücresinin dünya konumunu döner (hücre merkezi). ZHeight çağıran tarafça belirlenir. */
	inline FVector GridCoordToWorld(const FIntPoint& Coord, float GridSize, float ZHeight = 0.0f)
	{
		return FVector(static_cast<float>(Coord.X) * GridSize, static_cast<float>(Coord.Y) * GridSize, ZHeight);
	}
}
