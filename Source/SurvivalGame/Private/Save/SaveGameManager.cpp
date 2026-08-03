#include "Save/SaveGameManager.h"
#include "Save/SaveDataSerializer.h"
#include "Save/SaveGameManagerSettings.h"
#include "Time/TimeKeeper.h"
#include "Player/PlayerCharacter.h"
#include "Player/HealthComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Construction/BuildingBase.h"
#include "Construction/BuildingDatabase.h"
#include "Construction/FoundationGrid.h"
#include "Production/ProductionMachine.h"
#include "Production/ResourceSimulation.h"
#include "Logistics/LogisticsNetwork.h"
#include "Weather/WeatherSimulation.h"
#include "Weather/SnowAccumulation.h"
#include "Harvesting/HarvestNode.h"
#include "NPC/NPCCharacter.h"
#include "NPC/NPCBrain.h"
#include "NPC/TaskScheduler.h"
#include "Audio/AudioManager.h"
#include "SurvivalGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
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
	Data.SaveVersion = 2;
	Data.TotalPlayTimeSeconds = TotalPlayTimeSeconds;

	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;

	// ---- Zaman (v1) ----
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	Data.TotalGameSeconds = Clock ? Clock->GetTotalGameSeconds() : 0.0;

	// ---- Oyuncu (v1) ----
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

	// ---- Hava Durumu (v2) ----
	if (const UWeatherSimulation* Weather = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr)
	{
		Data.Weather.CurrentState = Weather->GetCurrentState();
		Data.Weather.TransitionStartState = Weather->GetTransitionStartState();
		Data.Weather.TargetState = Weather->GetTargetState();
		Data.Weather.TransitionProgress = Weather->GetTransitionProgress();
		Data.Weather.TransitionElapsed = Weather->GetTransitionElapsed();
		Data.Weather.TimeSinceLastEvaluation = Weather->GetTimeSinceLastEvaluation();
	}

	// ---- Kar (v2) ----
	if (const USnowAccumulation* Snow = GI ? GI->GetSubsystem<USnowAccumulation>() : nullptr)
	{
		Data.SnowDepth = Snow->GetCurrentSnowDepth();
	}

	// ---- Kaynak Simulasyonu (v2) ----
	if (const UResourceSimulation* Resources = GI ? GI->GetSubsystem<UResourceSimulation>() : nullptr)
	{
		Data.Resources.FuelReserves = Resources->GetFuelReserves();
		Data.Resources.ThermalTemperature = Resources->GetThermalBudget().CurrentTemperature;
	}

	// ---- Insaat + Uretim (v2) ----
	if (World)
	{
		for (TActorIterator<ABuildingBase> It(World); It; ++It)
		{
			ABuildingBase* Building = *It;
			if (!Building || !Building->IsConstructed())
			{
				continue;
			}

			FBuildingSaveData BSD;
			BSD.BuildingID = Building->GetBuildingID();
			BSD.GridCoord = Building->GetGridCoord();
			BSD.Rotation = Building->GetActorRotation();

			if (const AProductionMachine* Machine = Cast<AProductionMachine>(Building))
			{
				BSD.ActiveRecipeID = Machine->GetActiveRecipeID();
				BSD.MachineProgress = Machine->GetRawProgress();
				BSD.MachineEnergy = Machine->GetCurrentEnergy();
				BSD.MachineState = static_cast<uint8>(Machine->GetProductionState());
				if (const UInventoryComponent* InBuf = Machine->GetInputBuffer())
				{
					BSD.InputSlots = InBuf->GetSlots();
				}
				if (const UInventoryComponent* OutBuf = Machine->GetOutputBuffer())
				{
					BSD.OutputSlots = OutBuf->GetSlots();
				}
			}

			Data.Buildings.Add(BSD);
		}
	}

	// ---- Hasat Dugumleri (v2) ----
	if (World)
	{
		for (TActorIterator<AHarvestNode> It(World); It; ++It)
		{
			const AHarvestNode* Node = *It;
			if (!Node)
			{
				continue;
			}

			FHarvestNodeSaveData HSD;
			HSD.ActorName = Node->GetFName();
			HSD.RemainingHarvests = Node->GetRemainingHarvests();
			HSD.bDepleted = Node->IsDepleted();
			HSD.DepletionGameTime = Node->GetDepletionGameTime();
			Data.HarvestNodes.Add(HSD);
		}
	}

	// ---- NPC (v2) ----
	if (World)
	{
		for (TActorIterator<ANPCCharacter> It(World); It; ++It)
		{
			const ANPCCharacter* NPC = *It;
			if (!NPC)
			{
				continue;
			}

			FNPCSaveData NSD;
			NSD.ActorName = NPC->GetFName();
			NSD.Position = NPC->GetActorLocation();
			NSD.Rotation = NPC->GetActorRotation();
			if (const UHealthComponent* Health = NPC->GetHealthComponent())
			{
				NSD.Health = Health->GetCurrentHealth();
			}
			if (const UNPCBrain* Brain = NPC->GetBrain())
			{
				NSD.BrainState = static_cast<uint8>(Brain->GetCurrentState());
				NSD.Fatigue = Brain->GetFatigue();
				NSD.Morale = Brain->GetMorale();
				NSD.bHasTask = Brain->HasTask();
				NSD.CurrentTask = Brain->GetCurrentTask();
				NSD.WorkElapsedTime = Brain->GetWorkElapsedTime();
				NSD.WalkingElapsedTime = Brain->GetWalkingElapsedTime();
			}
			Data.NPCs.Add(NSD);
		}
	}

	// ---- Gorev Kuyrugu (v2) ----
	if (const UTaskScheduler* Scheduler = GI ? GI->GetSubsystem<UTaskScheduler>() : nullptr)
	{
		for (const FTaskDefinition& TaskDef : Scheduler->GetPendingTasks())
		{
			FTaskSaveData TSD;
			TSD.TaskID = TaskDef.TaskID;
			TSD.TaskName = TaskDef.TaskName;
			TSD.TargetLocation = TaskDef.TargetLocation;
			TSD.Priority = TaskDef.Priority;
			TSD.RequiredSkillLevel = TaskDef.RequiredSkillLevel;
			TSD.WorkDuration = TaskDef.WorkDuration;
			TSD.AvailableAfterGameTime = TaskDef.AvailableAfterGameTime;
			Data.PendingTasks.Add(TSD);
		}
	}

	// ---- Ses Ayarlari (v2) ----
	if (const UAudioManager* Audio = GI ? GI->GetSubsystem<UAudioManager>() : nullptr)
	{
		Data.AudioVolumes = Audio->GetCategoryVolumes();
	}

	return Data;
}

