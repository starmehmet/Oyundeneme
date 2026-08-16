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
	constexpr int32 FlakeCount = 2200;     // kure kup'ten cok daha yuksek poligonlu — sayi biraz dusuk
	constexpr double BoxHalfXY = 750.0;    // yatay yari-genislik (UU) — kameraya siki
	constexpr double SpawnTopMin = 300.0;  // kameranin ustunde dogum araligi
	constexpr double SpawnTopMax = 950.0;
	constexpr double KillBelow = 350.0;    // kameranin bu kadar altina dusunce yeniden dogar
	constexpr double FallSpeed = 280.0;    // UU/sn
	constexpr double DriftX = 45.0;        // hafif ruzgar suruklemesi (UU/sn)
	constexpr double DriftY = 30.0;
	constexpr float FlakeScale = 0.05f;    // engine kuresi 100 UU -> ~5 UU yuvarlak tane

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
		SnowISM->ClearInstances();
		const FTransform Scale(FRotator::ZeroRotator, FVector::ZeroVector, FVector(FlakeScale));
		for (int32 i = 0; i < FlakeCount; ++i)
		{
			const FVector P = RandomFlakeAround(CameraLocation);
			FlakePositions.Add(P);
			FTransform Xform = Scale;
			Xform.SetLocation(P);
			SnowISM->AddInstance(Xform, /*bWorldSpace*/ true);
		}
	}
	else
	{
		FlakePositions.Reset();
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

	// Blizzard'da daha hizli/yogun dusme hissi: firtinada dusus ve suruklemeyi biraz artir.
	const double SpeedMul = 1.0;
	const FTransform ScaleOnly(FRotator::ZeroRotator, FVector::ZeroVector, FVector(FlakeScale));
	const int32 Num = FlakePositions.Num();
	for (int32 i = 0; i < Num; ++i)
	{
		FVector& P = FlakePositions[i];
		P.Z -= FallSpeed * SpeedMul * DeltaTime;
		P.X += DriftX * DeltaTime;
		P.Y += DriftY * DeltaTime;

		// Kutudan ciktiysa (asagi dustu ya da kameradan yatayda uzaklasti) tepeden yeniden dogur.
		const double DX = P.X - CameraLocation.X;
		const double DY = P.Y - CameraLocation.Y;
		if (P.Z < CameraLocation.Z - KillBelow ||
			FMath::Abs(DX) > BoxHalfXY * 1.4 || FMath::Abs(DY) > BoxHalfXY * 1.4)
		{
			P = RandomFlakeAround(CameraLocation);
		}

		FTransform Xform = ScaleOnly;
		Xform.SetLocation(P);
		// Render durumunu yalnizca son tanede bir kez isaretle (kare basi tek render guncellemesi).
		SnowISM->UpdateInstanceTransform(i, Xform, /*bWorldSpace*/ true,
			/*bMarkRenderStateDirty*/ (i == Num - 1), /*bTeleport*/ true);
	}
}
