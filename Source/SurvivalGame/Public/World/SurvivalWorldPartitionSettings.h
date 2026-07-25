#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SurvivalWorldPartitionSettings.generated.h"

/**
 * Sistem #18 — Dünya Bölümlendirme ayarları. Project Settings → Game → World Partition
 * altında görünür.
 *
 * Dosya adı BİLEREK `SurvivalWorldPartitionSettings.h` (`WorldPartitionSettings.h` DEĞİL) —
 * UE 5.8'in kendi native World Partition modülünde `Engine/Public/WorldPartition/
 * WorldPartitionSettings.h` ZATEN var; UHT header adlarının TÜM proje+motor genelinde
 * BENZERSİZ olmasını gerektiriyor (derleme hatasından ampirik olarak keşfedildi — sınıf
 * adı `USurvivalWorldPartitionSettings` çakışmıyor, yalnızca DOSYA adı çakışıyordu).
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "World Partition"))
class SURVIVALGAME_API USurvivalWorldPartitionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	USurvivalWorldPartitionSettings()
	{
		CategoryName = TEXT("Game");
	}

	/** MIMARI.md varsayılanı. */
	UPROPERTY(EditAnywhere, Config, Category = "WorldPartition", meta = (ClampMin = "100.0"))
	float CellSize = 6400.0f;

	UPROPERTY(EditAnywhere, Config, Category = "WorldPartition", meta = (ClampMin = "0.0"))
	float LoadRadius = 8000.0f;

	/** `UnloadRadius` HER ZAMAN `LoadRadius`'tan büyük olmalı — histerezis (bkz.
	 * `SurvivalWorldPartition::ShouldCellBeLoaded/Unloaded`). */
	UPROPERTY(EditAnywhere, Config, Category = "WorldPartition", meta = (ClampMin = "0.0"))
	float UnloadRadius = 12000.0f;

	/**
	 * Hücre yeniden-tarama + yükle/boşalt değerlendirme periyodu (sn). İnceleme bulgusu:
	 * bu, ani/uzun bir ışınlanmadan (henüz bu oyunda YOK — hızlı-seyahat/yeniden-doğma gibi
	 * bir özellik yok) sonra hücrelerin en fazla bu kadar süre BAYAT (gizli/collision-kapalı)
	 * kalabileceği anlamına gelir; normal yürüyüş hızında (LoadRadius/UnloadRadius'a göre
	 * ÇOK küçük) bu pratikte fark edilmez. Gerçek bir ışınlanma özelliği eklenince, o
	 * özellik kendi kod yolunda `EvaluateCellLoading`'i ELLE tetikleyebilir — burada
	 * spekülatif bir "ani-sıçrama algılama" mekanizması İCAT EDİLMEDİ (bkz. ADR).
	 */
	UPROPERTY(EditAnywhere, Config, Category = "WorldPartition", meta = (ClampMin = "0.1"))
	float EvaluationInterval = 1.0f;
};
