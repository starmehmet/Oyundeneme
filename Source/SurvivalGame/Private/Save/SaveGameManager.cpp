#include "Save/SaveGameManager.h"
#include "Save/SaveDataSerializer.h"
#include "Save/SaveGameManagerSettings.h"
#include "Time/TimeKeeper.h"
#include "Player/PlayerCharacter.h"
#include "Player/HealthComponent.h"
#include "Inventory/InventoryComponent.h"
#include "SurvivalGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

void USaveGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvival, Log, TEXT("SaveGameManager hazir"));
}

bool USaveGameManager::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* USaveGameManager::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void USaveGameManager::Tick(float DeltaTime)
{
	TotalPlayTimeSeconds += DeltaTime;

	const USaveGameManagerSettings* Settings = GetDefault<USaveGameManagerSettings>();
	if (Settings->AutosaveInterval <= 0.0f)
	{
		return; // otomatik kaydetme kapali
	}

	TimeSinceLastAutosave += DeltaTime;
	if (TimeSinceLastAutosave >= Settings->AutosaveInterval)
	{
		// Inceleme bulgusu (kucuk): sayac yalnizca BASARILI bir kaydetmede sifirlanir —
		// basarisiz bir otomatik-kaydetme (ornegin disk yazma hatasi) bir sonraki TAM
		// AutosaveInterval'a kadar degil, bir sonraki Tick'te tekrar denenir.
		if (SaveGame(Settings->AutosaveSlotName))
		{
			TimeSinceLastAutosave = 0.0f;
		}
	}
}

APlayerCharacter* USaveGameManager::FindPlayerCharacter() const
{
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	return PC ? Cast<APlayerCharacter>(PC->GetPawn()) : nullptr;
}

FGameSaveData USaveGameManager::BuildSaveDataFromLiveSystems() const
{
	FGameSaveData Data;
	Data.SaveVersion = 1;
	Data.TotalPlayTimeSeconds = TotalPlayTimeSeconds;

	const UGameInstance* GI = GetGameInstance();
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	Data.TotalGameSeconds = Clock ? Clock->GetTotalGameSeconds() : 0.0;

	if (const APlayerCharacter* Player = FindPlayerCharacter())
	{
		Data.PlayerPosition = Player->GetActorLocation();
		Data.PlayerBodyTemperature = Player->GetBodyTemperature();
		if (const UHealthComponent* Health = Player->GetHealthComponent())
		{
			Data.PlayerHealth = Health->GetCurrentHealth();
		}
		if (const UInventoryComponent* Inventory = Player->GetInventoryComponent())
		{
			Data.PlayerInventory = Inventory->GetSlots();
		}
	}
	return Data;
}

bool USaveGameManager::ApplySaveDataToLiveSystems(const FGameSaveData& Data)
{
	// Inceleme bulgusu: oyuncu piyonu ONCE bulunur — hicbir canli sistem degistirilmeden
	// once erken cikilir. Aksi halde TimeKeeper zaten guncellenmis olurken fonksiyon false
	// donerdi; cagiran taraf (header'daki "false dönerse HİÇBİR ŞEYİ değiştirmez" sozlesmesine
	// gore) bunu "hicbir sey degismedi" sanirdi — yari-yuklu, tutarsiz bir durum.
	APlayerCharacter* Player = FindPlayerCharacter();
	if (!Player)
	{
		UE_LOG(LogSurvival, Warning, TEXT("ApplySaveDataToLiveSystems: oyuncu piyonu bulunamadi — hicbir sey uygulanmadi"));
		return false;
	}

	TotalPlayTimeSeconds = Data.TotalPlayTimeSeconds;

	UGameInstance* GI = GetGameInstance();
	if (UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr)
	{
		Clock->SetTotalGameSeconds(Data.TotalGameSeconds);
	}

	// Inceleme bulgusu: SetActorLocation, TeleportSucceeded->OnTeleported zincirini tetiklemez —
	// UCharacterMovementComponent'in Velocity/MovementMode'u olum anindaki degerde kalir (oyuncu
	// dusuyorsa/yuruyor-durumdaysa yeni konumda ayni hizla kaymaya/dusmeye devam eder) VE hedefte
	// (kayit sonrasi insa edilmis olabilecek) geometriye gomulme kontrolu yapilmaz. TeleportTo
	// ikisini de motor kaynagindan dogrulanan sekilde cozer: TeleportPhysics + FindTeleportSpot.
	Player->TeleportTo(Data.PlayerPosition, Player->GetActorRotation());
	Player->SetBodyTemperature(Data.PlayerBodyTemperature);
	if (UHealthComponent* Health = Player->GetHealthComponent())
	{
		Health->SetCurrentHealthForLoad(Data.PlayerHealth);
	}
	if (UInventoryComponent* Inventory = Player->GetInventoryComponent())
	{
		Inventory->RestoreSlots(Data.PlayerInventory);
	}
	return true;
}

