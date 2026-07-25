// Sistem #16 birim testleri — saf gorev secim/oncelik/beceri/backoff matematigi (TaskSchedulerMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.NPC"

#include "Misc/AutomationTest.h"
#include "NPC/TaskSchedulerMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTaskSchedulerMathQualificationTest,
	"SurvivalGame.NPC.TaskSchedulerMath.BeceriVeKullanilabilirlik",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTaskSchedulerMathQualificationTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTaskScheduler;

	TestTrue(TEXT("beceri yeterli -> uygun"), IsNPCQualified(3, 2));
	TestTrue(TEXT("beceri tam esit -> uygun"), IsNPCQualified(2, 2));
	TestFalse(TEXT("beceri yetersiz -> uygun degil"), IsNPCQualified(1, 2));

	TestTrue(TEXT("backoff suresi gecti -> kullanilabilir"), IsTaskAvailable(10.0, 15.0));
	TestTrue(TEXT("backoff suresi tam simdi -> kullanilabilir"), IsTaskAvailable(10.0, 10.0));
	TestFalse(TEXT("backoff suresi henuz gecmedi -> kullanilamaz"), IsTaskAvailable(10.0, 5.0));

	TestEqual(TEXT("backoff hesabi"), ComputeBackoffAvailableTime(100.0, 30.0f), 130.0);
	TestEqual(TEXT("negatif backoff kelepcelenir"), ComputeBackoffAvailableTime(100.0, -5.0f), 100.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTaskSchedulerMathSelectionTest,
	"SurvivalGame.NPC.TaskSchedulerMath.EnIyiGorevSecimi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTaskSchedulerMathSelectionTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalTaskScheduler;

	FTaskDefinition LowPriority;
	LowPriority.TaskID = TEXT("Low");
	LowPriority.Priority = 1.0f;
	LowPriority.RequiredSkillLevel = 0;

	FTaskDefinition HighPriority;
	HighPriority.TaskID = TEXT("High");
	HighPriority.Priority = 5.0f;
	HighPriority.RequiredSkillLevel = 0;

	FTaskDefinition TooSkilled;
	TooSkilled.TaskID = TEXT("TooSkilled");
	TooSkilled.Priority = 10.0f; // en yuksek oncelik ama...
	TooSkilled.RequiredSkillLevel = 5; // ...bu NPC'nin becerisi yetmiyor

	FTaskDefinition OnBackoff;
	OnBackoff.TaskID = TEXT("OnBackoff");
	OnBackoff.Priority = 20.0f; // en yuksek oncelik ama...
	OnBackoff.RequiredSkillLevel = 0;
	OnBackoff.AvailableAfterGameTime = 1000.0; // ...henuz kullanilabilir degil

	TArray<FTaskDefinition> Tasks = { LowPriority, HighPriority, TooSkilled, OnBackoff };

	// NPC beceri=1, oyun-zamani=0.0: TooSkilled beceri yetersizligiyle, OnBackoff zamanlamayla
	// elenir -> geriye kalan Low/High arasinda EN YUKSEK oncelikli High secilmeli
	const int32 BestIndex = FindBestEligibleTaskIndex(Tasks, 1, 0.0);
	TestEqual(TEXT("uygun gorevler arasinda en yuksek oncelikli secilir"), BestIndex, 1); // HighPriority index=1

	// Hicbir gorev uygun degilse INDEX_NONE
	TArray<FTaskDefinition> OnlyIneligible = { TooSkilled, OnBackoff };
	TestEqual(TEXT("hicbir uygun gorev yok -> INDEX_NONE"), FindBestEligibleTaskIndex(OnlyIneligible, 1, 0.0), static_cast<int32>(INDEX_NONE));

	// Bos liste -> INDEX_NONE
	TestEqual(TEXT("bos liste -> INDEX_NONE"), FindBestEligibleTaskIndex(TArray<FTaskDefinition>(), 10, 0.0), static_cast<int32>(INDEX_NONE));

	// Yeterli beceri VE zaman gecince TooSkilled/OnBackoff da uygun hale gelir, en yuksek oncelikli (OnBackoff) secilir
	const int32 BestIndexLater = FindBestEligibleTaskIndex(Tasks, 5, 1000.0);
	TestEqual(TEXT("beceri+backoff karsilaninca en yuksek oncelikli secilir"), BestIndexLater, 3); // OnBackoff index=3

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
