// Sistem #9 birim testleri — saf ilerleme/bloke-durumu/kare-bolumleme mantigi (ProductionMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Production"

#include "Misc/AutomationTest.h"
#include "Production/ProductionMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProductionMathProgressTest,
	"SurvivalGame.Production.ProductionMath.Ilerleme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FProductionMathProgressTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalProduction;

	TestEqual(TEXT("yarisinda ilerleme 0.5"), ComputeProductionProgress(1.0f, 2.0f), 0.5f);
	TestEqual(TEXT("asilirsa 1'e kelepcelenir"), ComputeProductionProgress(5.0f, 2.0f), 1.0f);
	TestEqual(TEXT("ProductionTime<=0 -> her zaman tamamlanmis (1.0)"), ComputeProductionProgress(0.0f, 0.0f), 1.0f);

	TestFalse(TEXT("henuz tamamlanmadi"), IsProductionComplete(1.0f, 2.0f));
	TestTrue(TEXT("tam sinirda tamamlandi"), IsProductionComplete(2.0f, 2.0f));
	TestTrue(TEXT("asilinca tamamlandi"), IsProductionComplete(3.0f, 2.0f));

	// Enerji: hiz*sure, negatifler 0'a kelepcelenir
	TestEqual(TEXT("2 enerji/sn * 3sn = 6"), ComputeEnergyConsumed(2.0f, 3.0f), 6.0f);
	TestEqual(TEXT("negatif hiz -> 0"), ComputeEnergyConsumed(-5.0f, 3.0f), 0.0f);
	TestEqual(TEXT("negatif sure -> 0"), ComputeEnergyConsumed(2.0f, -1.0f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProductionMathBlockedStateTest,
	"SurvivalGame.Production.ProductionMath.BlokeDurumu",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FProductionMathBlockedStateTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalProduction;

	// TestEqual'in EProductionState icin bir asiri yuklemesi yok (yalniz int32/int64/float/double/
	// FVector/... — bkz. AutomationTest.h) — enum degerlerini int32'ye acikca donusturerek karsilastir.
	auto AsInt = [](EProductionState State) { return static_cast<int32>(State); };

	TestEqual(TEXT("hepsi tamam -> Running"), AsInt(DetermineBlockedState(true, true, true)), AsInt(EProductionState::Running));
	TestEqual(TEXT("girdi yok -> Blocked_NoInput"), AsInt(DetermineBlockedState(false, true, true)), AsInt(EProductionState::Blocked_NoInput));
	TestEqual(TEXT("cikti yeri yok -> Blocked_NoOutput"), AsInt(DetermineBlockedState(true, false, true)), AsInt(EProductionState::Blocked_NoOutput));
	TestEqual(TEXT("yakit yok -> Blocked_NoFuel"), AsInt(DetermineBlockedState(true, true, false)), AsInt(EProductionState::Blocked_NoFuel));

	// Oncelik: yakit > girdi > cikti (bkz. fonksiyon yorumu) — birden fazlasi eksikse yakit kazanir
	TestEqual(TEXT("yakit VE girdi yok -> yakit kazanir"), AsInt(DetermineBlockedState(false, true, false)), AsInt(EProductionState::Blocked_NoFuel));
	TestEqual(TEXT("girdi VE cikti yok -> girdi kazanir"), AsInt(DetermineBlockedState(false, false, true)), AsInt(EProductionState::Blocked_NoInput));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProductionMathBatchSizeTest,
	"SurvivalGame.Production.ProductionMath.KareBolumleme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FProductionMathBatchSizeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalProduction;

	// CLAUDE.md ornegi: 500 makine / 60 frame -> karede ~9 makine (ceil(500/60)=9)
	TestEqual(TEXT("500/60 -> 9 (yukari yuvarlanir)"), ComputeBatchSize(500, 60), 9);

	// Kucuk listeler: her zaman en az 1
	TestEqual(TEXT("5 makine / 60 frame -> en az 1"), ComputeBatchSize(5, 60), 1);
	TestEqual(TEXT("1 makine / 60 frame -> 1"), ComputeBatchSize(1, 60), 1);

	// Bos liste -> 0 (islenecek bir sey yok)
	TestEqual(TEXT("0 makine -> 0"), ComputeBatchSize(0, 60), 0);

	// Tam bolunen durum
	TestEqual(TEXT("120/60 -> tam 2"), ComputeBatchSize(120, 60), 2);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
