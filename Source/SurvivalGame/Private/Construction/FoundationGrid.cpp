#include "Construction/FoundationGrid.h"
#include "Construction/ConstructionMath.h"
#include "Construction/ConstructionSettings.h"
#include "Construction/BuildingBase.h"
#include "SurvivalGame.h"

void UFoundationGrid::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UConstructionSettings* Settings = GetDefault<UConstructionSettings>();
	GridSize = Settings->GridSize;

	UE_LOG(LogSurvival, Log, TEXT("FoundationGrid hazir: GridSize=%.0f"), GridSize);
}

FIntPoint UFoundationGrid::WorldToGridCoord(const FVector& WorldPosition) const
{
	return SurvivalConstruction::WorldToGridCoord(WorldPosition, GridSize);
}

FVector UFoundationGrid::GridCoordToWorld(const FIntPoint& Coord, float ZHeight) const
{
	return SurvivalConstruction::GridCoordToWorld(Coord, GridSize, ZHeight);
}

bool UFoundationGrid::RegisterBuilding(ABuildingBase* Building, const FIntPoint& Coord)
{
	if (!Building || OccupiedCells.Contains(Coord))
	{
		return false;
	}

	OccupiedCells.Add(Coord);
	BuildingMap.Add(Coord, Building);
	return true;
}

void UFoundationGrid::UnregisterBuilding(const FIntPoint& Coord)
{
	OccupiedCells.Remove(Coord);
	BuildingMap.Remove(Coord);
}

ABuildingBase* UFoundationGrid::GetBuildingAt(const FIntPoint& Coord) const
{
	const TObjectPtr<ABuildingBase>* Found = BuildingMap.Find(Coord);
	return Found ? Found->Get() : nullptr;
}
