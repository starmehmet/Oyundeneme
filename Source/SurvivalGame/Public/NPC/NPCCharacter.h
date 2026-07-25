#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

class UNPCBrain;
class UHealthComponent;

/**
 * Sistem #15 — MIMARI.md'nin `ANPCCharacter : ACharacter`'ı. `Morale`/`Fatigue`/`CurrentState`
 * MIMARI'de bu sınıfın ÜZERİNDE tutuluyordu — burada bilerek `UNPCBrain` bileşenine taşındı
 * (Sistem #2'nin `APlayerCharacter`+ayrı bileşenler mimarisiyle TUTARLI: "davranış/durum
 * bileşende, aktör yalnızca kabuk", bkz. ADR). `Health` alanı MIMARI'nin ayrı bir `float`
 * taslağı YERİNE Sistem #12'nin `UHealthComponent`'ini YENİDEN KULLANIR.
 *
 * `AIControllerClass`/`AutoPossessAI` motorun VARSAYILAN `AAIController`'ını kullanır — özel
 * bir AI controller alt sınıfı GEREKMEDİ (`MoveToLocation` zaten taban sınıfta var).
 */
UCLASS()
class SURVIVALGAME_API ANPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCCharacter();

	UFUNCTION(BlueprintPure, Category = "NPC")
	UNPCBrain* GetBrain() const { return BrainComp; }

	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const { return HealthComp; }

private:
	UPROPERTY(VisibleAnywhere, Category = "NPC")
	TObjectPtr<UNPCBrain> BrainComp;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComp;
};
