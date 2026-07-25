// Sistem #14 birim testleri — saf kar birikimi/erime/hareket/cig matematigi (SnowMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Weather"

#include "Misc/AutomationTest.h"
#include "Weather/SnowMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSnowMathAccumulationTest,
	"SurvivalGame.Weather.SnowMath.BirikimVeErime",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSnowMathAccumulationTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalSnow;

	// Kar yagisli degilse -> birikim yok (yagis olsa bile)
	TestEqual(TEXT("kar yagisli degil -> birikim 0"), ComputeSnowAccumulationRate(false, 0.8f, 0.1f), 0.0f);

	// Kar yagisli -> yagis*katsayi
	TestEqual(TEXT("kar yagisli -> yagis*katsayi"), ComputeSnowAccumulationRate(true, 0.8f, 0.1f), 0.08f, 0.001f);

	// Donma noktasinda/altinda -> erime yok
	TestEqual(TEXT("donma noktasinda -> erime 0"), ComputeSnowMeltRate(0.0f, 0.0f, 0.1f), 0.0f);
	TestEqual(TEXT("donma noktasi altinda -> erime 0"), ComputeSnowMeltRate(-5.0f, 0.0f, 0.1f), 0.0f);

	// Donma noktasi ustunde -> derece basina erime
	TestEqual(TEXT("5 derece ustunde -> 5*katsayi erime"), ComputeSnowMeltRate(5.0f, 0.0f, 0.1f), 0.5f, 0.001f);

	// Net delta: birikim > erime -> pozitif; erime > birikim -> negatif
	TestTrue(TEXT("birikim>erime -> pozitif delta"), ComputeSnowDepthDelta(0.1f, 0.02f, 1.0f) > 0.0f);
	TestTrue(TEXT("erime>birikim -> negatif delta"), ComputeSnowDepthDelta(0.02f, 0.1f, 1.0f) < 0.0f);
	TestEqual(TEXT("deltaTime=0 -> delta=0"), ComputeSnowDepthDelta(0.1f, 0.02f, 0.0f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSnowMathMovementTest,
	"SurvivalGame.Weather.SnowMath.HareketVeInsaat",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSnowMathMovementTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalSnow;

	// Kar yok -> tam hiz
	TestEqual(TEXT("kar yok -> carpan 1.0"), ComputeMovementSpeedMultiplier(0.0f, 50.0f, 0.4f), 1.0f);

	// Tavan derinlikte/ustunde -> minimum carpan
	TestEqual(TEXT("tavan derinlikte -> minimum carpan"), ComputeMovementSpeedMultiplier(50.0f, 50.0f, 0.4f), 0.4f, 0.001f);
	TestEqual(TEXT("tavan ustunde -> minimum carpanda kelepcelenir"), ComputeMovementSpeedMultiplier(200.0f, 50.0f, 0.4f), 0.4f, 0.001f);

	// Yari derinlik -> yari yolda dogrusal
	TestEqual(TEXT("yari derinlik -> dogrusal ortalama"), ComputeMovementSpeedMultiplier(25.0f, 50.0f, 0.4f), 0.7f, 0.001f);

	// Insaat engelleme
	TestFalse(TEXT("esik altinda -> engellenmez"), IsConstructionBlocked(20.0f, 30.0f));
	TestFalse(TEXT("esikte tam -> engellenmez (> kullanilir, >= degil)"), IsConstructionBlocked(30.0f, 30.0f));
	TestTrue(TEXT("esik ustunde -> engellenir"), IsConstructionBlocked(31.0f, 30.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSnowMathAvalancheTest,
	"SurvivalGame.Weather.SnowMath.CigRiski",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSnowMathAvalancheTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalSnow;

	// Duz zemin (yukari bakan normal) -> egim 0
	TestEqual(TEXT("duz zemin -> egim 0"), ComputeSlopeAngleDegrees(FVector::UpVector), 0.0f, 0.01f);

	// Dikey duvar (yatay normal) -> egim 90
	TestEqual(TEXT("dikey duvar -> egim 90"), ComputeSlopeAngleDegrees(FVector::ForwardVector), 90.0f, 0.01f);

	// 45 derecelik yamac
	const FVector DiagonalNormal = FVector(1.0f, 0.0f, 1.0f).GetSafeNormal();
	TestEqual(TEXT("45 derecelik yamac"), ComputeSlopeAngleDegrees(DiagonalNormal), 45.0f, 0.1f);

	// Cig riski: derinlik yetersiz -> risk yok (uygun egimde bile)
	TestFalse(TEXT("derinlik yetersiz -> risk yok"), IsAvalancheRisk(10.0f, 40.0f, 40.0f, 30.0f, 55.0f));

	// Cig riski: derinlik yeterli ama duz zemin -> risk yok
	TestFalse(TEXT("duz zemin -> risk yok"), IsAvalancheRisk(50.0f, 5.0f, 40.0f, 30.0f, 55.0f));

	// Cig riski: derinlik yeterli ama asiri dik -> risk yok (kar tutunamaz varsayimi)
	TestFalse(TEXT("asiri dik -> risk yok"), IsAvalancheRisk(50.0f, 80.0f, 40.0f, 30.0f, 55.0f));

	// Cig riski: derinlik yeterli VE uygun orta-dik egim -> risk VAR
	TestTrue(TEXT("yeterli derinlik + uygun egim -> risk var"), IsAvalancheRisk(50.0f, 40.0f, 40.0f, 30.0f, 55.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
