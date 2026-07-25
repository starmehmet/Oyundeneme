#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFocusedInteractableChanged,
	AActor*, NewFocus, const FText&, Prompt);

/**
 * Sistem #3 — Etkileşim sorgusu ve yürütmesi. APlayerCharacter'a takılır.
 * Her TraceInterval karede bir kamera bakış yönünde küre süpürmesi yapar
 * (ECC_Interaction kanalı — varsayılan yanıt Ignore, yalnızca etkileşilebilirler
 * Block ile kaydolur), odaktaki aktörü önbelleğe alır, değişimde debounce'lu
 * delegate yayınlar. UI (Sistem #20) bu delegate'e abone olur.
 *
 * Menzil kuralı: prompt VE yürütme, kameradan değil PAWN konumundan ölçülür
 * (InteractionMath::IsWithinRange) — üçüncü şahıs kamera-mesafesi yanıltıcıdır.
 */
UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** IA_Interact tetiklenince kontrolörden çağrılır. Başarıyla etkileşildiyse true. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusedInteractableChanged OnFocusedInteractableChanged;

private:
	void UpdateFocus();
	AActor* QueryBestInteractable() const;
	void SetFocus(AActor* NewFocus);

	/** Kaç karede bir trace atılır (1 = her kare). */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "1", ClampMax = "30"))
	int32 TraceInterval = 4;

	/** Bakış yönünde süpürme uzunluğu (UU) — kamera pawn'ın gerisinde olduğundan cömert. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "50.0"))
	float TraceDistance = 700.0f;

	/** Süpürme küresi yarıçapı (UU) — küçük hedefleri ıskalamayı önler. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "1.0"))
	float TraceRadius = 30.0f;

	/** Prompt guncellemeleri arasi asgari sure (sn) — UI titremesini onler. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float PromptMinInterval = 0.2f;

	TWeakObjectPtr<AActor> FocusedInteractable;
	uint64 FrameCounter = 0;
	double LastPromptUpdateTime = -1.0;
};
