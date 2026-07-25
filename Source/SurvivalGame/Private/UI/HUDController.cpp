#include "UI/HUDController.h"
#include "UI/ScreenBase.h"
#include "SurvivalGame.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

APlayerController* UHUDController::GetOwningPlayerController() const
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	return OwningPawn ? OwningPawn->GetController<APlayerController>() : nullptr;
}

bool UHUDController::IsWidgetActive(TSubclassOf<UUserWidget> WidgetClass) const
{
	for (const TObjectPtr<UUserWidget>& Widget : ActiveWidgets)
	{
		if (IsValid(Widget) && Widget->GetClass() == WidgetClass)
		{
			return true;
		}
	}
	return false;
}

UUserWidget* UHUDController::ShowWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("ShowWidget: WidgetClass==null, atlaniyor"));
		return nullptr;
	}

	for (const TObjectPtr<UUserWidget>& Widget : ActiveWidgets)
	{
		if (IsValid(Widget) && Widget->GetClass() == WidgetClass)
		{
			return Widget;
		}
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("ShowWidget: sahip PlayerController bulunamadi"));
		return nullptr;
	}

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (!NewWidget)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("ShowWidget: CreateWidget basarisiz (%s)"), *WidgetClass->GetName());
		return nullptr;
	}

	NewWidget->AddToViewport();
	ActiveWidgets.Add(NewWidget);
	NewWidget->OnNativeDestruct.AddUObject(this, &UHUDController::HandleWidgetDestructed);
	UE_LOG(LogSurvivalUI, Log, TEXT("ShowWidget: %s (aktif=%d)"), *WidgetClass->GetName(), ActiveWidgets.Num());
	return NewWidget;
}

void UHUDController::HideWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	for (int32 Index = 0; Index < ActiveWidgets.Num(); ++Index)
	{
		UUserWidget* Widget = ActiveWidgets[Index];
		if (!IsValid(Widget))
		{
			ActiveWidgets.RemoveAt(Index);
			--Index;
			continue;
		}
		if (Widget->GetClass() == WidgetClass)
		{
			// Kayittan ONCE cikar, SONRA RemoveFromParent cagir — RemoveFromParent SENKRON
			// olarak OnNativeDestruct'i (HandleWidgetDestructed) tetikler; ters sirada
			// yapilsaydi handler AYNI diziyi bu dongu HALA uzerinde calisirken mutasyona
			// ugratirdi (inceleme bulgusuyla ayni kok neden — bkz. HandleWidgetDestructed).
			ActiveWidgets.RemoveAt(Index);
			Widget->RemoveFromParent();
			UE_LOG(LogSurvivalUI, Log, TEXT("HideWidget: %s (aktif=%d)"), *WidgetClass->GetName(), ActiveWidgets.Num());
			return;
		}
	}
}

void UHUDController::HandleWidgetDestructed(UUserWidget* Widget)
{
	const int32 Index = ActiveWidgets.IndexOfByKey(Widget);
	if (Index == INDEX_NONE)
	{
		// Normal yol: HideWidget zaten kendisi cikarmisti, cift-isleme yok.
		return;
	}
	ActiveWidgets.RemoveAt(Index);
	UE_LOG(LogSurvivalUI, Log, TEXT("HandleWidgetDestructed: %s HUD'dan kendiligimden kaldirildi (aktif=%d)"),
		*Widget->GetClass()->GetName(), ActiveWidgets.Num());
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin ----

namespace
{
	UHUDController* FindPlayerHUDController(UWorld* World)
	{
		const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
		const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		return Pawn ? Pawn->FindComponentByClass<UHUDController>() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdHUDShowWidget(
		TEXT("hud_show_widget"),
		TEXT("UScreenBase'i (icerik yokken test icin taban sinifin kendisi) HUD'a ekler"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (UHUDController* HUD = FindPlayerHUDController(World))
				{
					HUD->ShowWidget(UScreenBase::StaticClass());
				}
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdHUDHideWidget(
		TEXT("hud_hide_widget"),
		TEXT("UScreenBase'i HUD'dan kaldirir"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (UHUDController* HUD = FindPlayerHUDController(World))
				{
					HUD->HideWidget(UScreenBase::StaticClass());
				}
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdHUDDump(
		TEXT("hud_dump"),
		TEXT("HUD aktif widget sayisini logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (const UHUDController* HUD = FindPlayerHUDController(World))
				{
					UE_LOG(LogSurvivalUI, Log, TEXT("HUD: aktif-widget=%d"), HUD->GetActiveWidgetCount());
				}
			}));
}
