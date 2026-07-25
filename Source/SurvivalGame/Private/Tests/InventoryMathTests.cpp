// Sistem #4 birim testleri — saf yığın/ağırlık mantığı (InventoryMath.h).
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Inventory"

#include "Misc/AutomationTest.h"
#include "Inventory/InventoryMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryMathStackTest,
	"SurvivalGame.Inventory.InventoryMath.YiginKabul",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInventoryMathStackTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalInventory;

	// Bos yigin, bol miktar, sinir 10 -> tamami sigar
	TestEqual(TEXT("bos yigina 5/10 tamami siger"), ComputeAcceptedIntoStack(0, 5, 10), 5);
	TestEqual(TEXT("bos yigina 5/10 tasan yok"), ComputeStackOverflow(0, 5, 10), 0);

	// Yarim dolu yigin, sinirdan fazla istek -> tasar
	TestEqual(TEXT("7 dolu, 10 sinir, 5 istek -> 3 kabul"), ComputeAcceptedIntoStack(7, 5, 10), 3);
	TestEqual(TEXT("7 dolu, 10 sinir, 5 istek -> 2 tasar"), ComputeStackOverflow(7, 5, 10), 2);

	// Tam dolu yigin -> hicbiri sigmaz
	TestEqual(TEXT("tam dolu yigina hicbiri sigmaz"), ComputeAcceptedIntoStack(10, 3, 10), 0);
	TestEqual(TEXT("tam dolu yiginda tamami tasar"), ComputeStackOverflow(10, 3, 10), 3);

	// Sifir/negatif istek -> hicbir sey kabul edilmez
	TestEqual(TEXT("sifir istek -> sifir kabul"), ComputeAcceptedIntoStack(0, 0, 10), 0);
	TestEqual(TEXT("negatif istek -> sifir kabul"), ComputeAcceptedIntoStack(0, -5, 10), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryMathWeightTest,
	"SurvivalGame.Inventory.InventoryMath.AgirlikLimiti",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInventoryMathWeightTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalInventory;

	// 40/50 dolu, 5 birim eklemek (agirlik 1) -> 45, limit asilmaz
	TestFalse(TEXT("40+5*1=45 <= 50, asilmaz"), WouldExceedWeightLimit(40.0f, 5.0f, 50.0f));

	// 48/50 dolu, agirlik 1 olan 5 adet -> 53 > 50, asilir
	TestTrue(TEXT("48+5=53 > 50, asilir"), WouldExceedWeightLimit(48.0f, 5.0f, 50.0f));

	// Tam sinirda (48+2=50) -> asilmaz (esitlik kabul)
	TestFalse(TEXT("tam sinirda asilmaz"), WouldExceedWeightLimit(48.0f, 2.0f, 50.0f));

	// Karsilanabilir miktar: 45/50 dolu, birim agirlik 2, 10 adet istek -> yalnizca 2 adet sigar (4 birim <= 5 kalan)
	TestEqual(TEXT("45/50, birim=2, istek=10 -> 2 adet siger"),
		ComputeMaxAffordableCount(45.0f, 50.0f, 2.0f, 10), 2);

	// Agirliksiz oge (UnitWeight=0) -> istegin tamami her zaman karsilanir
	TestEqual(TEXT("agirliksiz oge sinirsiz"), ComputeMaxAffordableCount(49.9f, 50.0f, 0.0f, 999), 999);

	// Kapasite dolu (0 kalan) -> hicbiri karsilanamaz
	TestEqual(TEXT("kapasite dolu -> 0"), ComputeMaxAffordableCount(50.0f, 50.0f, 1.0f, 5), 0);

	// Regresyon: cok kucuk UnitWeight'te epsilon adet-uzayinda buyumemeli (inceleme bulgusu).
	// Kapasite tam dolu (0 kalan), UnitWeight=0.00002 -> DOGRU sonuc 0'dir (5 DEGIL).
	TestEqual(TEXT("kapasite dolu + cok kucuk birim agirlik -> yine 0"),
		ComputeMaxAffordableCount(50.0f, 50.0f, 0.00002f, 10), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
