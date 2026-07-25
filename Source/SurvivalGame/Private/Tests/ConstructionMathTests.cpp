// Sistem #7 birim testleri — saf izgara koordinat donusumu (ConstructionMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Construction"

#include "Misc/AutomationTest.h"
#include "Construction/ConstructionMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConstructionMathSnapTest,
	"SurvivalGame.Construction.ConstructionMath.IzgaraYapisma",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConstructionMathSnapTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalConstruction;

	// Hucre icinde rahat konumlar (sinir degeri DEGIL — IEEE yuvarlama belirsizligine girmemek icin,
	// bkz. Sistem #1 MinuteOfDayFloat dersi)
	{
		const FIntPoint Coord = WorldToGridCoord(FVector(100.0f, 0.0f, 0.0f), 400.0f);
		TestEqual(TEXT("100 UU -> hucre 0 (X)"), Coord.X, 0);
		TestEqual(TEXT("100 UU -> hucre 0 (Y)"), Coord.Y, 0);
	}
	{
		const FIntPoint Coord = WorldToGridCoord(FVector(300.0f, -300.0f, 0.0f), 400.0f);
		TestEqual(TEXT("300 UU -> hucre 1 (X)"), Coord.X, 1);
		TestEqual(TEXT("-300 UU -> hucre -1 (Y)"), Coord.Y, -1);
	}
	{
		const FIntPoint Coord = WorldToGridCoord(FVector(850.0f, 0.0f, 0.0f), 400.0f);
		TestEqual(TEXT("850 UU -> hucre 2 (en yakina yuvarlanir)"), Coord.X, 2);
	}

	// Gecersiz GridSize -> guvenli varsayilan (0,0), crash/NaN yok
	{
		const FIntPoint Coord = WorldToGridCoord(FVector(500.0f, 500.0f, 0.0f), 0.0f);
		TestEqual(TEXT("GridSize<=0 -> (0,0) X"), Coord.X, 0);
		TestEqual(TEXT("GridSize<=0 -> (0,0) Y"), Coord.Y, 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConstructionMathRoundTripTest,
	"SurvivalGame.Construction.ConstructionMath.GidisDonus",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConstructionMathRoundTripTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalConstruction;

	// GridCoordToWorld dogru hucre merkezini + verilen ZHeight'i dondurmeli
	{
		const FVector World = GridCoordToWorld(FIntPoint(2, -3), 400.0f, 50.0f);
		// FVector bilesenleri UE5'te (LWC) double — float literal ile karsilastirma
		// TestEqual'i belirsiz asiri yukleme yapar (derleme hatasi, C2666).
		TestEqual(TEXT("hucre (2,-3) -> X=800"), World.X, 800.0);
		TestEqual(TEXT("hucre (2,-3) -> Y=-1200"), World.Y, -1200.0);
		TestEqual(TEXT("ZHeight aynen tasinir"), World.Z, 50.0);
	}

	// Tam katlarda gidis-donus birebir ayni hucreye donmeli
	{
		const FIntPoint Original(5, -7);
		const FVector World = GridCoordToWorld(Original, 400.0f);
		const FIntPoint RoundTripped = WorldToGridCoord(World, 400.0f);
		TestEqual(TEXT("gidis-donus X"), RoundTripped.X, Original.X);
		TestEqual(TEXT("gidis-donus Y"), RoundTripped.Y, Original.Y);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
