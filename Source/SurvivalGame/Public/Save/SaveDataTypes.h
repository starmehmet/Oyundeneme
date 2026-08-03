#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/InventoryComponent.h"
#include "Weather/WeatherTypes.h"
#include "Production/ProductionState.h"
#include "Production/ResourceTypes.h"
#include "NPC/NPCState.h"
#include "NPC/NPCTaskData.h"
#include "NPC/TaskDefinition.h"
#include "Audio/SoundCategory.h"
#include "SaveDataTypes.generated.h"

USTRUCT()
struct FBuildingSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName BuildingID;

	UPROPERTY()
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FName ActiveRecipeID;

	UPROPERTY()
	float MachineProgress = 0.0f;

	UPROPERTY()
	float MachineEnergy = 0.0f;

	UPROPERTY()
	uint8 MachineState = 0;

	UPROPERTY()
	TArray<FInventorySlot> InputSlots;

	UPROPERTY()
	TArray<FInventorySlot> OutputSlots;
};

USTRUCT()
struct FWeatherSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FWeatherState CurrentState;

	UPROPERTY()
	FWeatherState TransitionStartState;

	UPROPERTY()
	FWeatherState TargetState;

	UPROPERTY()
	float TransitionProgress = 1.0f;

	UPROPERTY()
	float TransitionElapsed = 0.0f;

	UPROPERTY()
	float TimeSinceLastEvaluation = 0.0f;
};

USTRUCT()
struct FResourceSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FName, FFuelReserve> FuelReserves;

	UPROPERTY()
	float ThermalTemperature = 0.0f;
};

USTRUCT()
struct FHarvestNodeSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorName;

	UPROPERTY()
	int32 RemainingHarvests = 0;

	UPROPERTY()
	bool bDepleted = false;

	UPROPERTY()
	double DepletionGameTime = 0.0;
};

USTRUCT()
struct FNPCSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorName;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	float Health = 100.0f;

	UPROPERTY()
	uint8 BrainState = 0;

	UPROPERTY()
	float Fatigue = 0.0f;

	UPROPERTY()
	float Morale = 1.0f;

	UPROPERTY()
	bool bHasTask = false;

	UPROPERTY()
	FNPCTaskData CurrentTask;

	UPROPERTY()
	float WorkElapsedTime = 0.0f;

	UPROPERTY()
	float WalkingElapsedTime = 0.0f;
};

USTRUCT()
struct FTaskSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName TaskID;

	UPROPERTY()
	FText TaskName;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float Priority = 0.0f;

	UPROPERTY()
	int32 RequiredSkillLevel = 0;

	UPROPERTY()
	float WorkDuration = 5.0f;

	UPROPERTY()
	double AvailableAfterGameTime = 0.0;
};

/**
 * Sistem #17 — SaveVersion=2: Zaman+Oyuncu (v1) + Insaat/Uretim/Hava/Kar/Kaynak/
 * Hasat/NPC/Gorev/Ses (v2). Eski kayitlar (v1) yeni alanlar icin bos/varsayilan
 * degerlerle acilir (ekleme-tabanli, kirilma yok).
 */
USTRUCT()
struct FGameSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SaveVersion = 2;

	UPROPERTY()
	float TotalPlayTimeSeconds = 0.0f;

	UPROPERTY()
	double TotalGameSeconds = 0.0;

	// ---- Oyuncu (v1) ----
	UPROPERTY()
	FVector PlayerPosition = FVector::ZeroVector;

	UPROPERTY()
	float PlayerHealth = 100.0f;

	UPROPERTY()
	float PlayerBodyTemperature = 37.0f;

	UPROPERTY()
	TArray<FInventorySlot> PlayerInventory;

	// ---- Insaat + Uretim (v2) ----
	UPROPERTY()
	TArray<FBuildingSaveData> Buildings;

	// ---- Hava Durumu (v2) ----
	UPROPERTY()
	FWeatherSaveData Weather;

	// ---- Kar (v2) ----
	UPROPERTY()
	float SnowDepth = 0.0f;

	// ---- Kaynak Simulasyonu (v2) ----
	UPROPERTY()
	FResourceSaveData Resources;

	// ---- Hasat Dugumleri (v2) ----
	UPROPERTY()
	TArray<FHarvestNodeSaveData> HarvestNodes;

	// ---- NPC (v2) ----
	UPROPERTY()
	TArray<FNPCSaveData> NPCs;

	// ---- Gorev Kuyrugu (v2) ----
	UPROPERTY()
	TArray<FTaskSaveData> PendingTasks;

	// ---- Ses Ayarlari (v2) ----
	UPROPERTY()
	TMap<ESoundCategory, float> AudioVolumes;
};

UCLASS()
class SURVIVALGAME_API USurvivalSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameSaveData Payload;
};
