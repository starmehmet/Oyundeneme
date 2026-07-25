#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio/SoundCategory.h"
#include "Audio/SoundInstance.h"
#include "AudioManager.generated.h"

/** Bir kategorinin ses seviyesi degistiginde yayinlanir — CALINMAKTA OLAN uzun-omurlu sesler
 * (ozellikle muzik) buna abone olup kendi bilesenlerinin sesini canli guncelleyebilir (bkz.
 * UMusicManager::HandleCategoryVolumeChanged). Tek-seferlik SFX'lerin abone olmasina GEREK
 * YOK — PlaySoundEffect her cagrida CategoryVolumes'u zaten TAZE okur. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCategoryVolumeChanged, ESoundCategory, Category, float, NewVolume);

/**
 * Sistem #19 — Ses efektlerinin tek sahibi: kategori/master karisimini uygular (bkz.
 * AudioMath.h ComputeEffectiveVolume), 3D sesleri mesafeye gore budar (ShouldCullSound), ve
 * gercek motor API'lerine (UGameplayStatics::PlaySoundAtLocation/PlaySound2D) INCE bir
 * sarmalayici olarak devreder — proje icinde HENUZ hicbir USoundWave/USoundCue varligi
 * olmadigindan (Content/ tamamen sessiz), `Sound==nullptr` GUVENLI islenir (loglanir, crash
 * etmez) ve bu, PIE/konsol dogrulamasinin gercekte test edebilecegi tek yoldur.
 */
UCLASS()
class SURVIVALGAME_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Sound==nullptr ise (henuz icerik yok) guvenli sekilde loglar ve hicbir sey calmaz. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySoundEffect(const FSoundInstance& SoundInstance);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCategoryVolume(ESoundCategory Category, float Volume);

	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetCategoryVolume(ESoundCategory Category) const;

	UPROPERTY(BlueprintAssignable, Category = "Audio")
	FOnCategoryVolumeChanged OnCategoryVolumeChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	TMap<ESoundCategory, float> CategoryVolumes;
};
