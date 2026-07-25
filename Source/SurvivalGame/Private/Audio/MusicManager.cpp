#include "Audio/MusicManager.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioMath.h"
#include "Audio/AudioManagerSettings.h"
#include "SurvivalGame.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"

void UMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UAudioManager* AudioManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAudioManager>() : nullptr)
	{
		AudioManager->OnCategoryVolumeChanged.AddDynamic(this, &UMusicManager::HandleCategoryVolumeChanged);
	}
	UE_LOG(LogSurvivalAudio, Log, TEXT("MusicManager hazir"));
}

void UMusicManager::HandleCategoryVolumeChanged(ESoundCategory Category, float NewVolume)
{
	if (Category != ESoundCategory::Music && Category != ESoundCategory::Master)
	{
		return;
	}
	if (IsValid(CurrentMusicComponent))
	{
		CurrentMusicComponent->SetVolumeMultiplier(GetEffectiveMusicVolume());
	}
}

float UMusicManager::GetEffectiveMusicVolume() const
{
	const UGameInstance* GI = GetGameInstance();
	const UAudioManager* AudioManager = GI ? GI->GetSubsystem<UAudioManager>() : nullptr;
	if (!AudioManager)
	{
		return 1.0f;
	}
	return SurvivalAudio::ComputeEffectiveVolume(1.0f,
		AudioManager->GetCategoryVolume(ESoundCategory::Music),
		AudioManager->GetCategoryVolume(ESoundCategory::Master));
}

void UMusicManager::PlayMusic(USoundBase* NewMusic)
{
	// Icerik henuz yok (Content/ tamamen sessiz) — null guvenli islenir (ADR, AudioManager.cpp ile ayni gerekce).
	if (!NewMusic)
	{
		UE_LOG(LogSurvivalAudio, Verbose, TEXT("PlayMusic: NewMusic==null, atlaniyor"));
		return;
	}

	// IsValid() (bare non-null DEGIL) — bAutoDestroy=false gecilse bile (asagida) baska bir
	// yoldan (ornegin seviye gecisi) motor tarafinda yok edilmis olabilir (inceleme bulgusu).
	if (IsValid(CurrentMusicComponent))
	{
		CurrentMusicComponent->Stop();
		CurrentMusicComponent->DestroyComponent();
	}
	CurrentMusicComponent = nullptr;

	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// bAutoDestroy=false: bu bilesenin yasam dongusunu ELLE (Stop+DestroyComponent) yonetiyoruz;
	// motorun calma bitince kendiliginden yok etmesi CurrentMusicComponent'i bizim haberimiz
	// olmadan gecersiz kilardi (inceleme bulgusu).
	CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, NewMusic, GetEffectiveMusicVolume(),
		/*PitchMultiplier=*/1.0f, /*StartTime=*/0.0f, /*ConcurrencySettings=*/nullptr,
		/*bPersistAcrossLevelTransition=*/false, /*bAutoDestroy=*/false);
	CurrentMusic = CurrentMusicComponent ? NewMusic : nullptr;

	if (CurrentMusicComponent)
	{
		UE_LOG(LogSurvivalAudio, Log, TEXT("PlayMusic: %s"), *NewMusic->GetName());
	}
	else
	{
		UE_LOG(LogSurvivalAudio, Warning, TEXT("PlayMusic: %s baslatilamadi (SpawnSound2D null dondu)"), *NewMusic->GetName());
	}
}

void UMusicManager::CrossfadeMusic(USoundBase* NewMusic, float Duration)
{
	if (!NewMusic)
	{
		UE_LOG(LogSurvivalAudio, Verbose, TEXT("CrossfadeMusic: NewMusic==null, atlaniyor"));
		return;
	}

	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const UAudioManagerSettings* Settings = GetDefault<UAudioManagerSettings>();
	const float FadeDuration = Duration > 0.0f ? Duration : Settings->DefaultMusicCrossfadeDuration;

	// FadeOut (engine kaynagindan dogrulandi, AudioComponent.cpp) bileseni YOK ETMEZ, yalnizca
	// susturur — bu yuzden temizlik burada ELLE bir zamanlayicidir. IsValid() (bare non-null
	// DEGIL) — bAutoDestroy=false gecilse bile baska bir yoldan yok edilmis olabilir.
	if (IsValid(CurrentMusicComponent))
	{
		UAudioComponent* OldComponent = CurrentMusicComponent;
		OldComponent->FadeOut(FadeDuration, 0.0f);
		const TWeakObjectPtr<UAudioComponent> WeakOld(OldComponent);
		FTimerHandle CleanupHandle;
		World->GetTimerManager().SetTimer(CleanupHandle, FTimerDelegate::CreateLambda([WeakOld]()
			{
				if (UAudioComponent* Comp = WeakOld.Get())
				{
					Comp->Stop();
					Comp->DestroyComponent();
				}
			}), FMath::Max(FadeDuration, 0.01f), false);
	}

	// CreateSound2D (SpawnSound2D'nin AKSINE) hemen calmaz — FadeIn kendi PlayInternal'ini
	// cagirir; SpawnSound2D+FadeIn yapilsaydi bir kare tam sesle calip SONRA solmaya baslardi.
	// bAutoDestroy=false: PlayMusic ile ayni gerekce (yasam dongusu burada elle yonetiliyor).
	UAudioComponent* NewComponent = UGameplayStatics::CreateSound2D(World, NewMusic, 1.0f,
		/*PitchMultiplier=*/1.0f, /*StartTime=*/0.0f, /*ConcurrencySettings=*/nullptr,
		/*bPersistAcrossLevelTransition=*/false, /*bAutoDestroy=*/false);
	if (NewComponent)
	{
		NewComponent->FadeIn(FadeDuration, GetEffectiveMusicVolume());
	}
	CurrentMusicComponent = NewComponent;
	CurrentMusic = NewComponent ? NewMusic : nullptr;

	if (NewComponent)
	{
		UE_LOG(LogSurvivalAudio, Log, TEXT("CrossfadeMusic: %s (sure=%.1fsn)"), *NewMusic->GetName(), FadeDuration);
	}
	else
	{
		UE_LOG(LogSurvivalAudio, Warning, TEXT("CrossfadeMusic: %s baslatilamadi (CreateSound2D null dondu)"), *NewMusic->GetName());
	}
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdMusicTestPlay(
		TEXT("music_test_play"),
		TEXT("NewMusic==null ile PlayMusic+CrossfadeMusic'i cagirir (crash etmemesi gereken guvenli yol)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UMusicManager* Manager = GI ? GI->GetSubsystem<UMusicManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				Manager->PlayMusic(nullptr);
				Manager->CrossfadeMusic(nullptr, 1.0f);
				UE_LOG(LogSurvivalAudio, Log, TEXT("music_test_play: PlayMusic/CrossfadeMusic(null) crash etmeden tamamlandi"));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdMusicDump(
		TEXT("music_dump"),
		TEXT("Muzik yoneticisinin durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const UMusicManager* Manager = GI ? GI->GetSubsystem<UMusicManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				const USoundBase* Current = Manager->GetCurrentMusic();
				UE_LOG(LogSurvivalAudio, Log, TEXT("Muzik: mevcut-parca=%s"),
					Current ? *Current->GetName() : TEXT("(yok)"));
			}));
}
