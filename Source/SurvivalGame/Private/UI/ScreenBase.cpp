#include "UI/ScreenBase.h"
#include "SurvivalGame.h"

void UScreenBase::OnScreenOpened_Implementation()
{
	UE_LOG(LogSurvivalUI, Verbose, TEXT("Ekran acildi: %s"), *GetClass()->GetName());
}

void UScreenBase::OnScreenClosed_Implementation()
{
	UE_LOG(LogSurvivalUI, Verbose, TEXT("Ekran kapandi: %s"), *GetClass()->GetName());
}
