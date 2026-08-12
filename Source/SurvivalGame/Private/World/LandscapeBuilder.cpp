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
#include "Harvesting/HarvestNode.h"
#include "EngineUtils.h"

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

	// ------------------------------------------------------------------------------------------
	// survival_populate_world — genis dunya seyrek dagilimi (Task 6).
	//
	// GenerateLandscape TEK BUYUK arazi uretir; bu komut o arazinin UZERINE binlerce hasat
	// dugumu (agac/kaya/damar/...) dagitir — Task 5'in elle tasarladigi baslangic vadisi HARIC.
	// Izgara + HashCoord deseni HeightmapMath.h ile AYNI felsefeyi tasir: motor RNG'si (Rand /
	// FRandomStream) KULLANILMAZ, (HucreX, HucreY, Tohum) uclusu HER ZAMAN ayni sonucu verir —
	// ayni tohumla komut tekrar calistirilirsa ayni dagilim elde edilir.
	// ------------------------------------------------------------------------------------------

	// GenerateLandscape'teki Offset hesabiyla AYNI formul (bkz. yukarida "Scale(100,100,100)" ve
	// "-ComponentCount * QuadsPerComponent * Scale.X * 0.5"): landscape orijin-merkezli
	// yerlestirildigi icin dunya-uzayi yari-genisligi buradan yeniden turetilir. Olcek (100
	// UU/quad) burada sabit varsayilir — Landscape aktorunun RelativeScale3D'si iki ayri yerden
	// degistirilmez (HeightmapMath.h'nin HeightSpan yorumuyla ayni gerekce).
	constexpr double LandscapeScaleUU = 100.0;
	constexpr double WorldHalfExtentUU =
		static_cast<double>(ComponentCount) * static_cast<double>(QuadsPerComponent) * LandscapeScaleUU * 0.5;

	// Task 5'in elle yerlestirdigi baslangic vadisi orijin cevresinde bu yaricapta — bu alandaki
	// hucreler ATLANIR (uzerine yazilmaz).
	constexpr double StartValleySkipRadiusUU = 15000.0;

	// Asagi dogru dunya trace sinirlari: gercek arazi Z araligi +/-58 m civarinda kaliyor
	// (HeightmapMath.h: HeightSpan=15000 uint16-birimi, Landscape Z olcegi 100/128 UU-per-birim
	// ile carpilinca ~ +/-5860 UU eder) — +/-50000 UU bu araligi rahat kapsayan sabit bir
	// ust/alt sinir.
	constexpr double TraceStartZ = 50000.0;
	constexpr double TraceEndZ = -50000.0;

	// Asiri kucuk bir Yogunluk (orn. yazim hatasi) izgarayi yuz binlerce hucreye cikarip editoru
	// dondurebilir — ProfilingCommands.cpp'nin profile_spawn_stress'teki MaxStressCount
	// kelepcesiyle AYNI gerekce. 1000 UU alt sinir varsayilan 25000'i etkilemez; kazayla asiri
	// kucuk bir deger girilirse dahi izgara ~200x200 hucreyle sinirli kalir.
	constexpr int32 MinDensityUU = 1000;

	/**
	 * Tip agirlikli dagilim (Task 6 brief, toplam %100): Agac 30, Kaya 25, LifBitkisi 10,
	 * MeyveAgaci 10, DemirDamari 8, KomurDamari 7, BakirDamari 4, KilYatagi 3, KumYigini 2,
	 * TuzYatagi 1. Kumulatif esik tablosu: UnitHash [0,1) ilk ustune CIKAMADIGI esige denk gelen
	 * tipi secer.
	 */
	FName PickHarvestNodeType(float UnitHash)
	{
		struct FWeightedNodeType
		{
			const TCHAR* NodeID;
			float CumulativeThreshold;
		};
		static const FWeightedNodeType Table[] = {
			{ TEXT("Agac"),        0.30f },  // %30
			{ TEXT("Kaya"),        0.55f },  // %25
			{ TEXT("LifBitkisi"),  0.65f },  // %10
			{ TEXT("MeyveAgaci"),  0.75f },  // %10
			{ TEXT("DemirDamari"), 0.83f },  // %8
			{ TEXT("KomurDamari"), 0.90f },  // %7
			{ TEXT("BakirDamari"), 0.94f },  // %4
			{ TEXT("KilYatagi"),   0.97f },  // %3
			{ TEXT("KumYigini"),   0.99f },  // %2
			{ TEXT("TuzYatagi"),   1.01f },  // %1 — ust sinir 1.0 degil 1.01: kayan nokta
			                                 // yuvarlamasi UnitHash'i tam 1.0'a tasirsa dahi yakalanir
		};

		for (const FWeightedNodeType& Entry : Table)
		{
			if (UnitHash < Entry.CumulativeThreshold)
			{
				return FName(Entry.NodeID);
			}
		}
		return FName(TEXT("TuzYatagi")); // teorik olarak erisilmez guvenlik agi
	}

	void PopulateWorld(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_populate_world: gecerli bir World yok"));
			return;
		}

		// Inceleme bulgusu (majör): komut idempotent DEGIL — ikinci calistirma ayni
		// hucrelere ikinci bir dugum seti dogurur (ustuste binen dugumler, elle temizlemesi
		// zor). Bloke ETMIYORUZ (kasitli tekrar-calistirma / farkli Yogunluk denemesi meshru
		// olabilir), yalnizca UYARIYORUZ. Vadi ICINDEKI dugumler Task 5'in elle yerlestirdigi
		// baslangic seti oldugu icin sayima DAHIL EDILMEZ — yalnizca vadi DISINDAKI (>15000 UU)
		// mevcut AHarvestNode'lar onceki bir populate kosusundan kalma olabilir.
		{
			int32 ExistingWorldNodeCount = 0;
			for (TActorIterator<AHarvestNode> It(World); It; ++It)
			{
				const AHarvestNode* Existing = *It;
				if (!Existing)
				{
					continue;
				}
				const FVector ExistingLocation = Existing->GetActorLocation();
				const double DistFromOrigin = FMath::Sqrt(
					ExistingLocation.X * ExistingLocation.X + ExistingLocation.Y * ExistingLocation.Y);
				if (DistFromOrigin > StartValleySkipRadiusUU)
				{
					++ExistingWorldNodeCount;
				}
			}
			if (ExistingWorldNodeCount > 0)
			{
				UE_LOG(LogSurvival, Warning,
					TEXT("survival_populate_world: %d mevcut dunya dugumu bulundu (vadi disinda, orijinden >%.0f UU) — tekrar calistirma cogaltir, once temizleyin"),
					ExistingWorldNodeCount, StartValleySkipRadiusUU);
			}
		}

		const int32 Seed = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1337;
		int32 Density = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 25000;
		if (Density < MinDensityUU)
		{
			UE_LOG(LogSurvival, Warning,
				TEXT("survival_populate_world: Yogunluk (%d) cok kucuk — %d UU'ya kelepceleniyor (asiri hucre/donma onlemi)"),
				Density, MinDensityUU);
			Density = MinDensityUU;
		}

		const double StartTime = FPlatformTime::Seconds();
		const int32 HalfCellCount = FMath::FloorToInt32(WorldHalfExtentUU / static_cast<double>(Density));

		FCollisionQueryParams TraceParams;

		int32 PlacedCount = 0;
		int32 ValleySkippedCount = 0;
		int32 NoTraceHitCount = 0;
		int32 SpawnFailedCount = 0;
		TMap<FName, int32> CountsByType;

		for (int32 CellY = -HalfCellCount; CellY <= HalfCellCount; ++CellY)
		{
			for (int32 CellX = -HalfCellCount; CellX <= HalfCellCount; ++CellX)
			{
				const double CellCenterX = static_cast<double>(CellX) * Density;
				const double CellCenterY = static_cast<double>(CellY) * Density;

				// Ayni (HucreX, HucreY, Tohum) her zaman ayni uc karari verir: tip, X-ofseti ve
				// Y-ofseti farkli tuzlanmis (Tohum+1 / Tohum+2) hash'lerden gelir ki uc karar
				// birbirine baglanmasin (ayni hash'i iki eksen icin tekrar kullanmak X/Y
				// ofsetlerini kosegen bir desene kilitlerdi).
				const uint32 OffsetHashX = SurvivalHeightmap::HashCoord(CellX, CellY, Seed + 1);
				const uint32 OffsetHashY = SurvivalHeightmap::HashCoord(CellX, CellY, Seed + 2);

				// Hucre icinde [-Yogunluk/2, +Yogunluk/2) araliginda deterministik ofset — duz
				// izgara gorunumunu kirar, komsu hucrenin alanina tasmaz.
				const double JitterX = (static_cast<double>(SurvivalHeightmap::HashToUnit(OffsetHashX)) - 0.5) * Density;
				const double JitterY = (static_cast<double>(SurvivalHeightmap::HashToUnit(OffsetHashY)) - 0.5) * Density;
				const double SpawnX = CellCenterX + JitterX;
				const double SpawnY = CellCenterY + JitterY;

				// Inceleme bulgusu (kritik, hash bit-birebir yeniden hesaplanarak dogrulandi):
				// vadi-atlama testi HUCRE MERKEZI degil GERCEK
				// spawn noktasi (jitter UYGULANDIKTAN SONRA) uzerinde yapilmali — merkezi vadi
				// disinda olan bir hucrenin jitter'li noktasi (+/-Yogunluk/2 kayabilir) vadinin
				// icine dusebilirdi. Yaricap 15000 UU SABIT (Task 5'in uzak halkasiyla kesisme
				// ihtimali kullanici tarafindan kabul edildi — onemli olan vadiye HICBIR
				// tohumda dugum girmemesi).
				if (FMath::Sqrt(SpawnX * SpawnX + SpawnY * SpawnY) <= StartValleySkipRadiusUU)
				{
					++ValleySkippedCount;
					continue;
				}

				const uint32 TypeHash = SurvivalHeightmap::HashCoord(CellX, CellY, Seed);
				const FName NodeID = PickHarvestNodeType(SurvivalHeightmap::HashToUnit(TypeHash));

				FHitResult Hit;
				const FVector TraceStart(SpawnX, SpawnY, TraceStartZ);
				const FVector TraceEnd(SpawnX, SpawnY, TraceEndZ);
				const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
				if (!bHit)
				{
					++NoTraceHitCount;
					continue;
				}

				AHarvestNode* Node = World->SpawnActor<AHarvestNode>(Hit.ImpactPoint, FRotator::ZeroRotator);
				if (!Node)
				{
					++SpawnFailedCount;
					continue;
				}

				Node->SetNodeID(NodeID);
				++PlacedCount;
				++CountsByType.FindOrAdd(NodeID);
			}
		}

		const double Elapsed = FPlatformTime::Seconds() - StartTime;
		UE_LOG(LogSurvival, Log,
			TEXT("survival_populate_world: tohum=%d yogunluk=%d UU dugum=%d vadi-atlandi=%d trace-yok=%d spawn-basarisiz=%d sure=%.2f sn"),
			Seed, Density, PlacedCount, ValleySkippedCount, NoTraceHitCount, SpawnFailedCount, Elapsed);
		for (const TPair<FName, int32>& Pair : CountsByType)
		{
			UE_LOG(LogSurvival, Log, TEXT("survival_populate_world:   %s x%d"), *Pair.Key.ToString(), Pair.Value);
		}
		UE_LOG(LogSurvival, Log,
			TEXT("survival_populate_world: harita HENUZ KAYDEDILMEDI — save_assets cagir"));
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdPopulateWorld(
		TEXT("survival_populate_world"),
		TEXT("Genis dunyaya seyrek hasat dugumu dagitir (baslangic vadisi haric): survival_populate_world [Tohum=1337] [Yogunluk=25000]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(&PopulateWorld));
}

#endif // WITH_EDITOR
