// Sistem #11 birim testleri — saf agirlikli-secim/gecis-ilerleme/interpolasyon mantigi (WeatherMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Weather"

#include "Misc/AutomationTest.h"
#include "Weather/WeatherMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeatherMathWeightedSelectTest,
	"SurvivalGame.Weather.WeatherMath.AgirlikliSecim",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWeatherMathWeightedSelectTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWeather;

	// Esit agirlikli 4 secenek (toplam 4.0): [0,1)->0, [1,2)->1, [2,3)->2, [3,4)->3 (toplam-cebirsel)
	const TArray<float> EqualWeights = { 1.0f, 1.0f, 1.0f, 1.0f };
	TestEqual(TEXT("RandomValue 0.0 -> index 0"), SelectWeightedIndex(EqualWeights, 0.0f), 0);
	TestEqual(TEXT("RandomValue 0.24 -> index 0 (0.96 < 1.0)"), SelectWeightedIndex(EqualWeights, 0.24f), 0);
	TestEqual(TEXT("RandomValue 0.26 -> index 1 (1.04 >= 1.0)"), SelectWeightedIndex(EqualWeights, 0.26f), 1);
	TestEqual(TEXT("RandomValue 0.99 -> index 3 (son dilim)"), SelectWeightedIndex(EqualWeights, 0.99f), 3);

	// Agirlikli: [10, 0, 0] -> ne olursa olsun index 0 (digerleri 0 agirlikli)
	const TArray<float> SkewedWeights = { 10.0f, 0.0f, 0.0f };
	TestEqual(TEXT("tek agirlikli secenek -> her zaman index 0"), SelectWeightedIndex(SkewedWeights, 0.9f), 0);

	// Bos/tumu-sifir agirlik -> guvenlik icin 0
	const TArray<float> ZeroWeights = { 0.0f, 0.0f };
	TestEqual(TEXT("tumu sifir agirlik -> guvenli 0"), SelectWeightedIndex(ZeroWeights, 0.5f), 0);
	TestEqual(TEXT("bos liste -> guvenli 0"), SelectWeightedIndex(TArray<float>(), 0.5f), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeatherMathTransitionTest,
	"SurvivalGame.Weather.WeatherMath.GecisVeInterpolasyon",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWeatherMathTransitionTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWeather;

	TestEqual(TEXT("yarisinda ilerleme 0.5"), ComputeTransitionProgress(5.0f, 10.0f), 0.5f);
	TestEqual(TEXT("asilirsa 1'e kelepcelenir"), ComputeTransitionProgress(15.0f, 10.0f), 1.0f);
	TestEqual(TEXT("TransitionDuration<=0 -> her zaman tamamlanmis"), ComputeTransitionProgress(0.0f, 0.0f), 1.0f);

	FWeatherState From;
	From.Condition = EWeatherCondition::Clear;
	From.Temperature = 10.0f;
	From.Precipitation = 0.0f;

	FWeatherState To;
	To.Condition = EWeatherCondition::Rainy;
	To.Temperature = 20.0f;
	To.Precipitation = 1.0f;

	// Baslangicta (Alpha=0): degerler From'da, Condition From'da
	{
		const FWeatherState Mid = LerpWeatherState(From, To, 0.0f);
		TestEqual(TEXT("Alpha=0 -> sicaklik From'da"), Mid.Temperature, 10.0f);
		TestEqual(TEXT("Alpha=0 -> Condition From'da (Clear=0)"), static_cast<int32>(Mid.Condition), static_cast<int32>(EWeatherCondition::Clear));
	}

	// Yarida (Alpha=0.5): degerler ortada, Condition HALA From'da (henuz tamamlanmadi)
	{
		const FWeatherState Mid = LerpWeatherState(From, To, 0.5f);
		TestEqual(TEXT("Alpha=0.5 -> sicaklik ortada"), Mid.Temperature, 15.0f);
		TestEqual(TEXT("Alpha=0.5 -> Condition HALA From'da"), static_cast<int32>(Mid.Condition), static_cast<int32>(EWeatherCondition::Clear));
	}

	// Tamamlaninca (Alpha=1): degerler To'da, Condition artik To'ya gecti
	{
		const FWeatherState Mid = LerpWeatherState(From, To, 1.0f);
		TestEqual(TEXT("Alpha=1 -> sicaklik To'da"), Mid.Temperature, 20.0f);
		TestEqual(TEXT("Alpha=1 -> Condition To'ya gecti (Rainy)"), static_cast<int32>(Mid.Condition), static_cast<int32>(EWeatherCondition::Rainy));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWeatherMathWindDirectionTest,
	"SurvivalGame.Weather.WeatherMath.RuzgarYonu",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWeatherMathWindDirectionTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalWeather;

	// 0 derece -> (1,0,0)
	{
		const FVector Dir = ComputeWindDirection(0.0f);
		TestEqual(TEXT("0 derece X"), Dir.X, 1.0);
		TestEqual(TEXT("0 derece Y"), Dir.Y, 0.0);
	}

	// 0.25 -> 90 derece -> (0,1,0) (kucuk kayan-nokta toleransiyla)
	{
		const FVector Dir = ComputeWindDirection(0.25f);
		TestTrue(TEXT("90 derece X ~0"), FMath::IsNearlyEqual(Dir.X, 0.0, 0.001));
		TestTrue(TEXT("90 derece Y ~1"), FMath::IsNearlyEqual(Dir.Y, 1.0, 0.001));
	}

	// Her zaman birim vektor (Z=0)
	{
		const FVector Dir = ComputeWindDirection(0.6f);
		TestTrue(TEXT("birim vektor"), FMath::IsNearlyEqual(Dir.SizeSquared(), 1.0, 0.001));
		TestEqual(TEXT("Z her zaman 0"), Dir.Z, 0.0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
