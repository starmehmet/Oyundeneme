// Sistem #6 birim testleri — saf ilerleme/kuyruk mantığı (CraftingMath.h).
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Crafting"

#include "Misc/AutomationTest.h"
#include "Crafting/CraftingMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCraftingMathProgressTest,
	"SurvivalGame.Crafting.CraftingMath.Ilerleme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCraftingMathProgressTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalCrafting;

	TestEqual(TEXT("baslangic: 0/5 -> 0.0"), ComputeCraftProgress(0.0f, 5.0f), 0.0f, 1e-4f);
	TestEqual(TEXT("yarim: 2.5/5 -> 0.5"), ComputeCraftProgress(2.5f, 5.0f), 0.5f, 1e-4f);
	TestEqual(TEXT("tam: 5/5 -> 1.0"), ComputeCraftProgress(5.0f, 5.0f), 1.0f, 1e-4f);
	TestEqual(TEXT("asim: 8/5 -> 1.0 (kelepcelenir)"), ComputeCraftProgress(8.0f, 5.0f), 1.0f, 1e-4f);
	TestEqual(TEXT("CraftingTime<=0 -> her zaman 1.0 (anlik tarif)"), ComputeCraftProgress(0.0f, 0.0f), 1.0f, 1e-4f);

	TestFalse(TEXT("2.5/5 henuz tamamlanmadi"), IsCraftComplete(2.5f, 5.0f));
	TestTrue(TEXT("5/5 tamamlandi"), IsCraftComplete(5.0f, 5.0f));
	TestTrue(TEXT("8/5 tamamlandi (asilmis)"), IsCraftComplete(8.0f, 5.0f));
	TestTrue(TEXT("CraftingTime=0 -> her zaman tamamlanmis"), IsCraftComplete(0.0f, 0.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCraftingMathQueueTest,
	"SurvivalGame.Crafting.CraftingMath.Kuyruk",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCraftingMathQueueTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalCrafting;

	TestTrue(TEXT("bos kuyruk, sinir 16 -> eklenebilir"), CanEnqueue(0, 16));
	TestTrue(TEXT("15/16 -> eklenebilir"), CanEnqueue(15, 16));
	TestFalse(TEXT("16/16 -> dolu, eklenemez"), CanEnqueue(16, 16));
	TestFalse(TEXT("sinir 0 -> hicbir zaman eklenemez"), CanEnqueue(0, 0));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
