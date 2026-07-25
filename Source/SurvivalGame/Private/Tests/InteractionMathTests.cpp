// Sistem #3 birim testleri — saf etkileşim mantığı (InteractionMath.h).
// Çalıştırma: Automation RunTests SurvivalGame.Interaction

#include "Misc/AutomationTest.h"
#include "Interaction/InteractionMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionTraceThrottleTest,
	"SurvivalGame.Interaction.InteractionMath.TraceKisma",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInteractionTraceThrottleTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalInteraction;

	// Interval 4: yalnizca 0, 4, 8... karelerinde trace
	TestTrue(TEXT("kare 0 -> trace"), ShouldTraceThisFrame(0, 4));
	TestFalse(TEXT("kare 1 -> yok"), ShouldTraceThisFrame(1, 4));
	TestFalse(TEXT("kare 3 -> yok"), ShouldTraceThisFrame(3, 4));
	TestTrue(TEXT("kare 4 -> trace"), ShouldTraceThisFrame(4, 4));
	TestTrue(TEXT("kare 800 -> trace"), ShouldTraceThisFrame(800, 4));

	// Interval 1 ve altı: her kare
	TestTrue(TEXT("interval 1 her kare"), ShouldTraceThisFrame(7, 1));
	TestTrue(TEXT("interval 0 guvenli (her kare)"), ShouldTraceThisFrame(7, 0));
	TestTrue(TEXT("negatif interval guvenli"), ShouldTraceThisFrame(7, -3));

	// 10 karelik pencerede interval 4 tam 3 kez trace etmeli (0,4,8)
	int32 Count = 0;
	for (uint64 F = 0; F < 10; ++F) { if (ShouldTraceThisFrame(F, 4)) { ++Count; } }
	TestEqual(TEXT("10 karede 3 trace"), Count, 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionPromptDebounceTest,
	"SurvivalGame.Interaction.InteractionMath.PromptDebounce",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInteractionPromptDebounceTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalInteraction;

	// Odak degismediyse asla guncelleme
	TestFalse(TEXT("degisim yoksa guncelleme yok"), ShouldUpdatePrompt(10.0, 5.0, 0.2, false));

	// Ilk guncelleme (sentinel -1) her zaman gecer
	TestTrue(TEXT("ilk guncelleme gecer"), ShouldUpdatePrompt(0.0, -1.0, 0.2, true));

	// Aralik dolmadan tekrar degisim -> bastirilir
	TestFalse(TEXT("0.1 sn sonra bastirilir"), ShouldUpdatePrompt(5.1, 5.0, 0.2, true));

	// Aralik dolunca gecer (tam sinir dahil)
	TestTrue(TEXT("tam 0.2 sn sinirinda gecer"), ShouldUpdatePrompt(5.2, 5.0, 0.2, true));
	TestTrue(TEXT("0.5 sn sonra gecer"), ShouldUpdatePrompt(5.5, 5.0, 0.2, true));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionRangeTest,
	"SurvivalGame.Interaction.InteractionMath.Menzil",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInteractionRangeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalInteraction;

	const FVector Pawn(0, 0, 90);

	TestTrue(TEXT("100 UU hedef 250 menzilde"), IsWithinRange(Pawn, FVector(100, 0, 90), 250.0f));
	TestTrue(TEXT("tam sinirda dahil"), IsWithinRange(Pawn, FVector(250, 0, 90), 250.0f));
	TestFalse(TEXT("251 UU disari"), IsWithinRange(Pawn, FVector(251, 0, 90), 250.0f));

	// 3B mesafe (yukseklik farki dahil): 3-4-5 ucgeni
	TestTrue(TEXT("3B mesafe 500 = 500 menzil"), IsWithinRange(Pawn, Pawn + FVector(300, 400, 0), 500.0f));
	TestFalse(TEXT("3B mesafe 500 > 499 menzil"), IsWithinRange(Pawn, Pawn + FVector(300, 400, 0), 499.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
