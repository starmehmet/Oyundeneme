#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TaskSchedulerSettings.generated.h"

/**
 * Sistem #16 — Görev Planlayıcı ayarları. Project Settings → Game → Task Scheduler altında
 * görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Task Scheduler"))
class SURVIVALGAME_API UTaskSchedulerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTaskSchedulerSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** Bir görev başarısız olduktan (NPC hedefe varamadığından) SONRA yeniden atanabilir hale
	 * gelmeden önce beklenecek süre (sn). */
	UPROPERTY(EditAnywhere, Config, Category = "Task", meta = (ClampMin = "0.0"))
	float TaskFailureBackoffDuration = 30.0f;
};
