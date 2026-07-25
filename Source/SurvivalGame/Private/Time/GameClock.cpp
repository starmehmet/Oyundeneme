#include "Time/GameClock.h"
#include "Time/TimeKeeper.h"
#include "Engine/GameInstance.h"

UTimeKeeper* UGameClock::GetTimeKeeper() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UTimeKeeper>() : nullptr;
}

int32 UGameClock::GetCurrentMinute() const
{
	const UTimeKeeper* Keeper = GetTimeKeeper();
	return Keeper ? Keeper->GetMinuteOfDay() : 0;
}

int32 UGameClock::GetCurrentHour() const
{
	const UTimeKeeper* Keeper = GetTimeKeeper();
	return Keeper ? Keeper->GetHourOfDay() : 0;
}

int32 UGameClock::GetDayNumber() const
{
	const UTimeKeeper* Keeper = GetTimeKeeper();
	return Keeper ? Keeper->GetDayNumber() : 0;
}

FText UGameClock::GetTimeText() const
{
	const int32 Minute = GetCurrentMinute();
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minute / 60, Minute % 60));
}
