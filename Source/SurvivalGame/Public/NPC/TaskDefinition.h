#pragma once

#include "CoreMinimal.h"
#include "TaskDefinition.generated.h"

class AProductionMachine;

/**
 * Sistem #16 — MIMARI.md'nin `FTaskDefinition`'ı, iki alan EKLENEREK: `WorkDuration` (görev
 * tamamlanma SÜRESİ — MIMARI'nin taslağında hiç yoktu ama "görev tamamlamasını takip et" DoD'si
 * bir süre kavramı OLMADAN anlamsız, bkz. ADR) ve `AvailableAfterGameTime` (başarısızlık sonrası
 * backoff için zamanlayıcı — `UTaskScheduler`'ın kendi iç muhasebesi, veri tanımının parçası
 * çünkü yeniden kuyruğa alınan görevle BİRLİKTE taşınması gerekiyor).
 */
USTRUCT(BlueprintType)
struct FTaskDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName TaskID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FText TaskName;

	/** Ayarlıysa hedef konum bu makinenin konumundan türetilir (Sistem #9 ile entegrasyon —
	 * NPC'yi bir üretim makinesine gönderme). Ayarlı DEĞİLSE `TargetLocation` kullanılır. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TWeakObjectPtr<AProductionMachine> TargetMachine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float Priority = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	int32 RequiredSkillLevel = 0;

	/** NPC hedefe varıp Working durumuna girdikten sonra görevin tamamlanması için gereken süre (sn). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float WorkDuration = 5.0f;

	/** `UTaskScheduler` iç muhasebesi — bu oyun-zamanından ÖNCE bu görev atanabilir DEĞİLDİR
	 * (başarısızlık sonrası backoff). Yeni kuyruğa eklenen görevlerde 0 (her zaman uygun). */
	UPROPERTY()
	double AvailableAfterGameTime = 0.0;
};
