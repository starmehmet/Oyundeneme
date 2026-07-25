#include "Time/TimeKeeper.h"
#include "Time/TimeMath.h"
#include "Time/TimeSettings.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

void UTimeKeeper::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ApplyStartTimeFromSettings();

	UE_LOG(LogSurvival, Log, TEXT("TimeKeeper hazir: gun %d, %02d:%02d, olcek x%.1f (saat durdurulmus)"),
		LastBroadcastDay, LastBroadcastHour, LastBroadcastMinute % 60, TimeScale);
}

void UTimeKeeper::ApplyStartTimeFromSettings()
{
	const UTimeSettings* Settings = GetDefault<UTimeSettings>();
	TimeScale = Settings->TimeScale;
	TotalGameSeconds = Settings->StartHour * SurvivalTime::SecondsPerHour;

	LastBroadcastMinute = SurvivalTime::MinuteOfDay(TotalGameSeconds);
	LastBroadcastHour = SurvivalTime::HourOfDay(TotalGameSeconds);
	LastBroadcastDay = SurvivalTime::DayNumber(TotalGameSeconds);
}

void UTimeKeeper::StartClock()
{
	if (!bClockRunning)
	{
		bClockRunning = true;
		UE_LOG(LogSurvival, Log, TEXT("Saat basladi (gun %d, %02d:%02d)"),
			GetDayNumber(), GetHourOfDay(), GetMinuteOfDay() % 60);
	}
}

void UTimeKeeper::StopClock()
{
	if (bClockRunning)
	{
		bClockRunning = false;
		UE_LOG(LogSurvival, Log, TEXT("Saat durdu (gun %d, %02d:%02d)"),
			GetDayNumber(), GetHourOfDay(), GetMinuteOfDay() % 60);
	}
}

void UTimeKeeper::ResetForNewGame()
{
	ApplyStartTimeFromSettings();
	UE_LOG(LogSurvival, Log, TEXT("Saat yeni oyun icin sifirlandi: gun 0, %02d:00"),
		GetDefault<UTimeSettings>()->StartHour);
}

bool UTimeKeeper::IsTickable() const
{
	if (IsTemplate() || !bClockRunning)
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UTimeKeeper::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void UTimeKeeper::Tick(float DeltaTime)
{
	TotalGameSeconds += static_cast<double>(DeltaTime) * TimeScale;
	BroadcastRollovers();
}

int32 UTimeKeeper::GetMinuteOfDay() const
{
	return SurvivalTime::MinuteOfDay(TotalGameSeconds);
}

int32 UTimeKeeper::GetHourOfDay() const
{
	return SurvivalTime::HourOfDay(TotalGameSeconds);
}

int32 UTimeKeeper::GetDayNumber() const
{
	return SurvivalTime::DayNumber(TotalGameSeconds);
}

float UTimeKeeper::GetMinuteOfDayFloat() const
{
	return static_cast<float>(SurvivalTime::MinuteOfDayFloat(TotalGameSeconds));
}

void UTimeKeeper::SetTimeScale(float NewScale)
{
	TimeScale = FMath::Clamp(NewScale, 0.0f, 1000.0f);
	UE_LOG(LogSurvival, Log, TEXT("Zaman olcegi: x%.1f"), TimeScale);
}

void UTimeKeeper::SetTimeOfDay(int32 Hour, int32 Minute)
{
	Hour = FMath::Clamp(Hour, 0, 23);
	Minute = FMath::Clamp(Minute, 0, 59);
	const int32 Day = SurvivalTime::DayNumber(TotalGameSeconds);
	SetTotalGameSeconds(Day * SurvivalTime::SecondsPerDay
		+ Hour * SurvivalTime::SecondsPerHour
		+ Minute * SurvivalTime::SecondsPerMinute);
}

void UTimeKeeper::SetTotalGameSeconds(double NewTotal)
{
	TotalGameSeconds = FMath::Max(0.0, NewTotal);
	BroadcastRollovers();
}

void UTimeKeeper::BroadcastRollovers()
{
	const int32 Minute = SurvivalTime::MinuteOfDay(TotalGameSeconds);
	const int32 Hour = SurvivalTime::HourOfDay(TotalGameSeconds);
	const int32 Day = SurvivalTime::DayNumber(TotalGameSeconds);

	if (Minute != LastBroadcastMinute)
	{
		LastBroadcastMinute = Minute;
		OnMinuteChanged.Broadcast(Minute);
	}
	if (Hour != LastBroadcastHour)
	{
		LastBroadcastHour = Hour;
		OnHourChanged.Broadcast(Hour);
	}
	if (Day != LastBroadcastDay)
	{
		LastBroadcastDay = Day;
		OnDayChanged.Broadcast(Day);
	}
}

// ---- Konsol komutlari (YOL_HARITASI Hafta 1-2: dev araclarinin one alinan kismi) ----

namespace
{
	UTimeKeeper* FindTimeKeeper(UWorld* World)
	{
		return (World && World->GetGameInstance())
			? World->GetGameInstance()->GetSubsystem<UTimeKeeper>()
			: nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdTimeSetHour(
		TEXT("time_sethour"),
		TEXT("Oyun saatini ayarla: time_sethour <0-23> [dakika]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UTimeKeeper* Keeper = FindTimeKeeper(World);
				if (!Keeper || Args.Num() < 1)
				{
					UE_LOG(LogSurvival, Warning, TEXT("Kullanim: time_sethour <0-23> [dakika]"));
					return;
				}
				const int32 Hour = FCString::Atoi(*Args[0]);
				const int32 Minute = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 0;
				Keeper->SetTimeOfDay(Hour, Minute);
				UE_LOG(LogSurvival, Log, TEXT("Saat ayarlandi: %02d:%02d"),
					Keeper->GetHourOfDay(), Keeper->GetMinuteOfDay() % 60);
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdTimeSetScale(
		TEXT("time_settimescale"),
		TEXT("Zaman olcegini ayarla: time_settimescale <0-1000>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UTimeKeeper* Keeper = FindTimeKeeper(World);
				if (!Keeper || Args.Num() < 1)
				{
					UE_LOG(LogSurvival, Warning, TEXT("Kullanim: time_settimescale <0-1000>"));
					return;
				}
				Keeper->SetTimeScale(FCString::Atof(*Args[0]));
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdTimeDump(
		TEXT("time_dump"),
		TEXT("Mevcut oyun zamanini logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (const UTimeKeeper* Keeper = FindTimeKeeper(World))
				{
					UE_LOG(LogSurvival, Log, TEXT("Gun %d - %02d:%02d (olcek x%.1f, calisiyor: %s, toplam %.0f sn)"),
						Keeper->GetDayNumber(), Keeper->GetHourOfDay(), Keeper->GetMinuteOfDay() % 60,
						Keeper->GetTimeScale(), Keeper->IsClockRunning() ? TEXT("evet") : TEXT("hayir"),
						Keeper->GetTotalGameSeconds());
				}
			}));
}
