#include "Logistics/StorageNode.h"
#include "Logistics/LogisticsMath.h"
#include "Logistics/LogisticsNetwork.h"
#include "Inventory/InventoryComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"

AStorageNode::AStorageNode()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Storage = CreateDefaultSubobject<UInventoryComponent>(TEXT("Storage"));
}

void AStorageNode::BeginPlay()
{
	Super::BeginPlay();

	if (Storage)
	{
		for (const TPair<FName, int32>& Pair : StartingItems)
		{
			Storage->AddItem(Pair.Key, Pair.Value);
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULogisticsNetwork* Network = GI->GetSubsystem<ULogisticsNetwork>())
		{
			Network->RegisterNode(this);
		}
	}
}

void AStorageNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULogisticsNetwork* Network = GI->GetSubsystem<ULogisticsNetwork>())
		{
			Network->UnregisterNode(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool AStorageNode::CanAcceptItem(FName ItemID) const
{
	return SurvivalLogistics::CanNodeTypeAccept(NodeType);
}
