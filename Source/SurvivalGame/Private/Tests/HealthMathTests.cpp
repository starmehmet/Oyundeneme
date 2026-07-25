// Sistem #12 birim testleri — saf hasar/iyilesme kelepceleme mantigi (HealthMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Health"

#include "Misc/AutomationTest.h"
#include "Player/HealthMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthMathTest,
	"SurvivalGame.Health.HealthMath.HasarVeIyilesme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHealthMathTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHealth;

	// Hasar: normal durum, tam uygulanir
	TestEqual(TEXT("100 candan 30 hasar -> 30 uygulanir"), ComputeAppliedDamage(100.0f, 30.0f), 30.0f);

	// Hasar: kalan candan fazla istenirse kalanla sinirlanir (can negatife dusmez)
	TestEqual(TEXT("20 candan 50 hasar istenirse -> 20 uygulanir"), ComputeAppliedDamage(20.0f, 50.0f), 20.0f);

	// Hasar: negatif/sifir miktar -> hicbir sey uygulanmaz
	TestEqual(TEXT("negatif hasar -> 0"), ComputeAppliedDamage(50.0f, -10.0f), 0.0f);

	// Iyilesme: normal durum, tam uygulanir
	TestEqual(TEXT("50/100 candan 20 iyilesme -> 20 uygulanir"), ComputeAppliedHeal(50.0f, 100.0f, 20.0f), 20.0f);

	// Iyilesme: MaxHealth'i asacak miktar istenirse kalan bosluk kadar sinirlanir
	TestEqual(TEXT("90/100 candan 30 iyilesme istenirse -> yalniz 10 uygulanir"), ComputeAppliedHeal(90.0f, 100.0f, 30.0f), 10.0f);

	// Iyilesme: zaten tam doluysa -> 0
	TestEqual(TEXT("tam dolu candan iyilesme -> 0"), ComputeAppliedHeal(100.0f, 100.0f, 20.0f), 0.0f);

	TestFalse(TEXT("pozitif can -> olu degil"), IsDead(1.0f));
	TestTrue(TEXT("0 can -> olu"), IsDead(0.0f));
	TestTrue(TEXT("negatif can -> olu"), IsDead(-5.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
