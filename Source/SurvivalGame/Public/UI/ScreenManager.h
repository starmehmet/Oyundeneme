#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScreenManager.generated.h"

class UScreenBase;
class UUserWidget;

/**
 * Sistem #20 — Modal ekran yigininin (duraklatma menusu, secenekler vb.) tek sahibi.
 * `UAudioManager`/`UWorldPartitionHelper` ile ayni "Singleton=Subsystem" karari
 * (`UGameInstanceSubsystem`) — tek-oyunculu varsayimla `GetFirstLocalPlayerController`
 * kullanilir (bu projenin bastan beri gecerli tek-oyunculu varsayimi, bkz. TimeKeeper ADR).
 * Giris yonlendirmesi (MIMARI "UI vs oyun"): yigin BOSTAN dolmaya gecince
 * `FInputModeUIOnly`+fare imleci, yigin TAMAMEN bosalinca `FInputModeGameOnly`+imlec gizli —
 * karar `SurvivalUI::ShouldCaptureUIInput`'tan (saf fonksiyon) okunur.
 */
UCLASS()
class SURVIVALGAME_API UScreenManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Yeni bir ekranu yigina ekler (olusturur+viewport'a ekler), giris modunu gunceller. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UScreenBase* PushScreen(TSubclassOf<UScreenBase> ScreenClass);

	/** Yigindaki EN UST ekrani kaldirir (varsa), giris modunu gunceller. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopScreen();

	UFUNCTION(BlueprintPure, Category = "UI")
	UScreenBase* GetCurrentScreen() const { return ScreenStack.Num() > 0 ? ScreenStack.Last() : nullptr; }

	UFUNCTION(BlueprintPure, Category = "UI")
	int32 GetScreenStackDepth() const { return ScreenStack.Num(); }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TArray<TObjectPtr<UScreenBase>> ScreenStack;

private:
	void ApplyInputModeForCurrentStack();

	/** Inceleme bulgusu (kritik): bir ekran icerigi `PopScreen` DISINDA (ör. kendi kapat
	 * dugmesinden dogrudan `RemoveFromParent` ile) kapanirsa, `IsValid()` bunu ASLA fark
	 * edemez (widget hala gecerli bir UObject, yalnizca Slate'ten kopuk) — HER widget'in
	 * sahip oldugu `OnNativeDestruct` olayina (motor kaynagindan dogrulandi: `RemoveFromParent`
	 * SENKRON tetikler) abone olup etkilenen ekrani ANINDA yigindan cikarir. */
	void HandleScreenDestructed(UUserWidget* Widget);
};
