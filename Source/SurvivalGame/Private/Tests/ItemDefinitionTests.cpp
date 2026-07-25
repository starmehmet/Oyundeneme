// Sistem #5 birim testleri — FItemDefinition saf yardimci metodlari.
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Items"

#include "Misc/AutomationTest.h"
#include "Items/ItemDefinition.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemDefinitionPredicateTest,
	"SurvivalGame.Items.ItemDefinition.Yardimcilar",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FItemDefinitionPredicateTest::RunTest(const FString& Parameters)
{
	FItemDefinition Stackable;
	Stackable.MaxStackSize = 20;
	Stackable.MaxDurability = 0.0f;
	Stackable.Tags = { TEXT("Yakit"), TEXT("Yanabilir") };

	FItemDefinition NonStackableTool;
	NonStackableTool.MaxStackSize = 1;
	NonStackableTool.MaxDurability = 100.0f;

	TestTrue(TEXT("MaxStackSize=20 yigilabilir"), Stackable.IsStackable());
	TestFalse(TEXT("MaxStackSize=1 yigilamaz"), NonStackableTool.IsStackable());

	TestFalse(TEXT("MaxDurability=0 dayaniklilik takip etmez"), Stackable.HasDurability());
	TestTrue(TEXT("MaxDurability=100 dayaniklilik takip eder"), NonStackableTool.HasDurability());

	TestTrue(TEXT("'Yakit' etiketi var"), Stackable.HasTag(TEXT("Yakit")));
	TestFalse(TEXT("'Silah' etiketi yok"), Stackable.HasTag(TEXT("Silah")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
