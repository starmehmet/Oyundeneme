#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio/SoundCategory.h"
#include "MusicManager.generated.h"

/**
 * Sistem #19 — Muzik: `UAudioManager`'dan AYRI subsystem (MIMARI'de de ayri sinif) — muzigin
 * kendine ozgu "her an TEK bir aktif parca + gecis (crossfade)" durumu, SFX'in "her cagri
 * bagimsiz, durum yok" modelinden temelde farkli. Gercek gecis, motorun kendi
 * `UAudioComponent::FadeIn`/`FadeOut` fader'i ile yapilir (elle hesaplanan bir "gecis-ilerlemesi"
 * DEGIL — motorun ses is parcacigindaki (audio thread) fader'i taklit etmek gereksiz/riskli
 * olurdu). `FadeOut` engine kaynagindan (AudioComponent.cpp) DOGRULANDI: bilesenin kendisini
 * YOK ETMEZ, yalnizca sesi susturur — bu yuzden eski bilesenin `Stop`+`DestroyComponent`
 * cagrisi burada bir zamanlayici ile ELLE yapilir.
 */
UCLASS()
class SURVIVALGAME_API UMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Mevcut parcayi aninda durdurup yenisini aninda baslatir (gecis yok). */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMusic(class USoundBase* NewMusic);

	/** Mevcut parcayi Duration suresinde soldurup yeni parcayi ayni surede yukseltir.
	 * Duration<=0 verilirse AudioManagerSettings::DefaultMusicCrossfadeDuration kullanilir. */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void CrossfadeMusic(class USoundBase* NewMusic, float Duration = -1.0f);

	UFUNCTION(BlueprintPure, Category = "Audio")
	class USoundBase* GetCurrentMusic() const { return CurrentMusic; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class UAudioComponent> CurrentMusicComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class USoundBase> CurrentMusic;

	float GetEffectiveMusicVolume() const;

	/** UAudioManager::OnCategoryVolumeChanged'e abone — Music/Master degisince CALMAKTA olan
	 * parcanin sesini canli gunceller (inceleme bulgusu). */
	UFUNCTION()
	void HandleCategoryVolumeChanged(ESoundCategory Category, float NewVolume);
};
