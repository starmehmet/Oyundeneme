// Sistem #19 birim testleri — saf ses-karisimi/budama matematigi (AudioMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Audio"

#include "Misc/AutomationTest.h"
#include "Audio/AudioMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioMathEffectiveVolumeTest,
	"SurvivalGame.Audio.AudioMath.EfektifSesSeviyesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioMathEffectiveVolumeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalAudio;

	// Uc katman carpimsal karisir
	TestEqual(TEXT("1.0 * 0.5 * 0.5 = 0.25"), ComputeEffectiveVolume(1.0f, 0.5f, 0.5f), 0.25f);

	// Master sifirsa hicbir sey duyulmaz (kategori/taban ne olursa olsun)
	TestEqual(TEXT("master=0 -> efektif=0"), ComputeEffectiveVolume(1.0f, 1.0f, 0.0f), 0.0f);

	// Girdiler [0,1] araligina kelepcelenir (UI'dan sizan asiri/negatif deger guvenli)
	TestEqual(TEXT("asiri buyuk taban -> 1.0'a kelepcelenir"), ComputeEffectiveVolume(5.0f, 1.0f, 1.0f), 1.0f);
	TestEqual(TEXT("negatif kategori -> 0'a kelepcelenir"), ComputeEffectiveVolume(1.0f, -1.0f, 1.0f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioMathCullTest,
	"SurvivalGame.Audio.AudioMath.SesBudamasi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioMathCullTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalAudio;

	// Menzil icinde -> budanmaz
	TestFalse(TEXT("menzil icinde -> budanmaz"), ShouldCullSound(500.0f, 1000.0f));

	// Menzil disinda -> budanir
	TestTrue(TEXT("menzil disinda -> budanir"), ShouldCullSound(1500.0f, 1000.0f));

	// Tam sinirda -> HENUZ budanmamali (>, >= degil)
	TestFalse(TEXT("tam sinirda -> budanmamali"), ShouldCullSound(1000.0f, 1000.0f));

	// MaxAudibleDistance<=0 -> sinirsiz menzil, asla budanmaz (hem 0 hem NEGATIF icin — sozlesme "<=0")
	TestFalse(TEXT("sinirsiz menzil (=0) -> asla budanmaz"), ShouldCullSound(999999.0f, 0.0f));
	TestFalse(TEXT("sinirsiz menzil (negatif) -> asla budanmaz"), ShouldCullSound(999999.0f, -1.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
