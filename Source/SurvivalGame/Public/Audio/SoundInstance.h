#pragma once

#include "CoreMinimal.h"
#include "Audio/SoundCategory.h"
#include "SoundInstance.generated.h"

/**
 * Sistem #19 — Tek bir ses efekti calma istegi (MIMARI ile ayni alanlar). `Sound` alani
 * MIMARI'nin onerdigi `USoundWave*` yerine bilerek `USoundBase*` — hem `USoundWave` hem
 * `USoundCue` bundan turer ve `UGameplayStatics::PlaySoundAtLocation`/`PlaySound2D` zaten
 * `USoundBase*` bekler (ADR).
 */
USTRUCT(BlueprintType)
struct FSoundInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<class USoundBase> Sound = nullptr;

	/** Yalnizca bIs3D=true iken kullanilir. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	FVector Location = FVector::ZeroVector;

	/** Kategori/master ile carpimsal karisimdan ONCEKI taban seviye [0,1]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	ESoundCategory Category = ESoundCategory::SFX;

	/** true: konumlandirilmis (mesafe budamali) 3D ses. false: 2D/UI sesi (budanmaz). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bIs3D = true;
};
