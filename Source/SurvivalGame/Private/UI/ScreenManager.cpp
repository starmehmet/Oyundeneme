#include "UI/ScreenManager.h"
#include "UI/ScreenBase.h"
#include "UI/UIMath.h"
#include "SurvivalGame.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

void UScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalUI, Log, TEXT("ScreenManager hazir"));
}

UScreenBase* UScreenManager::PushScreen(TSubclassOf<UScreenBase> ScreenClass)
{
	if (!ScreenClass)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("PushScreen: ScreenClass==null, atlaniyor"));
		return nullptr;
	}

	APlayerController* PC = GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("PushScreen: PlayerController bulunamadi"));
		return nullptr;
	}

	UScreenBase* NewScreen = CreateWidget<UScreenBase>(PC, ScreenClass);
	if (!NewScreen)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("PushScreen: CreateWidget basarisiz (%s)"), *ScreenClass->GetName());
		return nullptr;
	}

	NewScreen->AddToViewport();
	ScreenStack.Add(NewScreen);
	NewScreen->OnNativeDestruct.AddUObject(this, &UScreenManager::HandleScreenDestructed);
	NewScreen->OnScreenOpened();
	ApplyInputModeForCurrentStack();

	UE_LOG(LogSurvivalUI, Log, TEXT("PushScreen: %s (yeni derinlik=%d)"), *ScreenClass->GetName(), ScreenStack.Num());
	return NewScreen;
}

void UScreenManager::PopScreen()
{
	if (ScreenStack.Num() == 0)
	{
		UE_LOG(LogSurvivalUI, Warning, TEXT("PopScreen: yigin zaten bos"));
		return;
	}

	UScreenBase* Top = ScreenStack.Pop();
	if (IsValid(Top))
	{
		Top->OnScreenClosed();
		Top->RemoveFromParent();
	}
	ApplyInputModeForCurrentStack();

	UE_LOG(LogSurvivalUI, Log, TEXT("PopScreen: yeni derinlik=%d"), ScreenStack.Num());
}

void UScreenManager::ApplyInputModeForCurrentStack()
{
	APlayerController* PC = GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	if (SurvivalUI::ShouldCaptureUIInput(ScreenStack.Num()))
	{
		// Inceleme bulgusu: SetWidgetToFocus verilmezse hicbir widget klavye/gamepad
		// odagina sahip olmaz — en ust ekrana aciktan odak veriliyor. PIE'de doGRULANDI:
		// SupportsKeyboardFocus() kontrolu OLMADAN icerik-bos UScreenBase (odaklanilacak
		// gercek alt widget'i yok) icin motor "Attempting to focus Non-Focusable widget"
		// HATASI logluyordu — yalnizca GERCEKTEN odak alabilen widget'lara odak veriliyor.
		FInputModeUIOnly InputMode;
		if (UScreenBase* Current = GetCurrentScreen())
		{
			const TSharedRef<SWidget> CurrentSlateWidget = Current->TakeWidget();
			if (CurrentSlateWidget->SupportsKeyboardFocus())
			{
				InputMode.SetWidgetToFocus(CurrentSlateWidget);
			}
		}
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

void UScreenManager::HandleScreenDestructed(UUserWidget* Widget)
{
	UScreenBase* AsScreen = Cast<UScreenBase>(Widget);
	const int32 Index = AsScreen ? ScreenStack.IndexOfByKey(AsScreen) : INDEX_NONE;
	if (Index == INDEX_NONE)
	{
		// Normal yol: PopScreen zaten kendisi cikarmisti, cift-isleme yok.
		return;
	}
	ScreenStack.RemoveAt(Index);
	ApplyInputModeForCurrentStack();
	UE_LOG(LogSurvivalUI, Log, TEXT("HandleScreenDestructed: %s yigindan kendiligimden cikarildi (yeni derinlik=%d)"),
		*Widget->GetClass()->GetName(), ScreenStack.Num());
}

// ---- Konsol komutlari: PIE/dev dogrulamasi icin ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdUIPushScreen(
		TEXT("ui_push_screen"),
		TEXT("UScreenBase'i (icerik yokken test icin taban sinifin kendisi) yigina ekler"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UScreenManager* Manager = GI ? GI->GetSubsystem<UScreenManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				Manager->PushScreen(UScreenBase::StaticClass());
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdUIPopScreen(
		TEXT("ui_pop_screen"),
		TEXT("Yigindaki en ust ekrani kaldirir"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				UScreenManager* Manager = GI ? GI->GetSubsystem<UScreenManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				Manager->PopScreen();
			}));

	FAutoConsoleCommandWithWorldAndArgs GCmdUIDump(
		TEXT("ui_dump"),
		TEXT("Ekran yigini durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				const UScreenManager* Manager = GI ? GI->GetSubsystem<UScreenManager>() : nullptr;
				if (!Manager)
				{
					return;
				}
				const UScreenBase* Current = Manager->GetCurrentScreen();
				const APlayerController* PC = GI ? GI->GetFirstLocalPlayerController() : nullptr;
				UE_LOG(LogSurvivalUI, Log,
					TEXT("UI: derinlik=%d ust-ekran=%s fare-imleci=%s"),
					Manager->GetScreenStackDepth(),
					Current ? *Current->GetClass()->GetName() : TEXT("(yok)"),
					(PC && PC->ShouldShowMouseCursor()) ? TEXT("gorunur") : TEXT("gizli"));
			}));
}
