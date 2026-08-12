// Baslangic haritasi arazi uretimi — saf yukseklik matematigi birim testleri (HeightmapMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.World"

#include "Misc/AutomationTest.h"
#include "World/HeightmapMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDeterminizmTest,
	"SurvivalGame.World.HeightmapMath.Determinizm",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDeterminizmTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 4242;

	// Ayni tohum + ayni koordinat -> bit-birebir ayni sonuc (iki ayri cagri)
	for (int32 i = 0; i < 32; ++i)
	{
		const float X = static_cast<float>(i) * 13.7f;
		const float Y = static_cast<float>(i) * 7.3f;
		TestEqual(TEXT("SampleFBM deterministik"), SampleFBM(X, Y, P), SampleFBM(X, Y, P));
	}

	// Tum heightmap de deterministik
	const TArray<uint16> A = GenerateHeightmap(64, 64, P);
	const TArray<uint16> B = GenerateHeightmap(64, 64, P);
	TestEqual(TEXT("heightmap uzunlugu ayni"), A.Num(), B.Num());
	bool bIdentical = (A.Num() == B.Num());
	for (int32 i = 0; bIdentical && i < A.Num(); ++i)
	{
		bIdentical = (A[i] == B[i]);
	}
	TestTrue(TEXT("heightmap bit-birebir ayni"), bIdentical);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapTohumAyrimiTest,
	"SurvivalGame.World.HeightmapMath.TohumAyrimi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapTohumAyrimiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P1; P1.Seed = 1;
	FHeightmapParams P2; P2.Seed = 2;
	// Duzlestirme bu testi bozmasin: maskeyi kapsam disina it
	P1.FlattenRadius = 0.0f; P1.FlattenFalloff = 0.0f;
	P2.FlattenRadius = 0.0f; P2.FlattenFalloff = 0.0f;

	double TotalDiff = 0.0;
	const int32 Samples = 256;
	for (int32 i = 0; i < Samples; ++i)
	{
		const float X = static_cast<float>(i) * 11.0f;
		const float Y = static_cast<float>(i) * 5.0f;
		TotalDiff += FMath::Abs(SampleFBM(X, Y, P1) - SampleFBM(X, Y, P2));
	}
	const double MeanDiff = TotalDiff / static_cast<double>(Samples);

	// Farkli tohum belirgin farkli arazi vermeli (ayni cikti = tohum kullanilmiyor demek)
	TestTrue(TEXT("farkli tohum -> belirgin fark"), MeanDiff > 0.02);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDegerAraligiTest,
	"SurvivalGame.World.HeightmapMath.DegerAraligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDegerAraligiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 99;

	// SampleFBM her zaman [0,1]
	bool bInUnitRange = true;
	for (int32 i = 0; i < 512; ++i)
	{
		const float V = SampleFBM(static_cast<float>(i) * 3.1f, static_cast<float>(i) * 2.7f, P);
		bInUnitRange = bInUnitRange && (V >= 0.0f) && (V <= 1.0f);
	}
	TestTrue(TEXT("SampleFBM [0,1] icinde"), bInUnitRange);

	// GenerateHeightmap ciktisi uint16 araliginda ve dogru uzunlukta
	const TArray<uint16> H = GenerateHeightmap(48, 32, P);
	TestEqual(TEXT("heightmap uzunlugu SizeX*SizeY"), H.Num(), 48 * 32);

	// Hedeflenen bant: HeightMidpoint +/- HeightSpan/2 icinde kalmali (tasma/sarma yok)
	const int32 Lo = HeightMidpoint - FMath::CeilToInt(HeightSpan * 0.5f) - 1;
	const int32 Hi = HeightMidpoint + FMath::CeilToInt(HeightSpan * 0.5f) + 1;
	bool bInBand = true;
	for (int32 i = 0; i < H.Num(); ++i)
	{
		bInBand = bInBand && (static_cast<int32>(H[i]) >= Lo) && (static_cast<int32>(H[i]) <= Hi);
	}
	TestTrue(TEXT("heightmap hedef bant icinde"), bInBand);

	// Bozuk girdi cokmez, bos doner
	TestEqual(TEXT("SizeX=0 -> bos"), GenerateHeightmap(0, 32, P).Num(), 0);
	TestEqual(TEXT("negatif boyut -> bos"), GenerateHeightmap(-4, 32, P).Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDuzlestirmeMaskesiTest,
	"SurvivalGame.World.HeightmapMath.DuzlestirmeMaskesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDuzlestirmeMaskesiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 7;
	P.FlattenCenter = FIntPoint(100, 100);
	P.FlattenRadius = 20.0f;
	P.FlattenFalloff = 50.0f;
	P.FlattenHeight = 0.35f;

	// Yaricap ICINDE: her zaman tam olarak FlattenHeight (insaat izgarasi duz zemin ister)
	bool bFlat = true;
	for (int32 dy = -20; dy <= 20; ++dy)
	{
		for (int32 dx = -20; dx <= 20; ++dx)
		{
			if (FMath::Sqrt(static_cast<float>(dx * dx + dy * dy)) > P.FlattenRadius) { continue; }
			const float Raw = SampleFBM(static_cast<float>(100 + dx), static_cast<float>(100 + dy), P);
			const float Masked = ApplyFlattenMask(Raw, 100 + dx, 100 + dy, P);
			bFlat = bFlat && FMath::IsNearlyEqual(Masked, P.FlattenHeight, KINDA_SMALL_NUMBER);
		}
	}
	TestTrue(TEXT("yaricap icinde tam duz"), bFlat);

	// Falloff DISINDA: maske hic etki etmemeli
	const int32 FarX = 100 + 200;
	const float RawFar = SampleFBM(static_cast<float>(FarX), 100.0f, P);
	TestEqual(TEXT("falloff disinda maske etkisiz"), ApplyFlattenMask(RawFar, FarX, 100, P), RawFar);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapGecisSurekliligiTest,
	"SurvivalGame.World.HeightmapMath.GecisSurekliligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapGecisSurekliligiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 11;
	P.FlattenCenter = FIntPoint(0, 0);
	P.FlattenRadius = 20.0f;
	P.FlattenFalloff = 60.0f;
	P.FlattenHeight = 0.35f;

	// Duz bolgeden gurultulu bolgeye gecerken ani sicrama olmamali:
	// komsu ornekler arasindaki fark kucuk kalmali (duvar/ucurum olusmaz)
	float MaxStep = 0.0f;
	float Prev = ApplyFlattenMask(SampleFBM(0.0f, 0.0f, P), 0, 0, P);
	for (int32 X = 1; X <= 100; ++X)
	{
		const float Cur = ApplyFlattenMask(SampleFBM(static_cast<float>(X), 0.0f, P), X, 0, P);
		MaxStep = FMath::Max(MaxStep, FMath::Abs(Cur - Prev));
		Prev = Cur;
	}
	TestTrue(TEXT("komsu ornekler arasi sicrama sinirli"), MaxStep < 0.25f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
