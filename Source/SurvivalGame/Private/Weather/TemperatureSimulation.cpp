#include "Weather/TemperatureSimulation.h"
#include "Weather/TemperatureMath.h"
#include "Weather/TemperatureSimulationSettings.h"
#include "Weather/WeatherSimulation.h"
#include "Time/TimeKeeper.h"
#include "Player/PlayerCharacter.h"
#include "Player/HealthComponent.h"
#include "SurvivalGame.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	APlayerCharacter* FindPlayerCharacter(UWorld* World)
	{
		const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? Cast<APlayerCharacter>(PC->GetPawn()) : nullptr;
	}
}

void UTemperatureSimulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalWeather, Log, TEXT("TemperatureSimulation hazir"));
}

bool UTemperatureSimulation::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UTemperatureSimulation::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

float UTemperatureSimulation::GetAmbientTemperature() const
{
	return LastAmbientTemperature;
}

float UTemperatureSimulation::GetEnvironmentTemperatureAt(const FVector& Position) const
{
	for (const TPair<TWeakObjectPtr<AActor>, FThermalZone>& Pair : ThermalZones)
	{
		if (!Pair.Key.IsValid())
		{
			continue;
		}
		const FThermalZone& Zone = Pair.Value;
		if (FVector::Dist(Position, Zone.Center) <= Zone.Radius)
		{
			return SurvivalTemperature::ComputeInteriorTemperature(
				LastAmbientTemperature, Zone.BaseTemperature, Zone.HeatSourcePower, Zone.InsulationFactor);
		}
	}
	return LastAmbientTemperature;
}

void UTemperatureSimulation::RegisterThermalZone(AActor* ZoneActor, const FThermalZone& ZoneData)
{
	if (ZoneActor)
	{
		ThermalZones.Add(ZoneActor, ZoneData);
	}
}

void UTemperatureSimulation::UnregisterThermalZone(AActor* ZoneActor)
{
	ThermalZones.Remove(ZoneActor);
}

void UTemperatureSimulation::ApplyPlayerThermalStress(APlayerCharacter* Player, float DeltaTime)
{
	if (!Player)
	{
		return;
	}

	const UTemperatureSimulationSettings* Settings = GetDefault<UTemperatureSimulationSettings>();

	const float EnvironmentTemp = GetEnvironmentTemperatureAt(Player->GetActorLocation());
	LastPlayerEnvironmentTemperature = EnvironmentTemp;

	const float TargetBodyTemp = SurvivalTemperature::ComputeTargetBodyTemperature(
		EnvironmentTemp, Settings->ComfortMinTemp, Settings->ComfortMaxTemp);

	const float Delta = SurvivalTemperature::ComputeBodyTemperatureDelta(
		Player->GetBodyTemperature(), TargetBodyTemp, Settings->AdaptationRate, DeltaTime);
	const float NewBodyTemp = Player->GetBodyTemperature() + Delta;
	Player->SetBodyTemperature(NewBodyTemp);

	const float Damage = SurvivalTemperature::ComputeThermalDamage(
		NewBodyTemp, Settings->SafeMinBodyTemp, Settings->SafeMaxBodyTemp, Settings->DamagePerSecondPerDegree, DeltaTime);
	if (Damage > 0.0f)
	{
		if (UHealthComponent* Health = Player->GetHealthComponent())
		{
			Health->TakeDamage(Damage);
		}
	}
}

void UTemperatureSimulation::PruneStaleRegistrations()
{
	for (auto It = ThermalZones.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void UTemperatureSimulation::Tick(float DeltaTime)
{
	PruneStaleRegistrations();

	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;

	const UWeatherSimulation* WeatherSim = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr;
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	const UTemperatureSimulationSettings* Settings = GetDefault<UTemperatureSimulationSettings>();

	const float WeatherTemp = WeatherSim ? WeatherSim->GetCurrentState().Temperature : 20.0f;
	const float HourOfDay = Clock ? Clock->GetMinuteOfDayFloat() / 60.0f : 12.0f;
	LastAmbientTemperature = WeatherTemp + Settings->DayNightAmplitude * SurvivalTemperature::ComputeTimeOfDayModulation(HourOfDay);

	if (APlayerCharacter* Player = FindPlayerCharacter(World))
	{
		ApplyPlayerThermalStress(Player, DeltaTime);
	}
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	UTemperatureSimulation* FindTemperatureSim(UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<UTemperatureSimulation>()
			: nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdTemperatureRegisterZone(
		TEXT("temperature_register_zone"),
		TEXT("Oyuncunun su anki konumunda bir termal bolge kaydet: temperature_register_zone <Radius> <BaseTemp> <HeatPower> <Insulation>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UTemperatureSimulation* Sim = FindTemperatureSim(World);
				APlayerCharacter* Player = FindPlayerCharacter(World);
				if (!Sim || !Player || Args.Num() < 4)
				{
					UE_LOG(LogSurvivalWeather, Warning,
						TEXT("Kullanim: temperature_register_zone <Radius> <BaseTemp> <HeatPower> <Insulation>"));
					return;
				}

				FThermalZone Zone;
				Zone.Center = Player->GetActorLocation();
				Zone.Radius = FCString::Atof(*Args[0]);
				Zone.BaseTemperature = FCString::Atof(*Args[1]);
				Zone.HeatSourcePower = FCString::Atof(*Args[2]);
				Zone.InsulationFactor = FCString::Atof(*Args[3]);
				Sim->RegisterThermalZone(Player, Zone);

				UE_LOG(LogSurvivalWeather, Log,
					TEXT("temperature_register_zone: yaricap=%.0f temel=%.1f isikaynagi=%.1f yalitim=%.2f (oyuncu konumunda)"),
					Zone.Radius, Zone.BaseTemperature, Zone.HeatSourcePower, Zone.InsulationFactor);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdTemperatureDump(
		TEXT("temperature_dump"),
		TEXT("Mevcut ortam/oyuncu sicakligi ve canini logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UTemperatureSimulation* Sim = FindTemperatureSim(World);
				const APlayerCharacter* Player = FindPlayerCharacter(World);
				if (!Sim)
				{
					return;
				}

				const UTemperatureSimulationSettings* Settings = GetDefault<UTemperatureSimulationSettings>();
				const float BodyTemp = Player ? Player->GetBodyTemperature() : -1.0f;
				const float Health = (Player && Player->GetHealthComponent()) ? Player->GetHealthComponent()->GetCurrentHealth() : -1.0f;
				const float EnvironmentTemp = Sim->GetEnvironmentTemperatureAt(Player ? Player->GetActorLocation() : FVector::ZeroVector);
				const float TargetBodyTemp = SurvivalTemperature::ComputeTargetBodyTemperature(
					EnvironmentTemp, Settings->ComfortMinTemp, Settings->ComfortMaxTemp);

				UE_LOG(LogSurvivalWeather, Log,
					TEXT("Sicaklik: ortam=%.1f oyuncu-cevre=%.1f hedef-vucut=%.1f vucut=%.1f can=%.1f"),
					Sim->GetAmbientTemperature(), EnvironmentTemp, TargetBodyTemp, BodyTemp, Health);
			}));
}
