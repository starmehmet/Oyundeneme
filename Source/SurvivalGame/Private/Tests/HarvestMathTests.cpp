// Sistem #29 birim testleri — saf verim-araligi/yeniden-dogma mantigi (HarvestMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Harvesting"

#include "Misc/AutomationTest.h"
#include "Harvesting/HarvestMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHarvestMathYieldRangeTest,
	"SurvivalGame.Harvesting.HarvestMath.VerimAraligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHarvestMathYieldRangeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHarvest;

	int32 OutMin = 0;
	int32 OutMax = 0;

	NormalizeYieldRange(2, 5, OutMin, OutMax);
	TestEqual(TEXT("normal aralik: Min degismez"), OutMin, 2);
	TestEqual(TEXT("normal aralik: Max degismez"), OutMax, 5);

	NormalizeYieldRange(5, 2, OutMin, OutMax);
	TestEqual(TEXT("bozuk aralik (Min>Max): Min degismez"), OutMin, 5);
	TestEqual(TEXT("bozuk aralik (Min>Max): Max, Min'e esitlenir"), OutMax, 5);

	NormalizeYieldRange(3, 3, OutMin, OutMax);
	TestEqual(TEXT("esit aralik: Min degismez"), OutMin, 3);
	TestEqual(TEXT("esit aralik: Max degismez"), OutMax, 3);

	// Inceleme bulgusu (minör): Min<1 -> 1'e kelepcelenir, aksi halde RequestedYield=0 uretip
	// dugumun hicbir sey vermeden tukenmesine yol aciyordu.
	NormalizeYieldRange(0, 3, OutMin, OutMax);
	TestEqual(TEXT("Min=0 -> 1'e kelepcelenir"), OutMin, 1);
	TestEqual(TEXT("Min=0 durumunda Max degismez"), OutMax, 3);

	NormalizeYieldRange(0, 0, OutMin, OutMax);
	TestEqual(TEXT("Min=Max=0 -> Min 1'e kelepcelenir"), OutMin, 1);
	TestEqual(TEXT("Min=Max=0 -> Max, kelepcelenmis Min'e esitlenir"), OutMax, 1);

	NormalizeYieldRange(-5, -2, OutMin, OutMax);
	TestEqual(TEXT("negatif Min -> 1'e kelepcelenir"), OutMin, 1);
	TestEqual(TEXT("negatif Max, kelepcelenmis Min'e esitlenir"), OutMax, 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHarvestMathRespawnReadyTest,
	"SurvivalGame.Harvesting.HarvestMath.YenidenDogma",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHarvestMathRespawnReadyTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHarvest;

	TestFalse(TEXT("henuz suresi dolmadi"), IsRespawnReady(0.0, 30.0, 60.0f));
	TestTrue(TEXT("tam sinirda hazir"), IsRespawnReady(0.0, 60.0, 60.0f));
	TestTrue(TEXT("suresi gectikten sonra hazir"), IsRespawnReady(0.0, 90.0, 60.0f));

	// RespawnSeconds<=0 -> her zaman hazir (anlik yeniden-dogma)
	TestTrue(TEXT("RespawnSeconds=0 -> her zaman hazir"), IsRespawnReady(0.0, 0.0, 0.0f));
	TestTrue(TEXT("RespawnSeconds negatif -> her zaman hazir"), IsRespawnReady(100.0, 100.0, -5.0f));

	// Mutlak zaman damgasi: hucre boslatilip gec kontrol edilse bile (buyuk CurrentGameTime farki)
	// kayip ilerleme olmadan hazir sayilir, negatife dusmez.
	TestTrue(TEXT("uzun bosaltma sonrasi tek seferde telafi"), IsRespawnReady(10.0, 10000.0, 60.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
