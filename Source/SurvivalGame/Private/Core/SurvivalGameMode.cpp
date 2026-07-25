#include "Core/SurvivalGameMode.h"
#include "Core/SurvivalPlayerController.h"
#include "Player/PlayerCharacter.h"
#include "Time/TimeKeeper.h"
#include "Time/GameClock.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"

ASurvivalGameMode::ASurvivalGameMode()
{
	// Editörde BP_PlayerController / BP_PlayerCharacter alt siniflari (Enhanced Input
	// varliklari atanmis) bu varsayilanlarin yerini almali — bkz. README kurulum notu.
	PlayerControllerClass = ASurvivalPlayerController::StaticClass();
	DefaultPawnClass = APlayerCharacter::StaticClass();
}

void ASurvivalGameMode::StartPlay()
{
	Super::StartPlay();

	const UGameInstance* GI = GetGameInstance();
	if (UTimeKeeper* Keeper = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr)
	{
		Keeper->StartClock();
	}

	if (const UGameClock* Clock = GI ? GI->GetSubsystem<UGameClock>() : nullptr)
	{
		UE_LOG(LogSurvival, Log, TEXT("Oyun basladi - gun %d, saat %s"),
			Clock->GetDayNumber(), *Clock->GetTimeText().ToString());
	}
}

void ASurvivalGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Oynanis dunyasi kapanirken saati durdur — menuye donuste zaman akmaya devam etmesin
	const UGameInstance* GI = GetGameInstance();
	if (UTimeKeeper* Keeper = GI ? GI->GetSubsystem<UTimeKeeper>() : nullptr)
	{
		Keeper->StopClock();
	}

	Super::EndPlay(EndPlayReason);
}
