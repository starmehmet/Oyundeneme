// Sistem #18 birim testleri — saf hucre-esleme/histerezis matematigi (WorldPartitionMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.World"

#include "Misc/AutomationTest.h"
#include "World/WorldPartitionMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldPartitionMathCellTest,
	"SurvivalGame.World.WorldPartitionMath.HucreEslemesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWorldPartitionMathCellTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWorldPartition;

	// Pozitif konumlar dogru hucreye eslenir
	TestTrue(TEXT("pozitif konum -> dogru hucre"), GetCellForPosition(FVector(6500.0, 100.0, 0.0), 6400.0f) == FIntPoint(1, 0));

	// Negatif konumlar TABAN BOLME ile dogru eslenir (yuvarlama degil)
	TestTrue(TEXT("negatif konum -> dogru hucre (taban bolme)"), GetCellForPosition(FVector(-100.0, -6500.0, 0.0), 6400.0f) == FIntPoint(-1, -2));

	// Orijin -> (0,0) hucresi
	TestTrue(TEXT("orijin -> (0,0)"), GetCellForPosition(FVector(0.0, 0.0, 0.0), 6400.0f) == FIntPoint(0, 0));

	// Hucre merkezi dogru hesaplanir
	const FVector Center = GetCellCenter(FIntPoint(1, 0), 6400.0f);
	TestEqual(TEXT("hucre merkezi X"), Center.X, 9600.0);
	TestEqual(TEXT("hucre merkezi Y"), Center.Y, 3200.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldPartitionMathHysteresisTest,
	"SurvivalGame.World.WorldPartitionMath.HisterezisliYukleBosalt",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWorldPartitionMathHysteresisTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWorldPartition;

	const float LoadRadius = 8000.0f;
	const float UnloadRadius = 12000.0f;

	// LoadRadius icinde -> yuklenmeli
	TestTrue(TEXT("LoadRadius icinde -> yuklenmeli"), ShouldCellBeLoaded(5000.0f, LoadRadius));
	TestFalse(TEXT("LoadRadius disinda -> yuklenmemeli"), ShouldCellBeLoaded(9000.0f, LoadRadius));

	// UnloadRadius'u asmadan -> BOSALTILMAMALI (histerezis: LoadRadius ile UnloadRadius arasi "karasiz bolge" degil, "kararli-yuklu" bolge)
	TestFalse(TEXT("LoadRadius ile UnloadRadius arasi -> bosaltilmamali"), ShouldCellBeUnloaded(9000.0f, UnloadRadius));

	// UnloadRadius'u asinca -> bosaltilmali
	TestTrue(TEXT("UnloadRadius asilinca -> bosaltilmali"), ShouldCellBeUnloaded(13000.0f, UnloadRadius));

	// Sinirlarda: tam LoadRadius -> yuklenmeli (<=); tam UnloadRadius -> HENUZ bosaltilmamali (>)
	TestTrue(TEXT("tam LoadRadius -> yuklenmeli"), ShouldCellBeLoaded(LoadRadius, LoadRadius));
	TestFalse(TEXT("tam UnloadRadius -> henuz bosaltilmamali"), ShouldCellBeUnloaded(UnloadRadius, UnloadRadius));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
