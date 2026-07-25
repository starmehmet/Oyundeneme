#include "Production/ResourceSimulation.h"
#include "Production/ResourceSimulationMath.h"
#include "Production/ResourceSimulationSettings.h"
#include "SurvivalGame.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	float SumValues(const TMap<TWeakObjectPtr<AActor>, float>& Registry)
	{
		float Total = 0.0f;
		for (const TPair<TWeakObjectPtr<AActor>, float>& Pair : Registry)
		{
			if (Pair.Key.IsValid())
			{
				Total += Pair.Value;
			}
		}
		return Total;
	}

	void PruneRegistry(TMap<TWeakObjectPtr<AActor>, float>& Registry)
	{
		for (auto It = Registry.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
			}
		}
	}
}

void UResourceSimulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalProduction, Log, TEXT("ResourceSimulation hazir"));
}

bool UResourceSimulation::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UResourceSimulation::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void UResourceSimulation::RegisterEnergyProducer(AActor* Producer, float OutputPerSecond)
{
	if (Producer)
	{
		EnergyProducers.Add(Producer, FMath::Max(0.0f, OutputPerSecond));
	}
}

void UResourceSimulation::UnregisterEnergyProducer(AActor* Producer)
{
	EnergyProducers.Remove(Producer);
}

void UResourceSimulation::RegisterEnergyConsumer(AActor* Consumer, float ConsumptionPerSecond)
{
	if (Consumer)
	{
		EnergyConsumers.Add(Consumer, FMath::Max(0.0f, ConsumptionPerSecond));
	}
}

void UResourceSimulation::UnregisterEnergyConsumer(AActor* Consumer)
{
	EnergyConsumers.Remove(Consumer);
}

void UResourceSimulation::RegisterHeatProducer(AActor* Producer, float HeatPerSecond)
{
	if (Producer)
	{
		HeatProducers.Add(Producer, FMath::Max(0.0f, HeatPerSecond));
	}
}

void UResourceSimulation::UnregisterHeatProducer(AActor* Producer)
{
	HeatProducers.Remove(Producer);
}

float UResourceSimulation::AddFuel(FName FuelID, float Amount, float MaxAmount)
{
	if (FuelID.IsNone() || Amount <= 0.0f)
	{
		return 0.0f;
	}

	FFuelReserve& Reserve = FuelReserves.FindOrAdd(FuelID);
	if (MaxAmount > 0.0f)
	{
		Reserve.MaxAmount = FMath::Max(Reserve.MaxAmount, MaxAmount);
	}
	else if (Reserve.MaxAmount <= 0.0f)
	{
		// Ilk kayit, MaxAmount belirtilmedi -> eklenen miktari tavan olarak kullan (makul varsayilan).
		Reserve.MaxAmount = Amount;
	}

	const float Before = Reserve.CurrentAmount;
	Reserve.CurrentAmount = FMath::Clamp(Reserve.CurrentAmount + Amount, 0.0f, Reserve.MaxAmount);
	return Reserve.CurrentAmount - Before; // GERCEKTEN kabul edilen miktar (AddItem/RequestTransport ile ayni desen)
}

void UResourceSimulation::SetFuelConsumptionRate(FName FuelID, float ConsumptionRate)
{
	if (FuelID.IsNone())
	{
		return;
	}
	FFuelReserve& Reserve = FuelReserves.FindOrAdd(FuelID);
	Reserve.ConsumptionRate = FMath::Max(0.0f, ConsumptionRate);
}

bool UResourceSimulation::GetFuelReserve(FName FuelID, FFuelReserve& OutReserve) const
{
	if (const FFuelReserve* Found = FuelReserves.Find(FuelID))
	{
		OutReserve = *Found;
		return true;
	}
	return false;
}

void UResourceSimulation::Tick(float DeltaTime)
{
	PruneStaleRegistrations();
	RecomputeEnergyBudget();
	RecomputeThermalBudget(DeltaTime);
	DepleteFuelReserves(DeltaTime);
	EvaluateScarcity();
}

void UResourceSimulation::PruneStaleRegistrations()
{
	PruneRegistry(EnergyProducers);
	PruneRegistry(EnergyConsumers);
	PruneRegistry(HeatProducers);
}

void UResourceSimulation::RecomputeEnergyBudget()
{
	EnergyBudget.TotalProduction = SumValues(EnergyProducers);
	EnergyBudget.TotalConsumption = SumValues(EnergyConsumers);
	EnergyBudget.CurrentBalance = EnergyBudget.TotalProduction - EnergyBudget.TotalConsumption;
	EnergyBudget.Frequency = SurvivalResource::ComputeFrequency(EnergyBudget.TotalProduction, EnergyBudget.TotalConsumption);
}

void UResourceSimulation::RecomputeThermalBudget(float DeltaTime)
{
	const UResourceSimulationSettings* Settings = GetDefault<UResourceSimulationSettings>();

	ThermalBudget.TotalHeatProduction = SumValues(HeatProducers);
	ThermalBudget.TotalHeatDissipation = Settings->BaselineHeatDissipation;

	const float Delta = SurvivalResource::ComputeTemperatureDelta(
		ThermalBudget.TotalHeatProduction, ThermalBudget.TotalHeatDissipation, DeltaTime);
	ThermalBudget.CurrentTemperature = FMath::Max(0.0f, ThermalBudget.CurrentTemperature + Delta);
}

void UResourceSimulation::DepleteFuelReserves(float DeltaTime)
{
	for (TPair<FName, FFuelReserve>& Pair : FuelReserves)
	{
		FFuelReserve& Reserve = Pair.Value;
		Reserve.CurrentAmount = FMath::Clamp(
			Reserve.CurrentAmount - Reserve.ConsumptionRate * DeltaTime, 0.0f, Reserve.MaxAmount);
	}
}

