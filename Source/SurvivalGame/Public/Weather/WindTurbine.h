#pragma once

#include "CoreMinimal.h"
#include "Production/ProductionMachine.h"
#include "WindTurbine.generated.h"

/**
 * Sistem #13 — MIMARI.md'nin `AWindTurbine : AProductionMachine` taslağı. Rüzgar hızından
 * enerji üreten bir yapı; `AProductionMachine`'in tarif/girdi/çıktı makinesini KULLANMAZ —
 * `AdvanceProduction`/`GetProgress`'i override edip kendi rüzgar→enerji mantığını uygular,
 * ama `UProductionManager`'ın zaten var olan kare-bölümlemeli tetikleme altyapısını
 * (BeginPlay/EndPlay'deki Register/UnregisterMachine, taban sınıftan miras) OLDUĞU GİBİ
 * kullanır — yeni bir tick sistemi YAZILMADI (bkz. ADR).
 *
 * Ürettiği enerji `UResourceSimulation::RegisterEnergyProducer` ile Sistem #10'un enerji
 * bütçesine kaydedilir — türbine özel, paralel bir enerji-muhasebesi YOK, her
 * `AdvanceProduction` çağrısında kayıt GÜNCELLENİR (aynı anahtarla `Add` üzerine yazar).
 *
 * Miras alınan `InputBuffer`/`OutputBuffer`/`AvailableRecipeIDs` KULLANILMAZ (boş kalır) —
 * taban sınıfın constructor'ını değiştirmeden en küçük deviasyon.
 */
UCLASS()
class SURVIVALGAME_API AWindTurbine : public AProductionMachine
{
	GENERATED_BODY()

public:
	AWindTurbine();

	virtual void AdvanceProduction(double CurrentGameTime) override;
	virtual float GetProgress() const override;

	/** InputBuffer/OutputBuffer hic KULLANILMADIGINDAN (bkz. sinif yorumu) Faz 1 entegrasyonunun
	 * lojistik-agi kayit proxy'leri (AProductionMachine::BeginPlay) burada BILEREK devre disi —
	 * kalici bos arabellekler agi kirletip gercek malzemeleri sessizce yutmasin. */
	virtual bool ShouldRegisterLogisticsNodes() const override { return false; }

	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetCurrentOutput() const { return CurrentOutput; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Wind", meta = (ClampMin = "0.0"))
	float CutInSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Wind", meta = (ClampMin = "0.0"))
	float RatedSpeed = 12.0f;

	/** Rüzgar `RatedSpeed`'e ulaştığında/üstünde üretilen enerji (`UResourceSimulation`'ın
	 * genel "birim/sn" ölçeğinde — gerçek Watt değil, Sistem #10'un diğer üreticileriyle aynı
	 * ölçek). */
	UPROPERTY(EditAnywhere, Category = "Wind", meta = (ClampMin = "0.0"))
	float RatedOutput = 50.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	float CurrentOutput = 0.0f;
};
