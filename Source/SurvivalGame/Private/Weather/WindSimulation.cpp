#include "Weather/WindSimulation.h"
#include "Weather/WindMath.h"
#include "Weather/WindSimulationSettings.h"
#include "Weather/WeatherSimulation.h"
#include "Time/TimeKeeper.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

void UWindSimulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalWeather, Log, TEXT("WindSimulation hazir"));
}

float UWindSimulation::GetGlobalWindSpeed() const
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherSimulation* WeatherSim = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr;
	return WeatherSim ? WeatherSim->GetCurrentState().WindSpeed : 0.0f;
}

FVector UWindSimulation::GetGlobalWindDirection() const
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherSimulation* WeatherSim = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr;
	return WeatherSim ? WeatherSim->GetCurrentState().WindDirection : FVector::ForwardVector;
}

FVector UWindSimulation::GetWindVectorAt(const FVector& Position) const
{
	const UGameInstance* GI = GetGameInstance();
	const UTimeKeeper* Clock = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr;
	const UWindSimulationSettings* Settings = GetDefault<UWindSimulationSettings>();

	const float GameTimeSeconds = Clock ? static_cast<float>(Clock->GetTotalGameSeconds()) : 0.0f;
	const FIntPoint GridCell = SurvivalWind::ComputeGustGridCell(Position, Settings->GustCellSize);
	const float PhaseOffset = SurvivalWind::ComputeGustPhaseOffset(GridCell.X, GridCell.Y);
	const float GustFactor = SurvivalWind::ComputeGustFactor(GameTimeSeconds, PhaseOffset, Settings->GustFrequency);

	const float FinalSpeed = SurvivalWind::ComputeWindSpeedAt(GetGlobalWindSpeed(), GustFactor, Settings->GustAmplitude);
	return GetGlobalWindDirection() * FinalSpeed;
}

float UWindSimulation::GetWindSpeedAt(const FVector& Position) const
{
	return GetWindVectorAt(Position).Size();
}

float UWindSimulation::GetWindLoadOnStructure(const FVector& Position, float ExposedArea) const
{
	const UWindSimulationSettings* Settings = GetDefault<UWindSimulationSettings>();
	return SurvivalWind::ComputeWindLoad(GetWindSpeedAt(Position), ExposedArea, Settings->DefaultDragCoefficient);
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	APawn* FindWindPlayerPawn(UWorld* World)
	{
		const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		return PC ? PC->GetPawn() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdWindDump(
		TEXT("wind_dump"),
		TEXT("Oyuncunun konumundaki ruzgar vektorunu ve varsayimsal bir yapi ustundeki yukunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const UWindSimulation* Sim = GI ? GI->GetSubsystem<UWindSimulation>() : nullptr;
				const APawn* Pawn = FindWindPlayerPawn(World);
				if (!Sim || !Pawn)
				{
					return;
				}

				const FVector Position = Pawn->GetActorLocation();
				const FVector Wind = Sim->GetWindVectorAt(Position);
				// 20 m^2: PIE dogrulamasi icin varsayimsal bir yuzey alani (gercek bir ExposedArea
				// alani ABuildingBase'de henuz yok — bkz. GetWindLoadOnStructure yorumu).
				const float Load = Sim->GetWindLoadOnStructure(Position, 20.0f);

				UE_LOG(LogSurvivalWeather, Log,
					TEXT("Ruzgar: kuresel-hiz=%.1f yerel-hiz=%.1f yon=(%.2f,%.2f,%.2f) varsayimsal-yuk(20m^2)=%.1f"),
					Sim->GetGlobalWindSpeed(), Wind.Size(), Wind.X, Wind.Y, Wind.Z, Load);
			}));
}
