#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUDController.generated.h"

class UUserWidget;

/**
 * Sistem #20 — Kalici HUD elemanlarinin (envanter paneli, makine durumu vb.) sahibi.
 * `UScreenManager`'in AKSINE bir YIGIN DEGIL — birbirinden bagimsiz, ayni anda birden fazla
 * acik olabilen widget'lar (modal olmayan). `UCameraManager` ile ayni desen: davranis
 * bilesen olarak `APlayerCharacter`'a eklenir (bkz. PlayerCharacter.h).
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UHUDController : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Ayni siniftan zaten aktif bir widget varsa YENIDEN OLUSTURMAZ, mevcut olani doner. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideWidget(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintPure, Category = "UI")
	int32 GetActiveWidgetCount() const { return ActiveWidgets.Num(); }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsWidgetActive(TSubclassOf<UUserWidget> WidgetClass) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TArray<TObjectPtr<UUserWidget>> ActiveWidgets;

private:
	class APlayerController* GetOwningPlayerController() const;

	/** Inceleme bulgusu (kritik, ScreenManager'daki ayni kok nedenin HUD gorunumu): bir widget
	 * `HideWidget` DISINDA (ör. kendi kapat dugmesinden dogrudan `RemoveFromParent` ile)
	 * kapanirsa, `IsValid()` bunu fark edemez — HER widget'in `OnNativeDestruct` olayina
	 * abone olup etkilenen kaydi ANINDA temizler. */
	void HandleWidgetDestructed(UUserWidget* Widget);
};
