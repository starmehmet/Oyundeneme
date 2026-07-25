// Sistem #6 birim testleri — FRecipeDefinition::GetAggregatedIngredients.
// Çalıştırma: Editor → Session Frontend → Automation → "SurvivalGame.Crafting"

#include "Misc/AutomationTest.h"
#include "Crafting/RecipeDefinition.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecipeDefinitionAggregateTest,
	"SurvivalGame.Crafting.RecipeDefinition.ToplananMalzemeler",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRecipeDefinitionAggregateTest::RunTest(const FString& Parameters)
{
	FRecipeDefinition Recipe;

	// Regresyon (inceleme bulgusu): ayni ItemID iki ayri satirda gecerse toplanmali,
	// yoksa satir-satir bagimsiz kontrol gercek maliyeti oldugundan dusuk gosterir.
	FRecipeIngredient Wood1;
	Wood1.ItemID = FName(TEXT("Odun"));
	Wood1.Count = 5;
	FRecipeIngredient Wood2;
	Wood2.ItemID = FName(TEXT("Odun"));
	Wood2.Count = 3;
	FRecipeIngredient Stone;
	Stone.ItemID = FName(TEXT("Tas"));
	Stone.Count = 2;
	Recipe.Ingredients = { Wood1, Wood2, Stone };

	const TMap<FName, int32> Aggregated = Recipe.GetAggregatedIngredients();

	TestEqual(TEXT("2 benzersiz ItemID'ye toplanmali (Odun, Tas)"), Aggregated.Num(), 2);
	TestTrue(TEXT("Odun anahtari var"), Aggregated.Contains(FName(TEXT("Odun"))));
	TestEqual(TEXT("Odun: 5+3=8 olarak toplanmali"), Aggregated.FindRef(FName(TEXT("Odun"))), 8);
	TestEqual(TEXT("Tas: tek satir, 2 kalmali"), Aggregated.FindRef(FName(TEXT("Tas"))), 2);

	// Bos tarif -> bos harita
	FRecipeDefinition EmptyRecipe;
	TestEqual(TEXT("bos Ingredients -> bos harita"), EmptyRecipe.GetAggregatedIngredients().Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
