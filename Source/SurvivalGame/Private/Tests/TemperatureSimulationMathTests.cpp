// Sistem #12 birim testleri — saf sicaklik/termal hasar mantigi (TemperatureMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Weather"

#include "Misc/AutomationTest.h"
#include "Weather/TemperatureMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemperatureMathTimeOfDayTest,
	"SurvivalGame.Weather.TemperatureMath.GunIciSalinim",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTemperatureMathTimeOfDayTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTemperature;

	// Zirve saatinde (varsayilan 16:00) salinim tam +1 olmali
	TestEqual(TEXT("zirve saatinde salinim = 1"), ComputeTimeOfDayModulation(16.0f), 1.0f, 0.001f);

	// Zirvenin 12 saat sonrasinda (04:00) salinim tam -1 olmali
	TestEqual(TEXT("zirvenin 12 saat sonrasinda salinim = -1"), ComputeTimeOfDayModulation(4.0f), -1.0f, 0.001f);

	// Ozel PeakHour parametresi de calismali
	TestEqual(TEXT("ozel zirve saatinde salinim = 1"), ComputeTimeOfDayModulation(8.0f, 8.0f), 1.0f, 0.001f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemperatureMathInteriorTest,
	"SurvivalGame.Weather.TemperatureMath.IcMekanKarisimi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTemperatureMathInteriorTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTemperature;

	// Yalitim=0 -> tamamen dis ortam sicakligi (+ isi kaynagi 0)
	TestEqual(TEXT("yalitim=0 -> tamamen ortam"), ComputeInteriorTemperature(-10.0f, 20.0f, 0.0f, 0.0f), -10.0f);

	// Yalitim=1 -> tamamen bolgenin kendi temel sicakligi (+ isi kaynagi)
	TestEqual(TEXT("yalitim=1 -> tamamen bolge temeli"), ComputeInteriorTemperature(-10.0f, 20.0f, 0.0f, 1.0f), 20.0f);

	// Yalitim=0.5 -> ortalama + isi kaynagi katkisi
	TestEqual(TEXT("yalitim=0.5 -> ortalama + isi kaynagi"), ComputeInteriorTemperature(0.0f, 20.0f, 5.0f, 0.5f), 15.0f);

	// Yalitim araligi disi degerler kelepcelenir
	TestEqual(TEXT("yalitim>1 kelepcelenir"), ComputeInteriorTemperature(-10.0f, 20.0f, 0.0f, 2.0f), 20.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemperatureMathTargetBodyTemperatureTest,
	"SurvivalGame.Weather.TemperatureMath.KonforBandiHedefSicaklik",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTemperatureMathTargetBodyTemperatureTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTemperature;

	// Konfor bandi icinde (ornek: Clear=22, Rainy=15, Stormy=13 -- DT_WeatherProfiles) -> hedef DEGISMEZ, tam normal vucut sicakligi
	TestEqual(TEXT("konfor bandi ortasinda -> hedef=normal"), ComputeTargetBodyTemperature(22.0f, 10.0f, 30.0f), 37.0f);
	TestEqual(TEXT("konfor bandi alt sinirinda -> hedef=normal"), ComputeTargetBodyTemperature(10.0f, 10.0f, 30.0f), 37.0f);
	TestEqual(TEXT("konfor bandi ust sinirinda -> hedef=normal"), ComputeTargetBodyTemperature(30.0f, 10.0f, 30.0f), 37.0f);

	// Bandin ALTINDA (ornek: Snowing=-2, Blizzard=-10) -> hedef, eksik kadar normalin ALTINA duser
	TestEqual(TEXT("bandin 12 altinda -> hedef 12 dusuk"), ComputeTargetBodyTemperature(-2.0f, 10.0f, 30.0f), 25.0f);
	TestEqual(TEXT("bandin 20 altinda -> hedef 20 dusuk"), ComputeTargetBodyTemperature(-10.0f, 10.0f, 30.0f), 17.0f);

	// Bandin USTUNDE (ornek: Heatwave=38) -> hedef, fazlalik kadar normalin USTUNE cikar
	TestEqual(TEXT("bandin 8 ustunde -> hedef 8 yuksek"), ComputeTargetBodyTemperature(38.0f, 10.0f, 30.0f), 45.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemperatureMathBodyDeltaTest,
	"SurvivalGame.Weather.TemperatureMath.VucutSicakligiSuruklenmesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTemperatureMathBodyDeltaTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTemperature;

	// Cevre daha soguksa delta negatif olmali (vucut sogumaya dogru surukleniyor)
	const float ColdDelta = ComputeBodyTemperatureDelta(37.0f, -10.0f, 0.05f, 1.0f);
	TestTrue(TEXT("soguk ortamda delta negatif"), ColdDelta < 0.0f);

	// Cevre daha sicaksa delta pozitif olmali
	const float HotDelta = ComputeBodyTemperatureDelta(37.0f, 45.0f, 0.05f, 1.0f);
	TestTrue(TEXT("sicak ortamda delta pozitif"), HotDelta > 0.0f);

	// Zaten cevreyle ayniysa delta sifir
	TestEqual(TEXT("cevreyle ayni sicaklikta delta=0"), ComputeBodyTemperatureDelta(20.0f, 20.0f, 0.05f, 1.0f), 0.0f);

	// DeltaTime=0 -> hicbir surukleme olmaz
	TestEqual(TEXT("deltaTime=0 -> delta=0"), ComputeBodyTemperatureDelta(37.0f, -10.0f, 0.05f, 0.0f), 0.0f);

	// Asiri buyuk DeltaTime bile vucudu hedefin OTESINE tasimamali (blend factor kelepcelenir)
	const float ExtremeDelta = ComputeBodyTemperatureDelta(37.0f, -10.0f, 0.05f, 100000.0f);
	TestEqual(TEXT("asiri deltaTime kelepcelenir -> tam hedefe gider, otesine gecmez"), 37.0f + ExtremeDelta, -10.0f, 0.001f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTemperatureMathThermalDamageTest,
	"SurvivalGame.Weather.TemperatureMath.TermalHasar",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTemperatureMathThermalDamageTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTemperature;

	// Guvenli aralik icinde -> hasar yok
	TestEqual(TEXT("guvenli aralikta hasar=0"), ComputeThermalDamage(37.0f, 35.0f, 39.0f, 2.0f, 1.0f), 0.0f);

	// Aralik sinirinda tam -> hasar yok
	TestEqual(TEXT("sinirda hasar=0"), ComputeThermalDamage(35.0f, 35.0f, 39.0f, 2.0f, 1.0f), 0.0f);

	// Alt sinirin altinda (hipotermi) -> derece basina hasar
	TestEqual(TEXT("hipotermi: 2 derece altinda, 2/derece/sn, 1sn -> 4 hasar"), ComputeThermalDamage(33.0f, 35.0f, 39.0f, 2.0f, 1.0f), 4.0f);

	// Ust sinirin ustunde (sicak carpmasi) -> derece basina hasar
	TestEqual(TEXT("sicak carpmasi: 3 derece ustunde, 2/derece/sn, 1sn -> 6 hasar"), ComputeThermalDamage(42.0f, 35.0f, 39.0f, 2.0f, 1.0f), 6.0f);

	// DeltaTime ile olceklenir
	TestEqual(TEXT("0.5sn'de yari hasar"), ComputeThermalDamage(42.0f, 35.0f, 39.0f, 2.0f, 0.5f), 3.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
