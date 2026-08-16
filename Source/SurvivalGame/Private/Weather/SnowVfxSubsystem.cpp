#include "Weather/SnowVfxSubsystem.h"
#include "Weather/WeatherSimulation.h"
#include "Weather/WeatherTypes.h"
#include "SurvivalGame.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

namespace
{
	// Kutu boyutlari ve dusme parametreleri (kamera-goreli). Yogunluk: taneler kameranin
	// HEMEN cevresinde SIK olmali yoksa gorunmez (500 tane 3 km kutuda metrede-bir = "yagis yok").
	constexpr int32 FlakeCount = 3000;     // ince + yogun kar
	constexpr double BoxHalfXY = 800.0;    // yatay yari-genislik (UU) — kameraya siki
	constexpr double SpawnTopMin = 300.0;  // kameranin ustunde dogum araligi
	constexpr double SpawnTopMax = 1000.0;
	constexpr double KillBelow = 350.0;    // kameranin bu kadar altina dusunce yeniden dogar
	// Yavas + varyasyonlu dusus (gercek kar tekduze/hizli degil, savrularak yumusak duser).
	constexpr double FallSpeedMin = 130.0; // UU/sn
	constexpr double FallSpeedMax = 210.0;
	constexpr double DriftX = 22.0;        // hafif ruzgar suruklemesi (UU/sn)
	constexpr double DriftY = 14.0;
	constexpr double SwayFreq = 1.6;       // yatay salinim frekansi (rad/sn)
	constexpr double SwayAmp = 38.0;       // salinim genligi (UU/sn) — savrulma hissi
	// Boyut karisimi: cogu ince, azi biraz iri (dogal kar dagilimi).
	constexpr float FlakeScaleMin = 0.018f; // ~1.8 UU
	constexpr float FlakeScaleMax = 0.05f;  // ~5 UU

	FVector RandomFlakeAround(const FVector& Cam)
	{
		return Cam + FVector(
			FMath::FRandRange(-BoxHalfXY, BoxHalfXY),
			FMath::FRandRange(-BoxHalfXY, BoxHalfXY),
			FMath::FRandRange(SpawnTopMin, SpawnTopMax));
	}
}

void USnowVfxSubsystem::EnsureInitialized()
{
	if (bInitialized)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Kar tanelerini tutan gorunmez host aktoru.
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;   // kaydedilmez (saf VFX)
	SnowHost = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	if (!SnowHost)
	{
		UE_LOG(LogSurvivalWeather, Warning, TEXT("SnowVfx: host aktor spawn edilemedi"));
		return;
	}
#if WITH_EDITOR
	SnowHost->SetActorLabel(TEXT("SnowVfxHost"));
#endif

	// Kure kullan (kup DEGIL) — kupler her aciden KARE gorunuyordu ("kareler yagiyor"); kure her
	// aciden yuvarlak, kar tanesine cok daha yakin.
	UStaticMesh* FlakeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!FlakeMesh)
	{
		UE_LOG(LogSurvivalWeather, Warning, TEXT("SnowVfx: engine kuresi yuklenemedi"));
		return;
	}

	SnowISM = NewObject<UInstancedStaticMeshComponent>(SnowHost);
	SnowISM->SetStaticMesh(FlakeMesh);
	SnowISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SnowISM->SetCastShadow(false);
	SnowISM->SetMobility(EComponentMobility::Movable);
	if (BaseMat)
	{
		// BasicShapeMaterial'in "Color" vektor parametresini beyaza cek (MCP'siz beyaz tane).
		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, SnowHost);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
			SnowISM->SetMaterial(0, MID);
		}
	}
	SnowHost->SetRootComponent(SnowISM);
	SnowISM->RegisterComponent();

	FlakePositions.Reserve(FlakeCount);
	bInitialized = true;
	UE_LOG(LogSurvivalWeather, Log, TEXT("SnowVfx: hazir (%d tane havuzu, C++ partikul)"), FlakeCount);
}

