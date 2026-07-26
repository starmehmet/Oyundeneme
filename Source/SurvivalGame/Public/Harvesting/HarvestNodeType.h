#pragma once

#include "CoreMinimal.h"
#include "HarvestNodeType.generated.h"

/**
 * Sistem #29 — v1'de davranisi DEGISTIRMEZ, yalnizca prompt/ikon secimi ve gelecekteki
 * tur-bazli dallanma icin veri kancasi (BuildingDefinition::ConstructionTime'daki "veri
 * duruyor, kodda dallanmiyor" durust-kapsam disipliniyle ayni, bkz. Docs/MIMARI.md #29).
 */
UENUM(BlueprintType)
enum class EHarvestNodeType : uint8
{
	Generic,
	Tree,
	Rock
};
