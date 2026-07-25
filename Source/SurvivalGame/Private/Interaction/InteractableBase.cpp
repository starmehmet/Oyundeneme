#include "Interaction/InteractableBase.h"
#include "SurvivalGame.h"
#include "Components/StaticMeshComponent.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// ECC_Interaction varsayilani Ignore — etkilesilebilirler ACIKCA Block ile kaydolur
	Mesh->SetCollisionResponseToChannel(ECC_Interaction, ECR_Block);

	InteractionPrompt = FText::FromString(TEXT("Etkiles"));
}

bool AInteractableBase::CanInteract_Implementation(APlayerCharacter* Interactor) const
{
	return bInteractionEnabled;
}

FText AInteractableBase::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

float AInteractableBase::GetInteractionDistance_Implementation() const
{
	return InteractionDistance;
}

void AInteractableBase::OnInteract_Implementation(APlayerCharacter* Interactor)
{
	++InteractionCount;
	UE_LOG(LogSurvival, Log, TEXT("InteractableBase: %s etkilesildi (toplam %d)"),
		*GetName(), InteractionCount);
}
