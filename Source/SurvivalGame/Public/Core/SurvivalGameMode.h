#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalGameMode.generated.h"

/**
 * Ana OYNANIŞ modu. Config/DefaultEngine.ini GlobalDefaultGameMode bunu işaret eder.
 * Ana menü haritası bu modu KULLANMAMALI (düz AGameModeBase kullanın) — saat kapısı
 * bu modun StartPlay/EndPlay'ine bağlı: menüde saat durur, oynanışta akar.
 */
UCLASS()
class SURVIVALGAME_API ASurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivalGameMode();

	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
