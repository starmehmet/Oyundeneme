// Baslangic haritasi arazi ureticisi — editor-only.
//
// ALandscapeProxy::Import() WITH_EDITOR icindedir (LandscapeProxy.h:1418, blok satir 1326'da
// aciliyor), dolayisiyla bu dosyanin TAMAMI editor derlemelerine ozeldir. Arazi editor
// zamaninda BIR KEZ uretilir ve .umap'e gomulur; pakete normal veri olarak gider, runtime
// maliyeti sifirdir.
//
// Cagri sirasi motorun kendi "New Landscape" akisindan alinmistir
// (LandscapeEditorDetailCustomization_NewLandscape.cpp:1180-1245).

#include "SurvivalGame.h"

#if WITH_EDITOR

#include "World/HeightmapMath.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// Gecerli UE landscape bolunmesi: bilesen = altbolum x altbolum-quad.
	constexpr int32 QuadsPerSection = 63;
	constexpr int32 SectionsPerComponent = 2;   // 2x2 altbolum
	constexpr int32 ComponentCount = 16;        // 16x16 bilesen
	constexpr int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;  // 126
	constexpr int32 LandscapeVerts = ComponentCount * QuadsPerComponent + 1;     // 2017

	void GenerateLandscape(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: gecerli bir World yok"));
			return;
		}

		SurvivalHeightmap::FHeightmapParams Params;
		Params.FlattenCenter = FIntPoint(LandscapeVerts / 2, LandscapeVerts / 2);
		if (Args.Num() > 0)
		{
			Params.Seed = FCString::Atoi(*Args[0]);
		}

		const double StartTime = FPlatformTime::Seconds();

		TArray<uint16> HeightData = SurvivalHeightmap::GenerateHeightmap(LandscapeVerts, LandscapeVerts, Params);
		if (HeightData.Num() != LandscapeVerts * LandscapeVerts)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: heightmap uretilemedi"));
			return;
		}

		// Motorun bekledigi kapsayicilar. Bos FGuid = taban katman (duzenleme katmani yok).
		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));

		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		MaterialLayerDataPerLayers.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

		// Landscape orijine gore ORTALANIR: baslangic vadisi (duzlestirme merkezi) orijine
		// dusmeli ki icerik koordinatlari okunur kalsin.
		const FVector Scale(100.0, 100.0, 100.0);
		const FVector Offset(
			-ComponentCount * QuadsPerComponent * Scale.X * 0.5,
			-ComponentCount * QuadsPerComponent * Scale.Y * 0.5,
			0.0);

		ALandscape* NewLandscape = World->SpawnActor<ALandscape>(Offset, FRotator::ZeroRotator);
		if (!NewLandscape)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: ALandscape spawn edilemedi"));
			return;
		}

		NewLandscape->SetActorRelativeScale3D(Scale);
		NewLandscape->StaticLightingLOD = FMath::DivideAndRoundUp(
			FMath::CeilLogTwo((LandscapeVerts * LandscapeVerts) / (2048 * 2048) + 1), static_cast<uint32>(2));

		NewLandscape->Import(
			FGuid::NewGuid(),
			0, 0,
			LandscapeVerts - 1, LandscapeVerts - 1,
			SectionsPerComponent, QuadsPerSection,
			HeightDataPerLayers,
			nullptr,
			MaterialLayerDataPerLayers,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>());

		ULandscapeInfo* Info = NewLandscape->GetLandscapeInfo();
		if (Info)
		{
			Info->UpdateLayerInfoMap(NewLandscape);
		}

		const double Elapsed = FPlatformTime::Seconds() - StartTime;
		UE_LOG(LogSurvival, Log,
			TEXT("survival_generate_landscape: tohum=%d vertex=%dx%d bilesen=%dx%d kenar=%.0f UU sure=%.2f sn"),
			Params.Seed, LandscapeVerts, LandscapeVerts, ComponentCount, ComponentCount,
			ComponentCount * QuadsPerComponent * Scale.X, Elapsed);
		UE_LOG(LogSurvival, Log,
			TEXT("survival_generate_landscape: harita HENUZ KAYDEDILMEDI — save_assets cagir"));
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdGenerateLandscape(
		TEXT("survival_generate_landscape"),
		TEXT("Baslangic haritasi arazisini uretir: survival_generate_landscape [Tohum=1337]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(&GenerateLandscape));
}

#endif // WITH_EDITOR