bool USaveGameManager::ApplySaveDataToLiveSystems(const FGameSaveData& Data)
{
	APlayerCharacter* Player = FindPlayerCharacter();
	if (!Player)
	{
		UE_LOG(LogSurvival, Warning, TEXT("ApplySaveDataToLiveSystems: oyuncu piyonu bulunamadi — hicbir sey uygulanmadi"));
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;

	// ---- Zaman + Oyuncu (v1) ----
	TotalPlayTimeSeconds = Data.TotalPlayTimeSeconds;

	if (UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr)
	{
		Clock->SetTotalGameSeconds(Data.TotalGameSeconds);
	}

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

	// ---- Hava Durumu (v2) ----
	if (UWeatherSimulation* Weather = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr)
	{
		Weather->RestoreStateForLoad(
			Data.Weather.CurrentState, Data.Weather.TransitionStartState,
			Data.Weather.TargetState, Data.Weather.TransitionProgress,
			Data.Weather.TransitionElapsed, Data.Weather.TimeSinceLastEvaluation);
	}

	// ---- Kar (v2) ----
	if (USnowAccumulation* Snow = GI ? GI->GetSubsystem<USnowAccumulation>() : nullptr)
	{
		Snow->SetSnowDepthForTesting(Data.SnowDepth);
	}

	// ---- Insaat: mevcut binalari yok et + kayittan yeniden olustur (v2) ----
	if (World)
	{
		TArray<ABuildingBase*> ExistingBuildings;
		for (TActorIterator<ABuildingBase> It(World); It; ++It)
		{
			ExistingBuildings.Add(*It);
		}
		for (ABuildingBase* Building : ExistingBuildings)
		{
			Building->Demolish();
		}

		const UBuildingDatabase* BuildingDB = GI ? GI->GetSubsystem<UBuildingDatabase>() : nullptr;
		UFoundationGrid* Grid = World->GetSubsystem<UFoundationGrid>();

		for (const FBuildingSaveData& BSD : Data.Buildings)
		{
			if (BSD.BuildingID.IsNone() || !BuildingDB)
			{
				continue;
			}

			FBuildingDefinition Def;
			if (!BuildingDB->FindBuilding(BSD.BuildingID, Def))
			{
				UE_LOG(LogSurvival, Warning, TEXT("ApplySaveData: BuildingID '%s' veritabaninda bulunamadi — atlaniyor"), *BSD.BuildingID.ToString());
				continue;
			}

			UClass* SpawnClass = Def.BuildingClass ? Def.BuildingClass.Get() : ABuildingBase::StaticClass();
			const FVector SpawnLocation = Grid ? Grid->GridCoordToWorld(BSD.GridCoord) : FVector::ZeroVector;
			ABuildingBase* Spawned = World->SpawnActor<ABuildingBase>(SpawnClass, SpawnLocation, BSD.Rotation);
			if (!Spawned)
			{
				UE_LOG(LogSurvival, Warning, TEXT("ApplySaveData: BuildingID '%s' spawn basarisiz"), *BSD.BuildingID.ToString());
				continue;
			}

			Spawned->BeginConstruction(BSD.BuildingID, Def, BSD.GridCoord);
			if (Grid)
			{
				Grid->RegisterBuilding(Spawned, BSD.GridCoord);
			}

			if (AProductionMachine* Machine = Cast<AProductionMachine>(Spawned))
			{
				Machine->RestoreStateForLoad(
					BSD.ActiveRecipeID, BSD.MachineProgress, BSD.MachineEnergy,
					static_cast<EProductionState>(BSD.MachineState),
					BSD.InputSlots, BSD.OutputSlots);
			}
		}
	}

	// ---- Kaynak Simulasyonu (v2) ----
	if (UResourceSimulation* Resources = GI ? GI->GetSubsystem<UResourceSimulation>() : nullptr)
	{
		Resources->RestoreStateForLoad(Data.Resources.FuelReserves, Data.Resources.ThermalTemperature);
	}

	// ---- Hasat Dugumleri (v2) ----
	if (World)
	{
		TMap<FName, const FHarvestNodeSaveData*> HarvestMap;
		for (const FHarvestNodeSaveData& HSD : Data.HarvestNodes)
		{
			HarvestMap.Add(HSD.ActorName, &HSD);
		}
		for (TActorIterator<AHarvestNode> It(World); It; ++It)
		{
			AHarvestNode* Node = *It;
			if (const FHarvestNodeSaveData* const* Found = HarvestMap.Find(Node->GetFName()))
			{
				Node->RestoreStateForLoad((*Found)->RemainingHarvests, (*Found)->bDepleted, (*Found)->DepletionGameTime);
			}
		}
	}

	// ---- NPC (v2) ----
	if (World)
	{
		TMap<FName, const FNPCSaveData*> NPCMap;
		for (const FNPCSaveData& NSD : Data.NPCs)
		{
			NPCMap.Add(NSD.ActorName, &NSD);
		}
		for (TActorIterator<ANPCCharacter> It(World); It; ++It)
		{
			ANPCCharacter* NPC = *It;
			const FNPCSaveData* const* Found = NPCMap.Find(NPC->GetFName());
			if (!Found)
			{
				continue;
			}
			const FNPCSaveData& NSD = **Found;
			NPC->TeleportTo(NSD.Position, NSD.Rotation);
			if (UHealthComponent* Health = NPC->GetHealthComponent())
			{
				Health->SetCurrentHealthForLoad(NSD.Health);
			}
			if (UNPCBrain* Brain = NPC->GetBrain())
			{
				Brain->RestoreStateForLoad(
					static_cast<ENPCState>(NSD.BrainState), NSD.Fatigue, NSD.Morale,
					NSD.bHasTask, NSD.CurrentTask, NSD.WorkElapsedTime, NSD.WalkingElapsedTime);
			}
		}
	}

	// ---- Gorev Kuyrugu (v2) ----
	if (UTaskScheduler* Scheduler = GI ? GI->GetSubsystem<UTaskScheduler>() : nullptr)
	{
		TArray<FTaskDefinition> RestoredTasks;
		for (const FTaskSaveData& TSD : Data.PendingTasks)
		{
			FTaskDefinition TaskDef;
			TaskDef.TaskID = TSD.TaskID;
			TaskDef.TaskName = TSD.TaskName;
			TaskDef.TargetLocation = TSD.TargetLocation;
			TaskDef.Priority = TSD.Priority;
			TaskDef.RequiredSkillLevel = TSD.RequiredSkillLevel;
			TaskDef.WorkDuration = TSD.WorkDuration;
			TaskDef.AvailableAfterGameTime = TSD.AvailableAfterGameTime;
			RestoredTasks.Add(TaskDef);
		}
		Scheduler->RestorePendingTasksForLoad(RestoredTasks);
	}

	// ---- Ses Ayarlari (v2) ----
	if (UAudioManager* Audio = GI ? GI->GetSubsystem<UAudioManager>() : nullptr)
	{
		Audio->RestoreVolumesForLoad(Data.AudioVolumes);
	}

	return true;
}

bool USaveGameManager::SaveGame(const FString& SlotName)
{
	// Transit esyalari kaydetmeden ONCE tamamla — BuildSaveDataFromLiveSystems const oldugu
	// icin yan etkili islem burada yapilir (lojistik durum degisikligi kayit verisine yansisin).
	if (ULogisticsNetwork* Logistics = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULogisticsNetwork>() : nullptr)
	{
		Logistics->CompleteAllTransportsForSave();
	}

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