bool USaveGameManager::SaveGame(const FString& SlotName)
{
	USurvivalSaveGame* SaveObject = NewObject<USurvivalSaveGame>();
	SaveObject->Payload = BuildSaveDataFromLiveSystems();

	const TArray<uint8> Compressed = SurvivalSave::CompressSaveObject(SaveObject);
	if (Compressed.Num() == 0)
	{
		UE_LOG(LogSurvival, Error, TEXT("SaveGame('%s'): sikistirma basarisiz"), *SlotName);
		return false;
	}

	if (!UGameplayStatics::SaveDataToSlot(Compressed, SlotName, 0))
	{
		UE_LOG(LogSurvival, Error, TEXT("SaveGame('%s'): diske yazma basarisiz"), *SlotName);
		return false;
	}

	UE_LOG(LogSurvival, Log, TEXT("SaveGame('%s'): basarili (%d bayt sikistirilmis)"), *SlotName, Compressed.Num());
	LastSavedSlotName = SlotName;
	return true;
}

bool USaveGameManager::LoadGame(const FString& SlotName)
{
	TArray<uint8> Compressed;
	if (!UGameplayStatics::LoadDataFromSlot(Compressed, SlotName, 0) || Compressed.Num() == 0)
	{
		UE_LOG(LogSurvival, Warning, TEXT("LoadGame('%s'): yuva bulunamadi/okunamadi"), *SlotName);
		return false;
	}

	USaveGame* LoadedBase = SurvivalSave::DecompressSaveObject(Compressed);
	USurvivalSaveGame* Loaded = Cast<USurvivalSaveGame>(LoadedBase);
	if (!Loaded)
	{
		UE_LOG(LogSurvival, Error, TEXT("LoadGame('%s'): kayit BOZUK (sikistirma/sinif uyusmazligi) — hicbir sey degistirilmedi"), *SlotName);
		return false;
	}

	SurvivalSave::MigrateSaveData(Loaded->Payload);

	const bool bApplied = ApplySaveDataToLiveSystems(Loaded->Payload);
	UE_LOG(LogSurvival, Log, TEXT("LoadGame('%s'): %s"), *SlotName, bApplied ? TEXT("basarili") : TEXT("kismi basarili (oyuncu bulunamadi)"));
	return bApplied;
}

bool USaveGameManager::RevertToLastSave()
{
	const USaveGameManagerSettings* Settings = GetDefault<USaveGameManagerSettings>();
	const FString SlotToLoad = !LastSavedSlotName.IsEmpty() ? LastSavedSlotName : Settings->AutosaveSlotName;

	if (!DoesSaveExist(SlotToLoad))
	{
		UE_LOG(LogSurvival, Warning, TEXT("RevertToLastSave: kayit noktasi yok ('%s'), geri donulemedi"), *SlotToLoad);
		return false;
	}

	UE_LOG(LogSurvival, Log, TEXT("RevertToLastSave: '%s' yuvasina donuluyor"), *SlotToLoad);
	return LoadGame(SlotToLoad);
}

bool USaveGameManager::DeleteSave(const FString& SlotName)
{
	return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

bool USaveGameManager::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdSaveGame(
		TEXT("save_game"),
		TEXT("Oyunu kaydeder: save_game [YuvaAdi] (varsayilan: QuickSave)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				USaveGameManager* Manager = GI ? GI->GetSubsystem<USaveGameManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				const FString SlotName = Args.Num() > 0 ? Args[0] : TEXT("QuickSave");
				Manager->SaveGame(SlotName);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdLoadGame(
		TEXT("load_game"),
		TEXT("Oyunu yukler: load_game [YuvaAdi] (varsayilan: QuickSave)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				USaveGameManager* Manager = GI ? GI->GetSubsystem<USaveGameManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				const FString SlotName = Args.Num() > 0 ? Args[0] : TEXT("QuickSave");
				Manager->LoadGame(SlotName);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdSaveDump(
		TEXT("save_dump"),
		TEXT("Kaydet/Yukle durumunu logla: save_dump [YuvaAdi] (varsayilan: QuickSave)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const USaveGameManager* Manager = GI ? GI->GetSubsystem<USaveGameManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				const FString SlotName = Args.Num() > 0 ? Args[0] : TEXT("QuickSave");
				UE_LOG(LogSurvival, Log, TEXT("SaveDump: toplam-oynama=%.1fsn yuva('%s')-var=%s"),
					Manager->GetTotalPlayTimeSeconds(), *SlotName, Manager->DoesSaveExist(SlotName) ? TEXT("EVET") : TEXT("hayir"));
			}));
}
