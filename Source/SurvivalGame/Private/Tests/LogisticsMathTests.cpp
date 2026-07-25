// Sistem #8 birim testleri — saf düğüm-uygunluğu/taşıma-süresi mantığı (LogisticsMath.h).
// Çalıştırma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Logistics"

#include "Misc/AutomationTest.h"
#include "Logistics/LogisticsMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLogisticsMathNodeTypeTest,
	"SurvivalGame.Logistics.LogisticsMath.DugumTipiKabul",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLogisticsMathNodeTypeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalLogistics;

	TestTrue(TEXT("Container kabul eder"), CanNodeTypeAccept(EStorageNodeType::Container));
	TestTrue(TEXT("MachineInput kabul eder"), CanNodeTypeAccept(EStorageNodeType::MachineInput));
	TestTrue(TEXT("Sink kabul eder"), CanNodeTypeAccept(EStorageNodeType::Sink));
	TestFalse(TEXT("MachineOutput kabul etmez (yalnizca uretir)"), CanNodeTypeAccept(EStorageNodeType::MachineOutput));
	TestFalse(TEXT("Source kabul etmez (yalnizca uretir)"), CanNodeTypeAccept(EStorageNodeType::Source));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLogisticsMathTransportTimeTest,
	"SurvivalGame.Logistics.LogisticsMath.TasimaSuresi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLogisticsMathTransportTimeTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalLogistics;

	// 1000 UU / 500 UU/sn = 2 sn
	TestEqual(TEXT("1000 UU, 500 hiz -> 2sn"), ComputeTransportTime(1000.0f, 500.0f), 2.0f);

	// Hiz <=0 -> anlik (0), sifira bolme yok
	TestEqual(TEXT("hiz 0 -> anlik"), ComputeTransportTime(1000.0f, 0.0f), 0.0f);
	TestEqual(TEXT("negatif hiz -> anlik"), ComputeTransportTime(1000.0f, -5.0f), 0.0f);

	// Negatif mesafe -> 0'a kelepcelenir (savunmaci)
	TestEqual(TEXT("negatif mesafe -> 0sn"), ComputeTransportTime(-100.0f, 500.0f), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLogisticsMathProgressTest,
	"SurvivalGame.Logistics.LogisticsMath.Ilerleme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLogisticsMathProgressTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalLogistics;

	TestEqual(TEXT("yarisinda ilerleme 0.5"), ComputeTransportProgress(1.0f, 2.0f), 0.5f);
	TestEqual(TEXT("asilirsa 1'e kelepcelenir"), ComputeTransportProgress(5.0f, 2.0f), 1.0f);
	TestEqual(TEXT("TransportTime<=0 -> her zaman tamamlanmis (1.0)"), ComputeTransportProgress(0.0f, 0.0f), 1.0f);

	TestFalse(TEXT("henuz tamamlanmadi"), IsTransportComplete(1.0f, 2.0f));
	TestTrue(TEXT("tam sinirda tamamlandi"), IsTransportComplete(2.0f, 2.0f));
	TestTrue(TEXT("asilinca tamamlandi"), IsTransportComplete(3.0f, 2.0f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
