#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenBase.generated.h"

/**
 * Sistem #20 — Yigina eklenebilen (modal) her ekranin (duraklatma menusu, secenekler,
 * kaydet/yukle) taban sinifi. BILEREK soyut DEGIL (`Abstract` degil) — icerik (WBP_*) henuz
 * yokken bile `UScreenManager::PushScreen(UScreenBase::StaticClass())` ile PIE'de gercek,
 * ornekleneblir bir widget olarak test edilebilir (BuildingBase'in icerik olmadan test
 * edilebilir olmasi ADR'siyle ayni gerekce). Gercek gorsel duzen (WBP_PauseMenu vb.) bu
 * sinifi Blueprint'te miras alip icerik fazinda eklenir.
 */
UCLASS()
class SURVIVALGAME_API UScreenBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** UScreenManager::PushScreen bu ekran yigina eklendiginde COAGRIR. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnScreenOpened();
	virtual void OnScreenOpened_Implementation();

	/** UScreenManager::PopScreen bu ekran yigindan cikarildiginda cagirir. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI")
	void OnScreenClosed();
	virtual void OnScreenClosed_Implementation();
};
