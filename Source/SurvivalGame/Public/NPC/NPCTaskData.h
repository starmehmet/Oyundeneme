#pragma once

#include "CoreMinimal.h"
#include "NPCTaskData.generated.h"

/**
 * Sistem #15 — MIMARI.md'nin polymorphic `UNPCTask : UObject` (+ `Execute`/`IsComplete`
 * virtual'ları) hiyerarşisi YERİNE minimal bir veri struct'ı. Gerçek görev KUYRUĞU/ÖNCELİK/
 * beceri-eşleşmesi Sistem #16'nın (`UTaskScheduler`, henüz YAZILMADI) işi — bu sistem yalnızca
 * "şu an bana atanmış TEK bir görev var mı, nerede" sorusuna cevap veriyor (bkz. ADR).
 */
USTRUCT(BlueprintType)
struct FNPCTaskData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName TaskID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	float Priority = 0.0f;

	/** Sistem #16 — NPC hedefe varıp Working durumuna girdikten sonra görevin tamamlanması
	 * için gereken süre (sn). 0 veya altı = hiç tamamlanmaz (süresiz iş, ör. devriye). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	float WorkDuration = 0.0f;
};
