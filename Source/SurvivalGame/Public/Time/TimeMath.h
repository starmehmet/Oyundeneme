#pragma once

#include "CoreMinimal.h"

/**
 * Sistem #1 — Zaman: saf, durum tutmayan zaman matematiği.
 * Tüm fonksiyonlar yan etkisiz; birim testleri Private/Tests/TimeMathTests.cpp.
 * Zaman temsili: TotalGameSeconds (double) — ölçek UYGULANMIŞ oyun-içi saniye.
 */
namespace SurvivalTime
{
	inline constexpr int32 MinutesPerDay = 1440;
	inline constexpr double SecondsPerMinute = 60.0;
	inline constexpr double SecondsPerHour = 3600.0;
	inline constexpr double SecondsPerDay = 86400.0;

	/**
	 * Bir kare icin "gercekci" sayilan azami gercek-zamanli DeltaTime (saniye). Sistem #29
	 * PIE testi sirasinda bulunan gercek hata: editor MCP cagrilarina yanit verirken oyun
	 * thread'i bloke olabiliyor, bir sonraki Tick'e DEV bir DeltaTime (dakikalarca) geliyor —
	 * TimeKeeper bunu dogrudan TotalGameSeconds'a eklerse (x10 olcekte) oyun-ici saat tek
	 * karede saatlerce ileri sicriyor. FMath::Min ile ClampMin/Max degil DOGRUDAN bir ust
	 * sinir — bu deger "gercek oynanista asla olmaz, yalnizca donma/hitch belirtisi" varsayimi.
	 */
	inline constexpr float MaxBelievableFrameDeltaTime = 1.0f;

	/** DeltaTime'i MaxBelievableFrameDeltaTime'a kelepceler (donma/hitch sonrasi sicrama korumasi). */
	inline float ClampFrameDeltaTime(float DeltaTime)
	{
		return FMath::Clamp(DeltaTime, 0.0f, MaxBelievableFrameDeltaTime);
	}

	/** Günün dakikası, kesirli [0, 1440). Negatif girişte de geçerli aralığa sarar. */
	inline double MinuteOfDayFloat(double TotalGameSeconds)
	{
		double Minute = FMath::Fmod(TotalGameSeconds / SecondsPerMinute, static_cast<double>(MinutesPerDay));
		if (Minute < 0.0)
		{
			Minute += MinutesPerDay;
		}
		// IEEE yuvarlama: sıfıra çok yakın negatif girişte toplam tam 1440.0'a
		// yapışabilir — [0, 1440) sözleşmesini koru (inceleme bulgusu)
		return Minute >= MinutesPerDay ? 0.0 : Minute;
	}

	/** Günün dakikası, tam sayı [0, 1439]. */
	inline int32 MinuteOfDay(double TotalGameSeconds)
	{
		return static_cast<int32>(MinuteOfDayFloat(TotalGameSeconds));
	}

	/** Günün saati [0, 23]. */
	inline int32 HourOfDay(double TotalGameSeconds)
	{
		return MinuteOfDay(TotalGameSeconds) / 60;
	}

	/** Gün numarası, 0'dan başlar. */
	inline int32 DayNumber(double TotalGameSeconds)
	{
		return static_cast<int32>(TotalGameSeconds / SecondsPerDay);
	}

	/** Gündüz mü? [SunriseMinute, SunsetMinute) aralığı gündüz sayılır. */
	inline bool IsDaytime(double MinuteOfDayF, double SunriseMinute, double SunsetMinute)
	{
		return MinuteOfDayF >= SunriseMinute && MinuteOfDayF < SunsetMinute;
	}

	/**
	 * Güneş pitch açısı (derece): gündoğumunda 0, öğlen -90, günbatımında -180,
	 * gece -180'den -360'a devam eder — güneş kesintisiz tam tur döner.
	 * Ön koşul: 0 < SunriseMinute < SunsetMinute < 1440.
	 */
	inline double ComputeSunPitchDegrees(double MinuteOfDayF, double SunriseMinute, double SunsetMinute)
	{
		const double DayLength = SunsetMinute - SunriseMinute;
		const double NightLength = MinutesPerDay - DayLength;

		if (IsDaytime(MinuteOfDayF, SunriseMinute, SunsetMinute))
		{
			const double Progress = (MinuteOfDayF - SunriseMinute) / DayLength;
			return -Progress * 180.0;
		}

		const double SinceSunset = MinuteOfDayF >= SunsetMinute
			? MinuteOfDayF - SunsetMinute
			: MinuteOfDayF + MinutesPerDay - SunsetMinute;
		const double Progress = SinceSunset / NightLength;
		return -180.0 - Progress * 180.0;
	}

	/**
	 * Gün ışığı faktörü [0, 1]: gece 0, tam gündüz 1; gündoğumu/batımı çevresinde
	 * TransitionMinutes genişliğinde smoothstep geçiş (merkez tam sunrise/sunset anı).
	 * Geçiş penceresi gün sınırlarına taşmayacak şekilde içeride kırpılır.
	 */
	inline double ComputeDaylightFactor(double MinuteOfDayF, double SunriseMinute, double SunsetMinute, double TransitionMinutes)
	{
		const double DayLength = SunsetMinute - SunriseMinute;
		// Pencere yarısı; gece/gündüz uçlarının 0'ın altına veya 1440 üstüne taşmasını engelle
		const double Half = FMath::Clamp(TransitionMinutes * 0.5,
			0.0,
			FMath::Min3(SunriseMinute, MinutesPerDay - SunsetMinute, DayLength * 0.5));

		auto SmoothStep01 = [](double T)
		{
			T = FMath::Clamp(T, 0.0, 1.0);
			return T * T * (3.0 - 2.0 * T);
		};

		if (Half <= 0.0)
		{
			return IsDaytime(MinuteOfDayF, SunriseMinute, SunsetMinute) ? 1.0 : 0.0;
		}

		if (MinuteOfDayF < SunriseMinute - Half || MinuteOfDayF >= SunsetMinute + Half)
		{
			return 0.0;
		}
		if (MinuteOfDayF < SunriseMinute + Half)
		{
			return SmoothStep01((MinuteOfDayF - (SunriseMinute - Half)) / (2.0 * Half));
		}
		if (MinuteOfDayF < SunsetMinute - Half)
		{
			return 1.0;
		}
		return SmoothStep01(((SunsetMinute + Half) - MinuteOfDayF) / (2.0 * Half));
	}
}
