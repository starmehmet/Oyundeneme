#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "DebugHudSubsystem.generated.h"

/**
 * Sistem #22 (Dev araçları) — ekran-üstü survival durum göstergesi.
 *
 * Gerçek görsel HUD/VFX içeriği henüz yok (placeholder aşaması), bu yüzden oynanışta hava/
 * sıcaklık/can gibi çekirdek sistemlerin etkisi oyuncuya GÖRÜNMÜYOR ("weather_force 7 hiçbir
 * şey yapmıyor" gibi görünür — oysa simülasyon çalışıyor, yalnızca görsel temsili yok). Bu
 * subsystem her kare `GEngine->AddOnScreenDebugMessage` ile temel durumu (hava, ortam/vücut
 * sıcaklığı, can, ağırlık) çizer — böylece sistemler test/oynanışta görünür olur.
 *
 * Varsayılan AÇIK: konsola gerek kalmadan hemen görünür. `survival_debug_hud [0|1]` ile
 * açılıp kapanır. Yalnızca Development/Editor'de derlenir (AddOnScreenDebugMessage Shipping'de
 * `ENABLE_DRAW_DEBUG` ile elenir) — ama subsystem/komut çalışır, çizim no-op olur.
 */
UCLASS()
class SURVIVALGAME_API UDebugHudSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }
	bool IsEnabled() const { return bEnabled; }

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDebugHudSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return bEnabled; }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

private:
	// Varsayilan ACIK — placeholder asamasinda sistemlerin gorunur olmasi icin (bkz. sinif yorumu).
	bool bEnabled = true;
};
