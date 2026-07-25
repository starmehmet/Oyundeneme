#include "Weather/SnowAccumulation.h"
#include "Weather/SnowMath.h"
#include "Weather/SnowSimulationSettings.h"
#include "Weather/WeatherSimulation.h"
#include "Weather/WeatherCondition.h"
#include "Weather/TemperatureSimulation.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"

void USnowAccumulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalWeather, Log, TEXT("SnowAccumulation hazir"));
}

bool USnowAccumulation::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* USnowAccumulation::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

float USnowAccumulation::GetMovementSpeedMultiplier() const
{
	const USnowSimulationSettings* Settings = GetDefault<USnowSimulationSettings>();
	return SurvivalSnow::ComputeMovementSpeedMultiplier(CurrentSnowDepth, Settings->MaxMovementPenaltyDepth, Settings->MinSpeedMultiplier);
}

bool USnowAccumulation::IsConstructionBlocked() const
{
	const USnowSimulationSettings* Settings = GetDefault<USnowSimulationSettings>();
	return SurvivalSnow::IsConstructionBlocked(CurrentSnowDepth, Settings->MaxConstructionDepth);
}

void USnowAccumulation::SetSnowDepthForTesting(float NewDepth)
{
	const USnowSimulationSettings* Settings = GetDefault<USnowSimulationSettings>();
	CurrentSnowDepth = FMath::Clamp(NewDepth, 0.0f, Settings->MaxSnowDepth);
}

void USnowAccumulation::Tick(float DeltaTime)
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherSimulation* WeatherSim = GI ? GI->GetSubsystem<UWeatherSimulation>() : nullptr;
	const UTemperatureSimulation* TempSim = GI ? GI->GetSubsystem<UTemperatureSimulation>() : nullptr;
	const USnowSimulationSettings* Settings = GetDefault<USnowSimulationSettings>();

	const EWeatherCondition Condition = WeatherSim ? WeatherSim->GetCurrentState().Condition : EWeatherCondition::Clear;
	const bool bIsSnowyCondition = (Condition == EWeatherCondition::Snowing || Condition == EWeatherCondition::Blizzard);
	const float Precipitation = WeatherSim ? WeatherSim->GetCurrentState().Precipitation : 0.0f;
	const float AmbientTemperature = TempSim ? TempSim->GetAmbientTemperature() : 20.0f;

	const float AccumulationRate = SurvivalSnow::ComputeSnowAccumulationRate(bIsSnowyCondition, Precipitation, Settings->AccumulationCoefficient);
	const float MeltRate = SurvivalSnow::ComputeSnowMeltRate(AmbientTemperature, Settings->FreezingTemperature, Settings->MeltCoefficient);
	const float Delta = SurvivalSnow::ComputeSnowDepthDelta(AccumulationRate, MeltRate, DeltaTime);
	CurrentSnowDepth = FMath::Clamp(CurrentSnowDepth + Delta, 0.0f, Settings->MaxSnowDepth);

	UWorld* World = GI ? GI->GetWorld() : nullptr;
	ApplyMovementPenalty(World);

	TimeSinceLastAvalancheCheck += DeltaTime;
	if (TimeSinceLastAvalancheCheck >= Settings->AvalancheCheckInterval)
	{
		TimeSinceLastAvalancheCheck = 0.0f;
		EvaluateAvalancheRisk(World);
	}
}

void USnowAccumulation::ApplyMovementPenalty(UWorld* World)
{
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	ACharacter* PlayerChar = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
	UCharacterMovementComponent* Movement = PlayerChar ? PlayerChar->GetCharacterMovement() : nullptr;
	if (!Movement)
	{
		return;
	}

	if (BaseWalkSpeedCachedFor != PlayerChar)
	{
		// Kar sistemi bu degeri degistirmeden ONCE, HER YENI piyon icin (ilk gorusme VEYA
		// piyon degisimi) tek seferlik yakalanir — aksi halde her Tick MaxWalkSpeed'i BIR
		// ONCEKI (zaten kucultulmus) degere gore olcekleyip carpanlari bilesik hale getirirdi
		// (kumulatif, yanlis kuculme); TEK bir bool bayrak olsaydi piyon degisiminde de YENI
		// piyonun kendi hizini hic yakalamaz, ESKI piyonun degeriyle ezerdi (inceleme bulgusu).
		BaseWalkSpeed = Movement->MaxWalkSpeed;
		BaseWalkSpeedCachedFor = PlayerChar;
	}

	Movement->MaxWalkSpeed = BaseWalkSpeed * GetMovementSpeedMultiplier();
}

void USnowAccumulation::EvaluateAvalancheRisk(UWorld* World)
{
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	const FVector Start = Pawn->GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, 1000.0f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);

	FHitResult Hit;
	const bool bHitGround = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	const USnowSimulationSettings* Settings = GetDefault<USnowSimulationSettings>();
	const float SlopeAngle = bHitGround ? SurvivalSnow::ComputeSlopeAngleDegrees(Hit.ImpactNormal) : 0.0f;
	const bool bNewRisk = bHitGround && SurvivalSnow::IsAvalancheRisk(
		CurrentSnowDepth, SlopeAngle, Settings->AvalancheMinDepth, Settings->AvalancheMinSlopeDegrees, Settings->AvalancheMaxSlopeDegrees);

	if (bNewRisk != bInAvalancheRisk)
	{
		bInAvalancheRisk = bNewRisk;
		OnAvalancheRiskChanged.Broadcast(bInAvalancheRisk);
		UE_LOG(LogSurvivalWeather, Warning, TEXT("Cig riski: %s (egim=%.1f derinlik=%.1f)"),
			bInAvalancheRisk ? TEXT("basladi") : TEXT("bitti"), SlopeAngle, CurrentSnowDepth);
	}
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	USnowAccumulation* FindSnowSim(UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<USnowAccumulation>()
			: nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdSnowSetDepth(
		TEXT("snow_set_depth"),
		TEXT("Kar derinligini dogrudan ayarla (dev-tool): snow_set_depth <Derinlik>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				USnowAccumulation* Sim = FindSnowSim(World);
				if (!Sim || Args.Num() < 1)
				{
					UE_LOG(LogSurvivalWeather, Warning, TEXT("Kullanim: snow_set_depth <Derinlik>"));
					return;
				}
				Sim->SetSnowDepthForTesting(FCString::Atof(*Args[0]));
				UE_LOG(LogSurvivalWeather, Log, TEXT("snow_set_depth: %.1f"), Sim->GetCurrentSnowDepth());
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdSnowDump(
		TEXT("snow_dump"),
		TEXT("Mevcut kar derinligi/hareket carpani/insaat-engeli/cig-riski durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const USnowAccumulation* Sim = FindSnowSim(World);
				if (!Sim)
				{
					return;
				}
				UE_LOG(LogSurvivalWeather, Log,
					TEXT("Kar: derinlik=%.1f hareket-carpani=%.2f insaat-engeli=%s cig-riski=%s"),
					Sim->GetCurrentSnowDepth(), Sim->GetMovementSpeedMultiplier(),
					Sim->IsConstructionBlocked() ? TEXT("EVET") : TEXT("hayir"),
					Sim->IsInAvalancheRisk() ? TEXT("EVET") : TEXT("hayir"));
			}));
}
