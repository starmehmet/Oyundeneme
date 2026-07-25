#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameClock.generated.h"

class UTimeKeeper;

/**
 * Sistem #1 — Diğer sistemlerin zamana baktığı ince cephe (facade).
 * Durum tutmaz; TimeKeeper'a yönlendirir. UI ve Blueprint tarafı bunu kullanır,
 * TimeKeeper'ın iç API'sine bağımlılık kurulmaz.
 */
UCLASS()
class SURVIVALGAME_API UGameClock : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Günün dakikası [0, 1439]. */
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentMinute() const;

	/** Günün saati [0, 23]. */
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetCurrentHour() const;

	/** Gün numarası, 0'dan başlar. */
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetDayNumber() const;

	/** "08:35" biçiminde saat metni — HUD saat göstergesi için. */
	UFUNCTION(BlueprintPure, Category = "Time")
	FText GetTimeText() const;

private:
	UTimeKeeper* GetTimeKeeper() const;
};
