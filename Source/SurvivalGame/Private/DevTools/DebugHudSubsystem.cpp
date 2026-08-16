#include "DevTools/DebugHudSubsystem.h"
#include "SurvivalGame.h"
#include "Player/PlayerCharacter.h"
#include "Player/HealthComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Weather/WeatherSimulation.h"
#include "Weather/TemperatureSimulation.h"
#include "Weather/WeatherTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"

namespace
{
	const TCHAR* WeatherLabel(EWeatherCondition Condition)
	{
		switch (Condition)
		{
		case EWeatherCondition::Clear:        return TEXT("Acik");
		case EWeatherCondition::PartlyCloudy: return TEXT("Parcali Bulutlu");
		case EWeatherCondition::Overcast:     return TEXT("Kapali");
		case EWeatherCondition::Foggy:        return TEXT("Sisli");
		case EWeatherCondition::Rainy:        return TEXT("Yagmurlu");
		case EWeatherCondition::Stormy:       return TEXT("Firtinali");
		case EWeatherCondition::Snowing:      return TEXT("Kar Yagisi");
		case EWeatherCondition::Blizzard:     return TEXT("KAR FIRTINASI");
		case EWeatherCondition::Heatwave:     return TEXT("SICAK DALGASI");
		default:                              return TEXT("?");
		}
	}
}

void UDebugHudSubsystem::Tick(float DeltaTime)
{
	if (!bEnabled || !GEngine)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// Sabit key'ler: her satir ayni key ile yazilir ki her kare ust uste birikmesin, tazelensin.
	// Sure=0 -> tek kare (bir sonraki Tick yeniden yazar).

	// Hava durumu (Sistem #11)
	if (UWeatherSimulation* Weather = GI->GetSubsystem<UWeatherSimulation>())
	{
		const FWeatherState& State = Weather->GetCurrentState();
		const bool bLethal = (State.Condition == EWeatherCondition::Blizzard || State.Condition == EWeatherCondition::Heatwave);
		GEngine->AddOnScreenDebugMessage(1001, 0.0f, bLethal ? FColor::Orange : FColor::Cyan,
			FString::Printf(TEXT("Hava: %s   (%.0f C, ruzgar %.0f, yagis %%%.0f)"),
				WeatherLabel(State.Condition), State.Temperature, State.WindSpeed, State.Precipitation * 100.0f));
	}

	// Ortam sicakligi (Sistem #12)
	if (UTemperatureSimulation* Temp = GI->GetSubsystem<UTemperatureSimulation>())
	{
		GEngine->AddOnScreenDebugMessage(1002, 0.0f, FColor::White,
			FString::Printf(TEXT("Ortam sicakligi: %.1f C"), Temp->GetAmbientTemperature()));
	}

	// Oyuncu durumu
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
	if (!PlayerChar)
	{
		GEngine->AddOnScreenDebugMessage(1003, 0.0f, FColor::Silver, TEXT("(oyuncu pawn yok)"));
		return;
	}

	// Vucut sicakligi — mavi=hipotermi riski, kirmizi=asiri sicak, yesil=guvenli
	const float BodyC = PlayerChar->GetBodyTemperature();
	const FColor BodyColor = (BodyC < 35.0f) ? FColor(80, 140, 255) : (BodyC > 39.0f ? FColor::Red : FColor::Green);
	GEngine->AddOnScreenDebugMessage(1003, 0.0f, BodyColor,
		FString::Printf(TEXT("Vucut sicakligi: %.1f C  (guvenli 35-39)"), BodyC));

	// Can
	if (UHealthComponent* Health = PlayerChar->GetHealthComponent())
	{
		const float HP = Health->GetCurrentHealth();
		const FColor HpColor = (HP < 30.0f) ? FColor::Red : (HP < 60.0f ? FColor::Orange : FColor::Green);
		GEngine->AddOnScreenDebugMessage(1004, 0.0f, HpColor,
			FString::Printf(TEXT("Can: %.0f / %.0f"), HP, Health->GetMaxHealth()));
	}

	// Envanter agirligi
	if (UInventoryComponent* Inv = PlayerChar->GetInventoryComponent())
	{
		GEngine->AddOnScreenDebugMessage(1005, 0.0f, FColor::Yellow,
			FString::Printf(TEXT("Agirlik: %.1f / %.1f kg"), Inv->GetCurrentWeight(), Inv->GetMaxWeight()));
	}
}

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdDebugHud(
		TEXT("survival_debug_hud"),
		TEXT("Ekran-ustu survival durum gostergesini ac/kapat: survival_debug_hud [0|1] (arg yoksa toggle)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UDebugHudSubsystem* Sub = GI ? GI->GetSubsystem<UDebugHudSubsystem>() : nullptr;
				if (!Sub)
				{
					UE_LOG(LogSurvival, Warning, TEXT("survival_debug_hud: subsystem bulunamadi"));
					return;
				}
				const bool bEnable = Args.Num() > 0 ? (FCString::Atoi(*Args[0]) != 0) : !Sub->IsEnabled();
				Sub->SetEnabled(bEnable);
				UE_LOG(LogSurvival, Log, TEXT("survival_debug_hud: %s"), bEnable ? TEXT("ACIK") : TEXT("kapali"));
			}));
}
