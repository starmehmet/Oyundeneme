#include "Weather/WindTurbine.h"
#include "Weather/WindSimulation.h"
#include "Weather/WindMath.h"
#include "Production/ResourceSimulation.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

AWindTurbine::AWindTurbine()
{
}

void AWindTurbine::AdvanceProduction(double CurrentGameTime)
{
	// Taban sinifin Tick_Production/recipe akisini KULLANMAZ (rüzgar->enerji anlik bir
	// donusum, girdi/cikti/yakit kavrami yok) — yalnizca kayit icin LastProductionUpdateTime
	// guncellenir (taban sinifta protected, buradan da erisilebilir).
	LastProductionUpdateTime = CurrentGameTime;

	const UGameInstance* GI = GetGameInstance();
	const UWindSimulation* WindSim = GI ? GI->GetSubsystem<UWindSimulation>() : nullptr;
	const float WindSpeed = WindSim ? WindSim->GetWindSpeedAt(GetActorLocation()) : 0.0f;

	CurrentOutput = SurvivalWind::ComputeTurbineOutput(WindSpeed, CutInSpeed, RatedSpeed, RatedOutput);
	State = (CurrentOutput > 0.0f) ? EProductionState::Running : EProductionState::Idle;

	if (UResourceSimulation* ResourceSim = GI ? GI->GetSubsystem<UResourceSimulation>() : nullptr)
	{
		ResourceSim->RegisterEnergyProducer(this, CurrentOutput);
	}
}

float AWindTurbine::GetProgress() const
{
	return RatedOutput > 0.0f ? FMath::Clamp(CurrentOutput / RatedOutput, 0.0f, 1.0f) : 0.0f;
}

void AWindTurbine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UResourceSimulation* ResourceSim = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UResourceSimulation>() : nullptr)
		{
			ResourceSim->UnregisterEnergyProducer(this);
		}
	}

	Super::EndPlay(EndPlayReason); // AProductionMachine::EndPlay: UProductionManager::UnregisterMachine
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (production_set_recipe'nin TActorIterator deseniyle ayni) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdTurbineDump(
		TEXT("turbine_dump"),
		TEXT("Sahnedeki ilk ruzgar turbininin durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}

				AWindTurbine* FoundTurbine = nullptr;
				for (TActorIterator<AWindTurbine> It(World); It; ++It)
				{
					FoundTurbine = *It;
					break;
				}
				if (!FoundTurbine)
				{
					UE_LOG(LogSurvivalWeather, Warning, TEXT("turbine_dump: sahnede AWindTurbine bulunamadi"));
					return;
				}

				UE_LOG(LogSurvivalWeather, Log, TEXT("Turbin: durum=%d ilerleme=%.2f cikti=%.1f"),
					static_cast<int32>(FoundTurbine->GetProductionState()), FoundTurbine->GetProgress(), FoundTurbine->GetCurrentOutput());
			}));
}
