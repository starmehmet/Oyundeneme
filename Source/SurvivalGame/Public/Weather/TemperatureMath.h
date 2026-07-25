#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #12 — Sıcaklık Simülasyonu: saf, durum tutmayan gün/gece salınımı, iç-mekan
 * karışımı, vücut sıcaklığı sürüklenmesi ve termal hasar mantığı. Yan etkisiz; birim
 * testleri Private/Tests/TemperatureSimulationMathTests.cpp.
 */
namespace SurvivalTemperature
{
	/**
	 * Gün içindeki saate göre [-1,1] salınım — 1 en sıcak saatte (varsayılan 16:00),
	 * -1 en soğuk saatte (12 saat sonra, 04:00). Basit kosinüs modeli; çağıran taraf bunu
	 * `DayNightAmplitude` ile çarpıp hava durumu sıcaklığına ekler.
	 */
	inline float ComputeTimeOfDayModulation(float HourOfDay, float PeakHour = 16.0f)
	{
		const float Angle = ((HourOfDay - PeakHour) / 24.0f) * 2.0f * PI;
		return FMath::Cos(Angle);
	}

	/**
	 * Bir bölgenin iç sıcaklığı: dış ortam ile bölgenin kendi temel sıcaklığı arasında
	 * `InsulationFactor`'a göre karışım + ek ısı kaynağı katkısı. InsulationFactor=0 -> tamamen
	 * ortam; InsulationFactor=1 -> tamamen BaseTemperature (+ HeatSourcePower).
	 */
	inline float ComputeInteriorTemperature(float AmbientTemperature, float ZoneBaseTemperature, float HeatSourcePower, float InsulationFactor)
	{
		const float Insulation = FMath::Clamp(InsulationFactor, 0.0f, 1.0f);
		const float BlendedBase = FMath::Lerp(AmbientTemperature, ZoneBaseTemperature, Insulation);
		return BlendedBase + HeatSourcePower;
	}

	/**
	 * Vücut sıcaklığının çevre sıcaklığına doğru bu DeltaTime'da sürükleneceği miktar
	 * (Newton soğuma yasası — TAM üstel yaklaşım, 1-exp(-rate*dt)). İnceleme bulgusu: önceki
	 * doğrusal (Euler) yaklaşım (rate*dt kelepçelenmiş) büyük DeltaTime'larda kare hızından
	 * BAĞIMSIZ değildi (aynı gerçek süreyi az sayıda büyük adıma mı yoksa çok sayıda küçük
	 * adıma mı böldüğüne göre farklı sonuç verirdi) — 1-exp(-rate*dt) kompozisyon altında
	 * TAM değişmezdir ve zaten [0,1) aralığında olduğundan ayrı bir Clamp gerekmez. Çağıran
	 * taraf sonucu CurrentBodyTemp'e EKLEMELİDİR.
	 */
	inline float ComputeBodyTemperatureDelta(float CurrentBodyTemp, float EnvironmentTemp, float AdaptationRate, float DeltaTime)
	{
		const float BlendFactor = 1.0f - FMath::Exp(-AdaptationRate * FMath::Max(0.0f, DeltaTime));
		return (EnvironmentTemp - CurrentBodyTemp) * BlendFactor;
	}

	/**
	 * Ortam sıcaklığından, vücudun bu tick'te SÜRÜKLENMEYE ÇALIŞACAĞI hedef vücut sıcaklığını
	 * türetir. İNCELEME BULGUSU (PIE): ComputeBodyTemperatureDelta'ya doğrudan ham ortam
	 * sıcaklığı hedef olarak verilirse, insan vücudu fiziksel olarak dış hava sıcaklığına
	 * eşitlenmiş gibi modellenir — ki bu yanlıştır (metabolizma/giysi normal havada çekirdek
	 * sıcaklığı ~37°C'de tutar). Gerçek veriyle doğrulandı: DT_WeatherProfiles'daki TÜM
	 * profiller (Heatwave hariç) 35°C'nin (SafeMinBodyTemp) altında, yani ham modelle oyuncu
	 * HER normal havada (en yaygın "Clear" dahil) hipotermiden ölüyordu — PIE'de gözlemlendi
	 * (CurrentHealth PIE başladıktan ~1 dakika içinde 0'a düştü, hiçbir aşırı hava olmadan).
	 *
	 * Düzeltme: [ComfortMinTemp, ComfortMaxTemp] aralığındaki ortam sıcaklığı hedefi
	 * DEĞİŞTİRMEZ (NormalBodyTemp'te sabit kalır — giysi/barınak bu aralığı zaten telafi
	 * ediyor varsayımı); yalnızca aralığın DIŞINDAKİ fazlalık/eksiklik hedefe yansır. Böylece
	 * sıradan hava (Clear/Rainy/Overcast vb.) güvenlidir, yalnızca gerçekten aşırı hava
	 * (Kar/Tipi/Sıcak Dalgası) vücut sıcaklığını tehlikeli aralığa iter.
	 */
	inline float ComputeTargetBodyTemperature(float EnvironmentTemp, float ComfortMinTemp, float ComfortMaxTemp, float NormalBodyTemp = 37.0f)
	{
		const float HeatExcess = FMath::Max(0.0f, EnvironmentTemp - ComfortMaxTemp);
		const float ColdDeficit = FMath::Max(0.0f, ComfortMinTemp - EnvironmentTemp);
		return NormalBodyTemp + HeatExcess - ColdDeficit;
	}

	/**
	 * Vücut sıcaklığı güvenli aralığın (SafeMinTemp, SafeMaxTemp) dışındaysa, bu DeltaTime'da
	 * uygulanacak hasar miktarı — aralık dışına her derece için DamagePerSecondPerDegree.
	 * Aralık içindeyse 0 döner.
	 */
	inline float ComputeThermalDamage(float BodyTemperature, float SafeMinTemp, float SafeMaxTemp, float DamagePerSecondPerDegree, float DeltaTime)
	{
		float DegreesOutOfRange = 0.0f;
		if (BodyTemperature < SafeMinTemp)
		{
			DegreesOutOfRange = SafeMinTemp - BodyTemperature; // hipotermi
		}
		else if (BodyTemperature > SafeMaxTemp)
		{
			DegreesOutOfRange = BodyTemperature - SafeMaxTemp; // sicak carpmasi
		}
		return FMath::Max(0.0f, DegreesOutOfRange) * FMath::Max(0.0f, DamagePerSecondPerDegree) * FMath::Max(0.0f, DeltaTime);
	}
}
