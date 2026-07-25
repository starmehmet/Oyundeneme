// Sistem #10 birim testleri — saf enerji-frekansi/termal/yakit-kitligi mantigi (ResourceSimulationMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Resource"

#include "Misc/AutomationTest.h"
#include "Production/ResourceSimulationMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResourceMathFrequencyTest,
	"SurvivalGame.Resource.ResourceSimulationMath.Frekans",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FResourceMathFrequencyTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalResource;

	// Tuketim yok -> nominal 50Hz (sebekede yuk yok)
	TestEqual(TEXT("tuketim 0 -> 50Hz"), ComputeFrequency(100.0f, 0.0f), 50.0f);

	// Uretim tuketimi tam karsiliyor -> 50Hz
	TestEqual(TEXT("uretim=tuketim -> 50Hz"), ComputeFrequency(100.0f, 100.0f), 50.0f);

	// Uretim fazlasi -> yine 50Hz'i ASMAZ (bkz. fonksiyon yorumu)
	TestEqual(TEXT("uretim > tuketim -> yine 50Hz (asilmaz)"), ComputeFrequency(200.0f, 100.0f), 50.0f);

	// Yarim uretim -> yarim frekans (basitlestirilmis dogrusal model)
	TestEqual(TEXT("uretim tuketimin yarisi -> 25Hz"), ComputeFrequency(50.0f, 100.0f), 25.0f);

	// Hic uretim yok, tuketim var -> 0Hz
	TestEqual(TEXT("uretim 0, tuketim var -> 0Hz"), ComputeFrequency(0.0f, 100.0f), 0.0f);

	TestTrue(TEXT("50Hz istikrarli"), IsFrequencyStable(50.0f));
	TestTrue(TEXT("49.5Hz sinirda istikrarli"), IsFrequencyStable(49.5f));
	TestFalse(TEXT("49.0Hz istikrarsiz"), IsFrequencyStable(49.0f));

	TestFalse(TEXT("48Hz tam sinirda brownout DEGIL"), IsBrownout(48.0f));
	TestTrue(TEXT("47.9Hz brownout"), IsBrownout(47.9f));
	TestFalse(TEXT("50Hz brownout degil"), IsBrownout(50.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResourceMathThermalTest,
	"SurvivalGame.Resource.ResourceSimulationMath.Termal",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FResourceMathThermalTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalResource;

	// Isi uretimi dagitimi asiyor -> pozitif delta (isinma)
	TestEqual(TEXT("10 uretim - 4 dagitim, 2sn -> +12"), ComputeTemperatureDelta(10.0f, 4.0f, 2.0f), 12.0f);

	// Dagitim uretimi asiyor -> negatif delta (sogumA)
	TestEqual(TEXT("4 uretim - 10 dagitim, 2sn -> -12"), ComputeTemperatureDelta(4.0f, 10.0f, 2.0f), -12.0f);

	// Denge -> degisim yok
	TestEqual(TEXT("uretim=dagitim -> 0"), ComputeTemperatureDelta(5.0f, 5.0f, 3.0f), 0.0f);

	// Negatif DeltaTime -> 0'a kelepcelenir (savunmaci)
	TestEqual(TEXT("negatif DeltaTime -> 0"), ComputeTemperatureDelta(10.0f, 4.0f, -1.0f), 0.0f);

	TestFalse(TEXT("sinirda asiri isinma degil"), IsOverheating(100.0f, 100.0f));
	TestTrue(TEXT("sinirin ustunde asiri isinma"), IsOverheating(100.1f, 100.0f));
	TestFalse(TEXT("sinirin altinda asiri isinma degil"), IsOverheating(50.0f, 100.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResourceMathFuelTest,
	"SurvivalGame.Resource.ResourceSimulationMath.YakitKitligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FResourceMathFuelTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalResource;

	// %10 esik: %5 kalan -> kritik
	TestTrue(TEXT("%5 kalan, %10 esik -> kritik"), IsFuelCritical(5.0f, 100.0f, 0.1f));

	// %50 kalan -> kritik degil
	TestFalse(TEXT("%50 kalan -> kritik degil"), IsFuelCritical(50.0f, 100.0f, 0.1f));

	// Tam esikte -> kritik DEGIL (sinir < ile kesin kucuklukten sonra basliyor)
	TestFalse(TEXT("tam esikte kritik degil"), IsFuelCritical(10.0f, 100.0f, 0.1f));

	// MaxAmount<=0 -> her zaman false (tanimsiz rezerv icin "kitlik" anlamsiz)
	TestFalse(TEXT("MaxAmount 0 -> her zaman false"), IsFuelCritical(0.0f, 0.0f, 0.1f));

	// Bos rezerv (0 kalan, gecerli MaxAmount) -> kesinlikle kritik
	TestTrue(TEXT("bos rezerv -> kritik"), IsFuelCritical(0.0f, 100.0f, 0.1f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