void UResourceSimulation::EvaluateScarcity()
{
	const UResourceSimulationSettings* Settings = GetDefault<UResourceSimulationSettings>();

	EScarcityReason NewReason = EScarcityReason::None;
	if (SurvivalResource::IsBrownout(EnergyBudget.Frequency))
	{
		NewReason = EScarcityReason::EnergyBrownout;
	}
	else if (SurvivalResource::IsOverheating(ThermalBudget.CurrentTemperature, Settings->MaxSafeTemperature))
	{
		NewReason = EScarcityReason::Overheating;
	}
	else
	{
		for (const TPair<FName, FFuelReserve>& Pair : FuelReserves)
		{
			if (SurvivalResource::IsFuelCritical(Pair.Value.CurrentAmount, Pair.Value.MaxAmount, Settings->FuelCriticalFraction))
			{
				NewReason = EScarcityReason::FuelCritical;
				break;
			}
		}
	}

	const bool bNewInScarcity = (NewReason != EScarcityReason::None);
	if (bNewInScarcity != bInScarcity || NewReason != CurrentScarcityReason)
	{
		bInScarcity = bNewInScarcity;
		CurrentScarcityReason = NewReason;
		OnScarcityStateChanged.Broadcast(bInScarcity, CurrentScarcityReason);

		UE_LOG(LogSurvivalProduction, Warning, TEXT("Kitlik alarmi: %s (sebep: %d)"),
			bInScarcity ? TEXT("basladi") : TEXT("bitti"), static_cast<int32>(CurrentScarcityReason));
	}
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi) ----

namespace
{
	UResourceSimulation* FindResourceSim(UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<UResourceSimulation>()
			: nullptr;
	}

	AActor* FindResourcePlayerPawn(UWorld* World)
	{
		const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? PC->GetPawn() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdResourceRegisterProducer(
		TEXT("resource_register_producer"),
		TEXT("Oyuncu piyonunu enerji ureticisi olarak kaydet: resource_register_producer <Rate>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UResourceSimulation* Sim = FindResourceSim(World);
				AActor* Pawn = FindResourcePlayerPawn(World);
				if (!Sim || !Pawn || Args.Num() < 1)
				{
					UE_LOG(LogSurvivalProduction, Warning, TEXT("Kullanim: resource_register_producer <Rate>"));
					return;
				}
				Sim->RegisterEnergyProducer(Pawn, FCString::Atof(*Args[0]));
				UE_LOG(LogSurvivalProduction, Log, TEXT("resource_register_producer: %s birim/sn"), *Args[0]);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdResourceRegisterConsumer(
		TEXT("resource_register_consumer"),
		TEXT("Oyuncu piyonunu enerji tuketicisi olarak kaydet: resource_register_consumer <Rate>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UResourceSimulation* Sim = FindResourceSim(World);
				AActor* Pawn = FindResourcePlayerPawn(World);
				if (!Sim || !Pawn || Args.Num() < 1)
				{
					UE_LOG(LogSurvivalProduction, Warning, TEXT("Kullanim: resource_register_consumer <Rate>"));
					return;
				}
				Sim->RegisterEnergyConsumer(Pawn, FCString::Atof(*Args[0]));
				UE_LOG(LogSurvivalProduction, Log, TEXT("resource_register_consumer: %s birim/sn"), *Args[0]);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdResourceAddFuel(
		TEXT("resource_add_fuel"),
		TEXT("Yakit rezervi ekle/olustur: resource_add_fuel <FuelID> <Miktar> [MaxAmount] [ConsumptionRate]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UResourceSimulation* Sim = FindResourceSim(World);
				if (!Sim || Args.Num() < 2)
				{
					UE_LOG(LogSurvivalProduction, Warning,
						TEXT("Kullanim: resource_add_fuel <FuelID> <Miktar> [MaxAmount] [ConsumptionRate]"));
					return;
				}
				const FName FuelID(*Args[0]);
				const float Requested = FCString::Atof(*Args[1]);
				const float MaxAmount = Args.Num() > 2 ? FCString::Atof(*Args[2]) : -1.0f;
				const float Accepted = Sim->AddFuel(FuelID, Requested, MaxAmount);
				if (Args.Num() > 3)
				{
					Sim->SetFuelConsumptionRate(FuelID, FCString::Atof(*Args[3]));
				}
				UE_LOG(LogSurvivalProduction, Log, TEXT("resource_add_fuel: '%s' +%.1f (istenen %.1f)%s"),
					*Args[0], Accepted, Requested, Accepted < Requested ? TEXT(" - TAVANA ULASILDI") : TEXT(""));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdResourceDump(
		TEXT("resource_dump"),
		TEXT("Mevcut enerji/termal/yakit durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UResourceSimulation* Sim = FindResourceSim(World);
				if (!Sim)
				{
					return;
				}
				const FEnergyBudget& EB = Sim->GetEnergyBudget();
				const FThermalBudget& TB = Sim->GetThermalBudget();
				UE_LOG(LogSurvivalProduction, Log,
					TEXT("Enerji: uretim=%.1f tuketim=%.1f denge=%.1f frekans=%.1fHz (%s) | Termal: uretim=%.1f dagitim=%.1f sicaklik=%.1f | Kitlik: %s"),
					EB.TotalProduction, EB.TotalConsumption, EB.CurrentBalance, EB.Frequency,
					EB.IsFrequencyStable() ? TEXT("istikrarli") : TEXT("istikrarsiz"),
					TB.TotalHeatProduction, TB.TotalHeatDissipation, TB.CurrentTemperature,
					Sim->IsInScarcity() ? TEXT("EVET") : TEXT("hayir"));
			}));
}
