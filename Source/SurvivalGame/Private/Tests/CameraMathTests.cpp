// Sistem #2 birim testleri — saf kamera matematiği (CameraMath.h).
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Player"

#include "Misc/AutomationTest.h"
#include "Player/CameraMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraMathZoomTest,
	"SurvivalGame.Player.CameraMath.ZoomAdimi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraMathZoomTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalCamera;

	constexpr float Min = 150.0f;
	constexpr float Max = 600.0f;
	constexpr float Step = 40.0f;

	// Pozitif tekerlek (yakınlaş) kolu kısaltır
	TestEqual(TEXT("yakınlaş: 350 -> 310"), ApplyZoomStep(350.0f, 1.0f, Step, Min, Max), 310.0f, 1e-4f);

	// Negatif tekerlek (uzaklaş) kolu uzatır
	TestEqual(TEXT("uzaklaş: 350 -> 390"), ApplyZoomStep(350.0f, -1.0f, Step, Min, Max), 390.0f, 1e-4f);

	// Sıfır delta değişmez
	TestEqual(TEXT("sıfır delta değişmez"), ApplyZoomStep(350.0f, 0.0f, Step, Min, Max), 350.0f, 1e-4f);

	// Alt sınır kelepçelenir
	TestEqual(TEXT("min altına inmez"), ApplyZoomStep(160.0f, 5.0f, Step, Min, Max), Min, 1e-4f);

	// Üst sınır kelepçelenir
	TestEqual(TEXT("max üstüne çıkmaz"), ApplyZoomStep(590.0f, -5.0f, Step, Min, Max), Max, 1e-4f);

	// Tam sınırda deltasız kalır
	TestEqual(TEXT("min sınırında sıfır delta"), ApplyZoomStep(Min, 0.0f, Step, Min, Max), Min, 1e-4f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
