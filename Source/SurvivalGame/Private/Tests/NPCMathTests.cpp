// Sistem #15 birim testleri — saf NPC durum/yorgunluk/moral matematigi (NPCMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.NPC"

#include "Misc/AutomationTest.h"
#include "NPC/NPCMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCMathStateTest,
	"SurvivalGame.NPC.NPCMath.DurumOncelikSirasi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNPCMathStateTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalNPC;

	// Hic bir sart yoksa -> Idle
	TestTrue(TEXT("hicbir sart yok -> Idle"), DetermineNPCState(false, false, false, false, false) == ENPCState::Idle);

	// Gorev var ama konumda degil -> Walking
	TestTrue(TEXT("gorev var, konumda degil -> Walking"), DetermineNPCState(false, false, false, true, false) == ENPCState::Walking);

	// Gorev var VE konumda -> Working
	TestTrue(TEXT("gorev var, konumda -> Working"), DetermineNPCState(false, false, false, true, true) == ENPCState::Working);

	// Uyku zamani, gorev olsa bile -> Sleeping (uyku gorevi ezer)
	TestTrue(TEXT("uyku zamani gorevi ezer -> Sleeping"), DetermineNPCState(false, true, false, true, true) == ENPCState::Sleeping);

	// Yemek yiyor, uyku olmasa bile gorevi ezer -> Eating
	TestTrue(TEXT("yemek gorevi ezer -> Eating"), DetermineNPCState(false, false, true, true, true) == ENPCState::Eating);

	// Yarali her seyi ezer -> Hurt (en yuksek oncelik)
	TestTrue(TEXT("yarali her seyi ezer -> Hurt"), DetermineNPCState(true, true, true, true, true) == ENPCState::Hurt);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCMathFatigueMoraleTest,
	"SurvivalGame.NPC.NPCMath.YorgunlukVeMoral",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNPCMathFatigueMoraleTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalNPC;

	// Calisirken yorgunluk artar
	TestEqual(TEXT("calisirken yorgunluk artar"), ComputeFatigueDelta(true, false, 2.0f, 5.0f, 1.0f), 2.0f);

	// Uyurken yorgunluk azalir (negatif delta)
	TestEqual(TEXT("uyurken yorgunluk azalir"), ComputeFatigueDelta(false, true, 2.0f, 5.0f, 1.0f), -5.0f);

	// Idle/Walking -> notr
	TestEqual(TEXT("ne calisiyor ne uyuyor -> notr"), ComputeFatigueDelta(false, false, 2.0f, 5.0f, 1.0f), 0.0f);

	// Asiri calisma esigi altinda -> moral toparlanir (pozitif)
	TestTrue(TEXT("esik altinda -> moral toparlanir"), ComputeMoraleDelta(0.5f, 0.05f, 0.1f, 0.8f, 1.0f) > 0.0f);

	// Asiri calisma esigi ustunde -> moral duser (negatif)
	TestTrue(TEXT("esik ustunde -> moral duser"), ComputeMoraleDelta(0.9f, 0.05f, 0.1f, 0.8f, 1.0f) < 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCMathLocationSleepHurtTest,
	"SurvivalGame.NPC.NPCMath.KonumUykuYaralanma",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNPCMathLocationSleepHurtTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalNPC;

	// Konum kontrolu
	TestTrue(TEXT("yaricap icinde -> vardi"), IsAtLocation(FVector(0, 0, 0), FVector(100, 0, 0), 150.0f));
	TestFalse(TEXT("yaricap disinda -> varmadi"), IsAtLocation(FVector(0, 0, 0), FVector(200, 0, 0), 150.0f));

	// Yaralanma esigi
	TestTrue(TEXT("esigin altinda -> yarali"), IsHurt(20.0f, 100.0f, 0.3f));
	TestFalse(TEXT("esigin ustunde -> yarali degil"), IsHurt(40.0f, 100.0f, 0.3f));

	// Uyku saati — sarmalsiz (ornek: 1-5)
	TestTrue(TEXT("sarmalsiz araliktaki saat -> uyku"), WantsToSleep(3, 1, 5));
	TestFalse(TEXT("sarmalsiz aralik disi saat -> uyku degil"), WantsToSleep(10, 1, 5));

	// Uyku saati — gece-yarisi SARMALI (22-6)
	TestTrue(TEXT("sarmalda gece yarisindan once -> uyku"), WantsToSleep(23, 22, 6));
	TestTrue(TEXT("sarmalda gece yarisindan sonra -> uyku"), WantsToSleep(2, 22, 6));
	TestFalse(TEXT("sarmalda gunduz -> uyku degil"), WantsToSleep(14, 22, 6));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
