#include "Weather/WeatherSimulation.h"
#include "Weather/WeatherMath.h"
#include "Weather/WeatherProfile.h"
#include "Weather/WeatherProfileDatabase.h"
#include "Weather/WeatherSimulationSettings.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UWeatherSimulation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalWeather, Log, TEXT("WeatherSimulation hazir"));
}

bool UWeatherSimulation::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UWeatherSimulation::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void UWeatherSimulation::Tick(float DeltaTime)
{
	const UWeatherSimulationSettings* Settings = GetDefault<UWeatherSimulationSettings>();

	// Ilk Tick'te HEMEN degerlendir (oyun 30sn boyunca sabit varsayilan degerlerle
	// baslamasin), sonrasinda DoD'nin "30sn'de bir" periyoduna sadik kalinir.
	//
	// TransitionProgress>=1.0f sarti KASITLI: bir gecis SURERKEN yeniden degerlendirme
	// YAPILMAZ — aksi halde RollNextWeather, CurrentState.Condition'i (gecis surerken hala
	// ESKI durum, bkz. LerpWeatherState) yeniden secebilir ve BeginTransitionTo'yu erken
	// tetikleyip devam eden gecisi TERSINE cevirir/sarsar (inceleme bulgusu — varsayilan
	// ayarlarda TransitionDuration<EvaluationInterval oldugu icin erisilemez ama yanlis
	// yapilandirma/ciddi kare takilmasinda gercek). Geciken degerlendirme KAYBOLMAZ — gecis
	// tamamlanir tamamlanmaz bir sonraki Tick'te hemen calisir (TimeSinceLastEvaluation
	// birikmeye devam ediyor).
	TimeSinceLastEvaluation += DeltaTime;
	if (TransitionProgress >= 1.0f && (!bHasEvaluatedOnce || TimeSinceLastEvaluation >= Settings->EvaluationInterval))
	{
		bHasEvaluatedOnce = true;
		TimeSinceLastEvaluation = 0.0f;
		EvaluateWeather();
	}

	if (TransitionProgress < 1.0f)
	{
		TransitionElapsed += DeltaTime;
		TransitionProgress = SurvivalWeather::ComputeTransitionProgress(TransitionElapsed, Settings->TransitionDuration);
		CurrentState = SurvivalWeather::LerpWeatherState(TransitionStartState, TargetState, TransitionProgress);
	}

	// Kitlik alarmindan (Sistem #10) FARKLI olarak BILEREK her Tick'te yayinlanir — hava
	// degerleri surekli-anlamli bilgidir (bkz. sinif yorumu).
	OnWeatherStateChanged.Broadcast(CurrentState);
}

void UWeatherSimulation::EvaluateWeather()
{
	const EWeatherCondition NextCondition = RollNextWeather();

	// Ilk degerlendirme: CurrentState struct-varsayilanlarinda (veritabanindan HIC doldurulmamis),
	// TransitionProgress da varsayilan 1.0f oldugu icin "zaten oraya gecis tamamlanmis" gibi
	// GORUNUR — bu yuzden bHasInitializedState kontrolu olmadan RollNextWeather ayni durumu
	// (ör. varsayilan Clear) dondurdugunde asagidaki erken-donus BeginTransitionTo'yu hic
	// cagirmadan CurrentState'i kalici olarak sahte varsayilanlarda birakirdi (inceleme bulgusu).
	if (bHasInitializedState && NextCondition == CurrentState.Condition && TransitionProgress >= 1.0f)
	{
		return; // gercekten zaten bu durumdayiz -> gecise gerek yok
	}

	BeginTransitionTo(NextCondition);
	bHasInitializedState = true;
}

void UWeatherSimulation::BeginTransitionTo(EWeatherCondition NewCondition)
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherProfileDatabase* DB = GI ? GI->GetSubsystem<UWeatherProfileDatabase>() : nullptr;
	FWeatherProfile Profile;
	if (!DB || !DB->FindProfile(NewCondition, Profile))
	{
		UE_LOG(LogSurvivalWeather, Warning, TEXT("BeginTransitionTo: '%d' icin profil bulunamadi"), static_cast<int32>(NewCondition));
		return;
	}

	TransitionStartState = CurrentState;

	TargetState.Condition = NewCondition;
	TargetState.Temperature = Profile.Temperature;
	TargetState.Humidity = Profile.Humidity;
	TargetState.WindSpeed = Profile.WindSpeed;
	TargetState.WindDirection = SurvivalWeather::ComputeWindDirection(FMath::FRand());
	TargetState.VisibilityDistance = Profile.VisibilityDistance;
	TargetState.Precipitation = Profile.Precipitation;

	TransitionElapsed = 0.0f;
	TransitionProgress = 0.0f;

	UE_LOG(LogSurvivalWeather, Log, TEXT("Hava durumu gecisi basladi: %d -> %d"),
		static_cast<int32>(CurrentState.Condition), static_cast<int32>(NewCondition));
}

