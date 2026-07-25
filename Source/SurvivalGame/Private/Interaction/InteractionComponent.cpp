#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableInterface.h"
#include "Interaction/InteractionMath.h"
#include "Player/PlayerCharacter.h"
#include "SurvivalGame.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Post-increment: ilk tick kare 0'i degerlendirir (InteractionMath sozlesmesi
	// ve birim testi kare-0'da trace bekler; pre-increment ilk trace'i 4. tick'e atiyordu)
	if (SurvivalInteraction::ShouldTraceThisFrame(FrameCounter++, TraceInterval))
	{
		UpdateFocus();
	}
}

void UInteractionComponent::UpdateFocus()
{
	AActor* Best = QueryBestInteractable();
	// IsStale(): odaktaki aktor yok edildiyse Get() null doner ve Best de null olur —
	// "degisim yok" gibi gorunup UI'da olu nesnenin prompt'u asili kalirdi (inceleme bulgusu).
	// SetFocus(nullptr) zayif isaretciyi acikca-null yapar, kosul kendiliginden susar.
	if (Best != FocusedInteractable.Get() || FocusedInteractable.IsStale())
	{
		const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
		if (SurvivalInteraction::ShouldUpdatePrompt(Now, LastPromptUpdateTime, PromptMinInterval, true))
		{
			LastPromptUpdateTime = Now;
			SetFocus(Best);
		}
	}
}

AActor* UInteractionComponent::QueryBestInteractable() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	UWorld* World = GetWorld();
	if (!OwnerPawn || !World)
	{
		return nullptr;
	}

	// Bakis noktasi: kontrolor kamerasi (ucuncu sahis follow kamera dahil)
	FVector ViewLocation;
	FRotator ViewRotation;
	if (const AController* OwnerController = OwnerPawn->GetController())
	{
		OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		OwnerPawn->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionTrace), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(OwnerPawn);

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit, ViewLocation, TraceEnd, FQuat::Identity, ECC_Interaction,
		FCollisionShape::MakeSphere(TraceRadius), Params);
	if (!bHit)
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		return nullptr;
	}

	// Menzil pawn'dan olculur; CanInteract hedefin kendi kurali
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwner());
	const float MaxDist = IInteractableInterface::Execute_GetInteractionDistance(HitActor);
	if (!SurvivalInteraction::IsWithinRange(OwnerPawn->GetActorLocation(), HitActor->GetActorLocation(), MaxDist))
	{
		return nullptr;
	}
	if (!IInteractableInterface::Execute_CanInteract(HitActor, PlayerChar))
	{
		return nullptr;
	}
	return HitActor;
}

void UInteractionComponent::SetFocus(AActor* NewFocus)
{
	FocusedInteractable = NewFocus;
	const FText Prompt = NewFocus
		? IInteractableInterface::Execute_GetInteractionPrompt(NewFocus)
		: FText::GetEmpty();
	OnFocusedInteractableChanged.Broadcast(NewFocus, Prompt);
}

bool UInteractionComponent::TryInteract()
{
	AActor* Target = FocusedInteractable.Get();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!Target || !OwnerPawn)
	{
		return false;
	}

	// Odak onbellekli (4 karede bir guncellenir) — yurutme aninda yeniden dogrula
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(OwnerPawn);
	const float MaxDist = IInteractableInterface::Execute_GetInteractionDistance(Target);
	if (!SurvivalInteraction::IsWithinRange(OwnerPawn->GetActorLocation(), Target->GetActorLocation(), MaxDist)
		|| !IInteractableInterface::Execute_CanInteract(Target, PlayerChar))
	{
		return false;
	}

	IInteractableInterface::Execute_OnInteract(Target, PlayerChar);
	UE_LOG(LogSurvival, Log, TEXT("Etkilesim: %s"), *Target->GetName());
	return true;
}
