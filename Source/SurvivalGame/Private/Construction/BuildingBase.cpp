#include "Construction/BuildingBase.h"
#include "Construction/FoundationGrid.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ABuildingBase::ABuildingBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

void ABuildingBase::BeginConstruction(FName InBuildingID, const FBuildingDefinition& InDefinition, const FIntPoint& InGridCoord)
{
	BuildingID = InBuildingID;
	GridCoord = InGridCoord;

	if (UStaticMesh* LoadedMesh = InDefinition.Mesh.LoadSynchronous())
	{
		Mesh->SetStaticMesh(LoadedMesh);
	}

	// Anlik tamamlanma (bkz. sinif yorumu) — Definition.ConstructionTime henuz tuketilmiyor.
	ConstructionProgress = 1.0f;
	bIsConstructed = true;
}

void ABuildingBase::Demolish()
{
	if (UWorld* World = GetWorld())
	{
		if (UFoundationGrid* Grid = World->GetSubsystem<UFoundationGrid>())
		{
			Grid->UnregisterBuilding(GridCoord);
		}
	}

	Destroy();
}