EWeatherCondition UWeatherSimulation::RollNextWeather() const
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherProfileDatabase* DB = GI ? GI->GetSubsystem<UWeatherProfileDatabase>() : nullptr;
	if (!DB)
	{
		return CurrentState.Condition;
	}

	const TArray<EWeatherCondition> Conditions = DB->GetAllConditions();
	if (Conditions.Num() == 0)
	{
		return CurrentState.Condition;
	}

	const UWeatherSimulationSettings* Settings = GetDefault<UWeatherSimulationSettings>();

	TArray<float> Weights;
	Weights.Reserve(Conditions.Num());
	for (const EWeatherCondition Condition : Conditions)
	{
		FWeatherProfile Profile;
		float Weight = DB->FindProfile(Condition, Profile) ? Profile.BaseWeight : 0.0f;
		if (Condition == CurrentState.Condition)
		{
			Weight *= Settings->PersistenceMultiplier;
		}
		Weights.Add(Weight);
	}

	const int32 Index = SurvivalWeather::SelectWeightedIndex(Weights, FMath::FRand());
	return Conditions.IsValidIndex(Index) ? Conditions[Index] : CurrentState.Condition;
}

bool UWeatherSimulation::ForceWeather(EWeatherCondition NewCondition)
{
	const UGameInstance* GI = GetGameInstance();
	const UWeatherProfileDatabase* DB = GI ? GI->GetSubsystem<UWeatherProfileDatabase>() : nullptr;
	FWeatherProfile Profile;
	if (!DB || !DB->FindProfile(NewCondition, Profile))
	{
		return false;
	}

	BeginTransitionTo(NewCondition);
	TimeSinceLastEvaluation = 0.0f; // degerlendirme dongusunu de sifirla — hemen ust uste ikinci (rastgele) bir gecis olmasin
	return true;
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi) ----

namespace
{
	UWeatherSimulation* FindWeatherSim(UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<UWeatherSimulation>()
			: nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdWeatherForce(
		TEXT("weather_force"),
		TEXT("Hava durumunu hemen degistir: weather_force <0-8> (bkz. EWeatherCondition: 0=Clear..8=Heatwave)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UWeatherSimulation* Sim = FindWeatherSim(World);
				if (!Sim || Args.Num() < 1)
				{
					UE_LOG(LogSurvivalWeather, Warning, TEXT("Kullanim: weather_force <0-8>"));
					return;
				}
				const int32 Value = FMath::Clamp(FCString::Atoi(*Args[0]), 0, 8);
				const bool bForced = Sim->ForceWeather(static_cast<EWeatherCondition>(Value));
				UE_LOG(LogSurvivalWeather, Log, TEXT("weather_force %d: %s"), Value, bForced ? TEXT("basarili") : TEXT("basarisiz (profil bulunamadi)"));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdWeatherDump(
		TEXT("weather_dump"),
		TEXT("Mevcut hava durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UWeatherSimulation* Sim = FindWeatherSim(World);
				if (!Sim)
				{
					return;
				}
				const FWeatherState& S = Sim->GetCurrentState();
				UE_LOG(LogSurvivalWeather, Log,
					TEXT("Hava: durum=%d sicaklik=%.1f nem=%.2f ruzgar=%.1f gorusMesafesi=%.0f yagis=%.2f | gecisIlerlemesi=%.2f"),
					static_cast<int32>(S.Condition), S.Temperature, S.Humidity, S.WindSpeed,
					S.VisibilityDistance, S.Precipitation, Sim->GetTransitionProgress());
			}));
}

void UWeatherSimulation::RestoreStateForLoad(const FWeatherState& InCurrent, const FWeatherState& InTransStart,
	const FWeatherState& InTarget, float InTransProgress, float InTransElapsed, float InTimeSinceEval)
{
	CurrentState = InCurrent;
	TransitionStartState = InTransStart;
	TargetState = InTarget;
	TransitionProgress = InTransProgress;
	TransitionElapsed = InTransElapsed;
	TimeSinceLastEvaluation = InTimeSinceEval;
	bHasEvaluatedOnce = true;
	bHasInitializedState = true;
}
