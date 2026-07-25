// Sistem #20 birim testleri — saf giris-yonlendirme mantigi (UIMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.UI"

#include "Misc/AutomationTest.h"
#include "UI/UIMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIMathCaptureInputTest,
	"SurvivalGame.UI.UIMath.GirisYakalama",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUIMathCaptureInputTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalUI;

	// Yigin bos -> oyun girisi (UI yakalamaz)
	TestFalse(TEXT("derinlik=0 -> UI yakalamaz"), ShouldCaptureUIInput(0));

	// Yigin dolu -> UI girisi yakalar
	TestTrue(TEXT("derinlik=1 -> UI yakalar"), ShouldCaptureUIInput(1));
	TestTrue(TEXT("derinlik=5 -> UI yakalar"), ShouldCaptureUIInput(5));

	// Negatif derinlik (cagiran hatasi) -> guvenlik icin bos sayilir
	TestFalse(TEXT("negatif derinlik -> guvenlik icin bos sayilir"), ShouldCaptureUIInput(-1));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
