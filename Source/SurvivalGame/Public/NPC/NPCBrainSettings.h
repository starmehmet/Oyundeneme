#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NPCBrainSettings.generated.h"

/**
 * Sistem #15 — NPC Yapay Zeka ayarları. Project Settings → Game → NPC Brain altında görünür.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "NPC Brain"))
class SURVIVALGAME_API UNPCBrainSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNPCBrainSettings()
	{
		CategoryName = TEXT("Game");
	}

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0"))
	float MaxFatigue = 100.0f;

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0"))
	float WorkFatigueRate = 2.0f;

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0"))
	float SleepRecoveryRate = 5.0f;

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0"))
	float MoraleRecoveryRate = 0.05f;

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0"))
	float MoraleDecayRate = 0.1f;

	/** Yorgunluk oranı (Fatigue/MaxFatigue) bunu aşarsa "aşırı çalışma" sayılır, moral düşer. */
	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OverworkFatigueRatio = 0.8f;

	/** Can, MaxHealth'in bu oranının altında/eşitse NPC "Hurt" sayılır. */
	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HurtThresholdFraction = 0.3f;

	/** Görev hedefine varmış sayılma yarıçapı (UU). */
	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "1.0"))
	float TaskAcceptanceRadius = 150.0f;

	/** Uyku başlangıç/bitiş saatleri (0-23) — `SleepStartHour > SleepEndHour` gece-yarısı
	 * sarmalını doğru ele alır (bkz. `SurvivalNPC::WantsToSleep`). */
	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0", ClampMax = "23"))
	int32 SleepStartHour = 22;

	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "0", ClampMax = "23"))
	int32 SleepEndHour = 6;

	/** Sistem #16 — NPC bu süreden UZUN süre Walking durumunda kalıp hedefe VARAMAZSA (yol
	 * bulunamadı/tıkalı), görev BAŞARISIZ sayılır (`OnTaskFailed`) ve iş bırakılır. */
	UPROPERTY(EditAnywhere, Config, Category = "NPC", meta = (ClampMin = "1.0"))
	float MaxWalkingDuration = 30.0f;
};
