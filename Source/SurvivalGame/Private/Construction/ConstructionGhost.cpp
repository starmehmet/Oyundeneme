#include "Construction/ConstructionGhost.h"
#include "Components/StaticMeshComponent.h"

AConstructionGhost::AConstructionGhost()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AConstructionGhost::UpdatePreview(UStaticMesh* PreviewMesh, const FVector& WorldLocation, const FIntPoint& InGridCoord, bool bValid)
{
	if (PreviewMesh && Mesh->GetStaticMesh() != PreviewMesh)
	{
		Mesh->SetStaticMesh(PreviewMesh);
	}
	SetActorLocation(WorldLocation);
	GridCoord = InGridCoord;
	bIsValid = bValid;
}
