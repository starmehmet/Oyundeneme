#include "Audio/AudioManager.h"
#include "Audio/AudioMath.h"
#include "Audio/AudioManagerSettings.h"
#include "SurvivalGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAudioManagerSettings* Settings = GetDefault<UAudioManagerSettings>();
	CategoryVolumes.Add(ESoundCategory::Master, Settings->DefaultMasterVolume);
	CategoryVolumes.Add(ESoundCategory::Music, Settings->DefaultMusicVolume);
	CategoryVolumes.Add(ESoundCategory::Ambient, Settings->DefaultAmbientVolume);
	CategoryVolumes.Add(ESoundCategory::SFX, Settings->DefaultSFXVolume);
	CategoryVolumes.Add(ESoundCategory::UI, Settings->DefaultUIVolume);
	CategoryVolumes.Add(ESoundCategory::Voice, Settings->DefaultVoiceVolume);

	UE_LOG(LogSurvivalAudio, Log, TEXT("AudioManager hazir"));
}

float UAudioManager::GetCategoryVolume(ESoundCategory Category) const
{
	const float* Found = CategoryVolumes.Find(Category);
	return Found ? *Found : 1.0f;
}

void UAudioManager::SetCategoryVolume(ESoundCategory Category, float Volume)
{
	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	CategoryVolumes.Add(Category, ClampedVolume);
	// Inceleme bulgusu: bu yayin olmadan, zaten calmakta olan uzun-omurlu bir ses (muzik)
	// kategori/master seviyesi degisince ANINDA tepki vermezdi — bir sonraki Play/Crossfade'e
	// kadar sessizce eski sesle calmaya devam ederdi.
	OnCategoryVolumeChanged.Broadcast(Category, ClampedVolume);
}

void UAudioManager::PlaySoundEffect(const FSoundInstance& SoundInstance)
{
	// Icerik henuz yok (Content/ tamamen sessiz) — bu, PlaySoundEffect'in gercekten test
	// edilebilecegi TEK yol: null Sound guvenli islensin, crash etmesin (ADR).
	if (!SoundInstance.Sound)
	{
		UE_LOG(LogSurvivalAudio, Verbose, TEXT("PlaySoundEffect: Sound==null, atlaniyor (kategori=%d)"),
			static_cast<uint8>(SoundInstance.Category));
		return;
	}

	const float MasterVolume = GetCategoryVolume(ESoundCategory::Master);
	const float CategoryVolume = GetCategoryVolume(SoundInstance.Category);
	const float EffectiveVolume = SurvivalAudio::ComputeEffectiveVolume(SoundInstance.Volume, CategoryVolume, MasterVolume);
	if (EffectiveVolume <= UE_KINDA_SMALL_NUMBER)
	{
		return;
	}

	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	if (!SoundInstance.bIs3D)
	{
		UGameplayStatics::PlaySound2D(World, SoundInstance.Sound, EffectiveVolume);
		return;
	}

	const UAudioManagerSettings* Settings = GetDefault<UAudioManagerSettings>();
	const APlayerController* PC = World->GetFirstPlayerController();
	const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	if (PlayerPawn)
	{
		const float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SoundInstance.Location);
		if (SurvivalAudio::ShouldCullSound(Distance, Settings->MaxAudibleDistance))
		{
			return;
		}
	}

	UGameplayStatics::PlaySoundAtLocation(World, SoundInstance.Sound, SoundInstance.Location, EffectiveVolume);
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin ----

namespace
{
	bool TryParseSoundCategory(const FString& Text, ESoundCategory& OutCategory)
	{
		static const TMap<FString, ESoundCategory> NameToCategory = {
			{TEXT("Master"), ESoundCategory::Master},
			{TEXT("Music"), ESoundCategory::Music},
			{TEXT("Ambient"), ESoundCategory::Ambient},
			{TEXT("SFX"), ESoundCategory::SFX},
			{TEXT("UI"), ESoundCategory::UI},
			{TEXT("Voice"), ESoundCategory::Voice},
		};
		for (const TPair<FString, ESoundCategory>& Pair : NameToCategory)
		{
			if (Pair.Key.Equals(Text, ESearchCase::IgnoreCase))
			{
				OutCategory = Pair.Value;
				return true;
			}
		}
		return false;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdAudioSetVolume(
		TEXT("audio_set_volume"),
		TEXT("Bir kategorinin ses seviyesini ayarlar: audio_set_volume [Master|Music|Ambient|SFX|UI|Voice] [0-1]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UAudioManager* Manager = GI ? GI->GetSubsystem<UAudioManager>() : nullptr;
				ESoundCategory Category;
				if (!Manager || Args.Num() < 2 || !TryParseSoundCategory(Args[0], Category))
				{
					UE_LOG(LogSurvivalAudio, Warning, TEXT("audio_set_volume: gecersiz kullanim"));
					return;
				}

				Manager->SetCategoryVolume(Category, FCString::Atof(*Args[1]));
				UE_LOG(LogSurvivalAudio, Log, TEXT("audio_set_volume: %s -> %.2f"), *Args[0], Manager->GetCategoryVolume(Category));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdAudioTestPlay(
		TEXT("audio_test_play"),
		TEXT("Sound==null bir FSoundInstance ile PlaySoundEffect'i cagirir (crash etmemesi gereken guvenli yol)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UAudioManager* Manager = GI ? GI->GetSubsystem<UAudioManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				FSoundInstance Instance;
				Instance.Category = ESoundCategory::SFX;
				Instance.bIs3D = false;
				Manager->PlaySoundEffect(Instance);
				UE_LOG(LogSurvivalAudio, Log, TEXT("audio_test_play: PlaySoundEffect(Sound=null) crash etmeden tamamlandi"));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdAudioDump(
		TEXT("audio_dump"),
		TEXT("Ses kategori seviyelerini logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const UAudioManager* Manager = GI ? GI->GetSubsystem<UAudioManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				UE_LOG(LogSurvivalAudio, Log,
					TEXT("Ses: master=%.2f music=%.2f ambient=%.2f sfx=%.2f ui=%.2f voice=%.2f"),
					Manager->GetCategoryVolume(ESoundCategory::Master),
					Manager->GetCategoryVolume(ESoundCategory::Music),
					Manager->GetCategoryVolume(ESoundCategory::Ambient),
					Manager->GetCategoryVolume(ESoundCategory::SFX),
					Manager->GetCategoryVolume(ESoundCategory::UI),
					Manager->GetCategoryVolume(ESoundCategory::Voice));
			}));
}
