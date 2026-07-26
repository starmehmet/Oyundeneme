// Sistem #1 birim testleri — saf zaman matematiği (TimeMath.h).
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Time"
// veya: UnrealEditor-Cmd.exe <proje> -ExecCmds="Automation RunTests SurvivalGame.Time; Quit" -nullrhi

#include "Misc/AutomationTest.h"
#include "Time/TimeMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimeMathBasicsTest,
	"SurvivalGame.Time.TimeMath.Temeller",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTimeMathBasicsTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTime;

	// Dakika / saat / gün dönüşümleri
	TestEqual(TEXT("t=0 → dakika 0"), MinuteOfDay(0.0), 0);
	TestEqual(TEXT("08:00 → dakika 480"), MinuteOfDay(8.0 * SecondsPerHour), 480);
	TestEqual(TEXT("23:59 → dakika 1439"), MinuteOfDay(23.0 * SecondsPerHour + 59.0 * SecondsPerMinute), 1439);
	TestEqual(TEXT("tam gün sonrası → dakika 0'a sarar"), MinuteOfDay(SecondsPerDay), 0);
	TestEqual(TEXT("2. gün 00:30 → dakika 30"), MinuteOfDay(SecondsPerDay + 1800.0), 30);
	TestEqual(TEXT("saat: 23:59 → 23"), HourOfDay(23.0 * SecondsPerHour + 59.0 * SecondsPerMinute), 23);
	TestEqual(TEXT("gün 0"), DayNumber(SecondsPerDay - 1.0), 0);
	TestEqual(TEXT("gün 3"), DayNumber(3.0 * SecondsPerDay + 100.0), 3);

	// Kesirli dakika yarım dakikayı korur
	TestEqual(TEXT("30 sn → 0.5 dakika"), MinuteOfDayFloat(30.0), 0.5, 1e-9);

	// IEEE sınır: sıfıra çok yakın negatif giriş [0, 1440) sözleşmesini bozmamalı
	// (yuvarlama toplamı tam 1440.0'a yapıştırabilir — inceleme bulgusu)
	TestTrue(TEXT("negatif epsilon girişte dakika < 1440"), MinuteOfDayFloat(-1e-12) < 1440.0);
	TestTrue(TEXT("negatif epsilon girişte dakika >= 0"), MinuteOfDayFloat(-1e-12) >= 0.0);
	TestTrue(TEXT("negatif epsilon girişte tam dakika <= 1439"), MinuteOfDay(-1e-12) <= 1439);
	TestTrue(TEXT("negatif girişte saat <= 23"), HourOfDay(-1e-12) <= 23);
	TestEqual(TEXT("negatif yarım gün sarar: -12 saat → dakika 720"), MinuteOfDay(-12.0 * SecondsPerHour), 720);

	// Gündüz kontrolü (sunrise=360, sunset=1080)
	TestFalse(TEXT("05:59 gece"), IsDaytime(359.0, 360.0, 1080.0));
	TestTrue(TEXT("06:00 gündüz başlar"), IsDaytime(360.0, 360.0, 1080.0));
	TestTrue(TEXT("öğlen gündüz"), IsDaytime(720.0, 360.0, 1080.0));
	TestFalse(TEXT("18:00 gece başlar"), IsDaytime(1080.0, 360.0, 1080.0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimeMathSunPitchTest,
	"SurvivalGame.Time.TimeMath.GunesPitch",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTimeMathSunPitchTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTime;
	constexpr double Sunrise = 360.0;
	constexpr double Sunset = 1080.0;

	TestEqual(TEXT("gündoğumu → 0°"), ComputeSunPitchDegrees(Sunrise, Sunrise, Sunset), 0.0, 1e-9);
	TestEqual(TEXT("öğlen → -90°"), ComputeSunPitchDegrees(720.0, Sunrise, Sunset), -90.0, 1e-9);
	TestEqual(TEXT("günbatımı → -180°"), ComputeSunPitchDegrees(Sunset, Sunrise, Sunset), -180.0, 1e-9);
	TestEqual(TEXT("gece yarısı (dakika 0) → -270°"), ComputeSunPitchDegrees(0.0, Sunrise, Sunset), -270.0, 1e-9);

	// Süreklilik: günbatımından hemen önce ve sonra sıçrama yok
	const double JustBefore = ComputeSunPitchDegrees(Sunset - 0.001, Sunrise, Sunset);
	const double JustAfter = ComputeSunPitchDegrees(Sunset + 0.001, Sunrise, Sunset);
	TestTrue(TEXT("günbatımı geçişi sürekli (<0.01° fark)"), FMath::Abs(JustBefore - JustAfter) < 0.01);

	// Gece sonu gündoğumuna yaklaşırken -360'a yaklaşır (0 mod 360 = gündoğumu ile hizalı)
	const double NearSunriseNight = ComputeSunPitchDegrees(Sunrise - 0.001, Sunrise, Sunset);
	TestTrue(TEXT("gece sonu ~-360°"), NearSunriseNight < -359.9);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimeMathDaylightFactorTest,
	"SurvivalGame.Time.TimeMath.IsikFaktoru",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTimeMathDaylightFactorTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTime;
	constexpr double Sunrise = 360.0;
	constexpr double Sunset = 1080.0;
	constexpr double Transition = 60.0;

	TestEqual(TEXT("gece derin → 0"), ComputeDaylightFactor(120.0, Sunrise, Sunset, Transition), 0.0, 1e-9);
	TestEqual(TEXT("pencere öncesi (05:29) → 0"), ComputeDaylightFactor(329.0, Sunrise, Sunset, Transition), 0.0, 1e-9);
	TestEqual(TEXT("tam gündoğumu → 0.5"), ComputeDaylightFactor(Sunrise, Sunrise, Sunset, Transition), 0.5, 1e-9);
	TestEqual(TEXT("pencere sonrası (06:31) → 1"), ComputeDaylightFactor(391.0, Sunrise, Sunset, Transition), 1.0, 1e-9);
	TestEqual(TEXT("öğlen → 1"), ComputeDaylightFactor(720.0, Sunrise, Sunset, Transition), 1.0, 1e-9);
	TestEqual(TEXT("tam günbatımı → 0.5"), ComputeDaylightFactor(Sunset, Sunrise, Sunset, Transition), 0.5, 1e-9);
	TestEqual(TEXT("günbatımı penceresi sonrası → 0"), ComputeDaylightFactor(1111.0, Sunrise, Sunset, Transition), 0.0, 1e-9);

	// Geçiş 0 → sert adım
	TestEqual(TEXT("geçiş 0, gece → 0"), ComputeDaylightFactor(359.9, Sunrise, Sunset, 0.0), 0.0, 1e-9);
	TestEqual(TEXT("geçiş 0, gündüz → 1"), ComputeDaylightFactor(360.0, Sunrise, Sunset, 0.0), 1.0, 1e-9);

	// Monotonluk: gündoğumu penceresi boyunca azalmaz
	double Prev = -1.0;
	for (double M = Sunrise - 30.0; M <= Sunrise + 30.0; M += 1.0)
	{
		const double F = ComputeDaylightFactor(M, Sunrise, Sunset, Transition);
		TestTrue(TEXT("gündoğumu penceresinde monoton artan"), F >= Prev);
		Prev = F;
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimeMathFrameDeltaClampTest,
	"SurvivalGame.Time.TimeMath.DeltaTimeKelepcesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTimeMathFrameDeltaClampTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTime;

	// Inceleme bulgusu (Sistem #29 PIE testi): motor thread'i bloke olup bir sonraki Tick'e
	// dev bir DeltaTime gelirse (donma/hitch), saat tek karede saatlerce ileri sicramamali.
	TestEqual(TEXT("normal kare (0.016sn) degismez"), ClampFrameDeltaTime(0.016f), 0.016f, 1e-6f);
	TestEqual(TEXT("sinirda (1.0sn) degismez"), ClampFrameDeltaTime(1.0f), 1.0f, 1e-6f);
	TestEqual(TEXT("dev sicrama (5832sn ~= 97dk donma) kelepcelenir"), ClampFrameDeltaTime(5832.0f), MaxBelievableFrameDeltaTime, 1e-6f);
	TestEqual(TEXT("negatif DeltaTime 0'a kelepcelenir"), ClampFrameDeltaTime(-1.0f), 0.0f, 1e-6f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
