// Sistem #13 birim testleri — saf gust/ruzgar-yuku/turbin matematigi (WindMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Weather"

#include "Misc/AutomationTest.h"
#include "Weather/WindMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWindMathGustTest,
	"SurvivalGame.Weather.WindMath.GustFazVeHiz",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWindMathGustTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWind;

	// Ayni hucre HER ZAMAN ayni fazi uretir (deterministik, RNG yok)
	const float PhaseA1 = ComputeGustPhaseOffset(5, -3);
	const float PhaseA2 = ComputeGustPhaseOffset(5, -3);
	TestEqual(TEXT("ayni hucre -> ayni faz"), PhaseA1, PhaseA2);

	// Farkli hucreler (genellikle) farkli faz uretir
	const float PhaseB = ComputeGustPhaseOffset(7, 11);
	TestNotEqual(TEXT("farkli hucre -> farkli faz (genel durum)"), PhaseA1, PhaseB);

	// Faz [0, 2*PI) araliginda
	TestTrue(TEXT("faz >= 0"), PhaseA1 >= 0.0f);
	TestTrue(TEXT("faz < 2*PI"), PhaseA1 < 2.0f * PI);

	// Grid hucresi: pozitif ve negatif konumlar dogru bolunuyor (taban bolme)
	TestTrue(TEXT("pozitif konum dogru hucre"), ComputeGustGridCell(FVector(2500.0, 100.0, 0.0), 1000.0f) == FIntPoint(2, 0));
	TestTrue(TEXT("negatif konum dogru hucre (taban bolme)"), ComputeGustGridCell(FVector(-100.0, -1500.0, 0.0), 1000.0f) == FIntPoint(-1, -2));

	// Gust faktoru [-1,1] araliginda ve hiz katkisi dogru isaretli
	const float GustFactor = ComputeGustFactor(0.0f, 0.0f, 1.0f); // sin(0)=0
	TestEqual(TEXT("t=0, faz=0 -> gust faktoru 0"), GustFactor, 0.0f);
	TestEqual(TEXT("gust faktoru 0 -> hiz degismez"), ComputeWindSpeedAt(10.0f, 0.0f, 5.0f), 10.0f);
	TestEqual(TEXT("pozitif gust -> hiz artar"), ComputeWindSpeedAt(10.0f, 1.0f, 5.0f), 15.0f);
	TestEqual(TEXT("negatif gust -> hiz azalir"), ComputeWindSpeedAt(10.0f, -1.0f, 5.0f), 5.0f);
	TestEqual(TEXT("buyuk negatif gust -> hiz negatife dusmez, 0'da kelepcelenir"), ComputeWindSpeedAt(2.0f, -1.0f, 100.0f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWindMathLoadTest,
	"SurvivalGame.Weather.WindMath.RuzgarYuku",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWindMathLoadTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWind;

	// 0.5 * Cd * v^2 * Alan
	TestEqual(TEXT("temel yuk hesabi"), ComputeWindLoad(10.0f, 20.0f, 1.2f), 0.5f * 1.2f * 100.0f * 20.0f);
	TestEqual(TEXT("ruzgar yok -> yuk yok"), ComputeWindLoad(0.0f, 20.0f, 1.2f), 0.0f);
	TestEqual(TEXT("negatif alan kelepcelenir -> yuk 0"), ComputeWindLoad(10.0f, -5.0f, 1.2f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWindMathTurbineTest,
	"SurvivalGame.Weather.WindMath.TurbinCiktisi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWindMathTurbineTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWind;

	// CutInSpeed altinda -> 0 cikti
	TestEqual(TEXT("cut-in altinda -> 0"), ComputeTurbineOutput(2.0f, 3.0f, 12.0f, 50.0f), 0.0f);

	// RatedSpeed'de/ustunde -> tam RatedOutput
	TestEqual(TEXT("rated hizda -> tam cikti"), ComputeTurbineOutput(12.0f, 3.0f, 12.0f, 50.0f), 50.0f);
	TestEqual(TEXT("rated ustunde -> tam cikti (sabit)"), ComputeTurbineOutput(20.0f, 3.0f, 12.0f, 50.0f), 50.0f);

	// Arada -> kubik ramp: yari yolda (orani 0.5) -> 0.5^3 = 0.125 katsayi
	const float Midpoint = 3.0f + (12.0f - 3.0f) * 0.5f; // 7.5
	TestEqual(TEXT("yari yolda kubik oran"), ComputeTurbineOutput(Midpoint, 3.0f, 12.0f, 50.0f), 50.0f * 0.125f, 0.01f);

	// Monotonluk: daha yuksek hiz (ayni aralikta) hep >= cikti verir
	const float LowOutput = ComputeTurbineOutput(5.0f, 3.0f, 12.0f, 50.0f);
	const float HighOutput = ComputeTurbineOutput(9.0f, 3.0f, 12.0f, 50.0f);
	TestTrue(TEXT("daha yuksek hiz -> daha yuksek (veya esit) cikti"), HighOutput >= LowOutput);

	// Hatali veri (RatedSpeed<=CutInSpeed) bolme hatasi vermez, cokme yok
	const float DegenerateOutput = ComputeTurbineOutput(5.0f, 10.0f, 10.0f, 50.0f);
	TestTrue(TEXT("RatedSpeed<=CutInSpeed -> cokme yok, sonlu deger"), FMath::IsFinite(DegenerateOutput));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