void USnowVfxSubsystem::SetSnowingActive(bool bNewActive, const FVector& CameraLocation)
{
	if (!SnowISM)
	{
		return;
	}

	if (bNewActive)
	{
		FlakePositions.Reset();
		FlakeSpeeds.Reset();
		FlakeScales.Reset();
		FlakePhases.Reset();
		SnowISM->ClearInstances();
		for (int32 i = 0; i < FlakeCount; ++i)
		{
			const FVector P = RandomFlakeAround(CameraLocation);
			// FRand^2 -> kucuge egilimli dagilim (cogu ince tane, azi biraz iri).
			const float R = FMath::FRand();
			const float FlakeScale = FlakeScaleMin + (FlakeScaleMax - FlakeScaleMin) * R * R;
			FlakePositions.Add(P);
			FlakeSpeeds.Add(FMath::FRandRange(static_cast<float>(FallSpeedMin), static_cast<float>(FallSpeedMax)));
			FlakeScales.Add(FlakeScale);
			FlakePhases.Add(FMath::FRandRange(0.0f, 2.0f * PI));
			SnowISM->AddInstance(FTransform(FRotator::ZeroRotator, P, FVector(FlakeScale)), /*bWorldSpace*/ true);
		}
	}
	else
	{
		FlakePositions.Reset();
		FlakeSpeeds.Reset();
		FlakeScales.Reset();
		FlakePhases.Reset();
		SnowISM->ClearInstances();
	}
	bActive = bNewActive;
}

void USnowVfxSubsystem::Tick(float DeltaTime)
{
	EnsureInitialized();
	if (!bInitialized || !SnowISM)
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (!GI)
	{
		return;
	}

	// Hava durumu — yalnizca kar yagisi / kar firtinasinda aktif.
	bool bShouldSnow = false;
	if (UWeatherSimulation* Weather = GI->GetSubsystem<UWeatherSimulation>())
	{
		const EWeatherCondition Condition = Weather->GetCurrentState().Condition;
		bShouldSnow = (Condition == EWeatherCondition::Snowing || Condition == EWeatherCondition::Blizzard);
	}

	// Kamera konumu (kutu bunu takip eder). Kamera gecersizse (henuz kurulmadi -> orijin)
	// oyuncu pawn'ina dus — yoksa taneler orijinde dogar, oyuncu vadinin dibinde (~ -7000 Z)
	// kalir ve kari HIC gormez.
	FVector CameraLocation = FVector::ZeroVector;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(World, 0))
	{
		CameraLocation = CamMgr->GetCameraLocation();
	}
	if (CameraLocation.IsNearlyZero())
	{
		if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			CameraLocation = PlayerPawn->GetActorLocation() + FVector(0.0, 0.0, 150.0);
		}
	}

	if (bShouldSnow != bActive)
	{
		SetSnowingActive(bShouldSnow, CameraLocation);
	}
	if (!bActive)
	{
		return;
	}

	// Salinim icin zaman birikimi (Date::Now yok — resume-guvenli, Tick'te DeltaTime toplanir).
	ElapsedTime += DeltaTime;
	const int32 Num = FlakePositions.Num();
	for (int32 i = 0; i < Num; ++i)
	{
		FVector& P = FlakePositions[i];
		P.Z -= FlakeSpeeds[i] * DeltaTime;
		// Yatay salinim: her tane kendi faziyla saga-sola savrulur (duz cizgi degil, floaty kar hissi).
		const double Phase = ElapsedTime * SwayFreq + FlakePhases[i];
		P.X += (DriftX + FMath::Sin(Phase) * SwayAmp) * DeltaTime;
		P.Y += (DriftY + FMath::Cos(Phase) * SwayAmp) * DeltaTime;

		// Kutudan ciktiysa (asagi dustu ya da kameradan yatayda uzaklasti) tepeden yeniden dogur.
		const double DX = P.X - CameraLocation.X;
		const double DY = P.Y - CameraLocation.Y;
		if (P.Z < CameraLocation.Z - KillBelow ||
			FMath::Abs(DX) > BoxHalfXY * 1.4 || FMath::Abs(DY) > BoxHalfXY * 1.4)
		{
			P = RandomFlakeAround(CameraLocation);
			// Yeniden dogunca boyut/hiz/faz tazele — havuz zamanla tekduzeye kaymasin.
			const float R = FMath::FRand();
			FlakeScales[i] = FlakeScaleMin + (FlakeScaleMax - FlakeScaleMin) * R * R;
			FlakeSpeeds[i] = FMath::FRandRange(static_cast<float>(FallSpeedMin), static_cast<float>(FallSpeedMax));
			FlakePhases[i] = FMath::FRandRange(0.0f, 2.0f * PI);
		}

		// Render durumunu yalnizca son tanede bir kez isaretle (kare basi tek render guncellemesi).
		SnowISM->UpdateInstanceTransform(i, FTransform(FRotator::ZeroRotator, P, FVector(FlakeScales[i])),
			/*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ (i == Num - 1), /*bTeleport*/ true);
	}
}
