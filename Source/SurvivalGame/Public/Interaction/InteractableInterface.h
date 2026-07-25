#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class APlayerCharacter;

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Sistem #3 — Etkileşilebilir her şeyin sözleşmesi. C++ VE Blueprint'ten uygulanabilir
 * (kapılar/makineler C++, içerik nesneleri BP) — bu yüzden BlueprintNativeEvent;
 * çağıran taraf HER ZAMAN Execute_* sarmalayıcılarını kullanmalıdır, doğrudan
 * sanal çağrı BP uygulamalarını atlar.
 */
class IInteractableInterface
{
	GENERATED_BODY()

public:
	/** Oyuncu şu an bu nesneyle etkileşebilir mi (kilit, durum, yetki)? */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(APlayerCharacter* Interactor) const;

	/** UI'da gösterilecek istem metni (örn. "Aç", "Topla"). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;

	/** Pawn'dan maksimum etkileşim mesafesi (UU). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	float GetInteractionDistance() const;

	/** Etkileşim gerçekleşti — hedef nesne davranışını burada uygular. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(APlayerCharacter* Interactor);
};
