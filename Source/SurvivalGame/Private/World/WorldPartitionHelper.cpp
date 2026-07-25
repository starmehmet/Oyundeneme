#include "World/WorldPartitionHelper.h"
#include "World/WorldPartitionMath.h"
#include "World/SurvivalWorldPartitionSettings.h"
#include "Construction/BuildingBase.h"
#include "NPC/NPCCharacter.h"
#include "Logistics/StorageNode.h"
#include "SurvivalGame.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "DevTools/ScopedTimer.h"

void UWorldPartitionHelper::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvival, Log, TEXT("WorldPartitionHelper hazir"));
}

bool UWorldPartitionHelper::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UWorld* World = GetWorld();
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* UWorldPartitionHelper::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

AActor* UWorldPartitionHelper::FindPlayerPawn() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	return PC ? PC->GetPawn() : nullptr;
}

void UWorldPartitionHelper::Tick(float DeltaTime)
{
	const USurvivalWorldPartitionSettings* Settings = GetDefault<USurvivalWorldPartitionSettings>();
	TimeSinceLastEvaluation += DeltaTime;
	if (TimeSinceLastEvaluation < Settings->EvaluationInterval)
	{
		return;
	}
	TimeSinceLastEvaluation = 0.0f;

	RebuildCellRegistry();
	EvaluateCellLoading();
}

void UWorldPartitionHelper::RebuildCellRegistry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Sistem #25 (Performans profiling) — bu fonksiyon daha once (profile_spawn_stress
	// incelemesinde) "kare-bolumlemesiz periyodik tarama, olculmedi" olarak dokumante edilmisti;
	// ilk gercek olcum burada. Esik (5ms) EvaluationInterval=1sn'de bir calistigi icin tek
	// seferlik bir gecikme olarak bile fark edilebilir olur diye secildi.
	SURVIVAL_SCOPED_TIMER_WARN("WorldPartitionHelper::RebuildCellRegistry", 5.0f);

	const USurvivalWorldPartitionSettings* Settings = GetDefault<USurvivalWorldPartitionSettings>();

	// Yeniden taramalar arasinda YUKLU/BOSALTILMIS durumunu KORUR (aksi halde her tarama
	// tum hucreleri "yuklu" varsayimina sifirlar, bosaltilmis bir hucre bir sonraki
	// EvaluateCellLoading'e kadar yanlislikla "yuklu" gorunurdu).
	TMap<FIntPoint, FWorldPartitionCell> NewCells;

	// Inceleme bulgusu: gizle/collision degistirme eskiden YALNIZCA SetCellLoaded'da
	// (hucre-seviyesi GECIS aninda) uygulaniyordu — zaten-bosaltilmis bir hucreye YENI bir
	// aktor eklenirse (insa edildi/dogdu) VEYA bir aktor (ozellikle bir NPC, kendi
	// AIController'i AdvanceBrain'den BAGIMSIZ hareket ettirmeye devam eder) BASKA bir
	// hucreye TASINIRSA, o aktor icin hicbir GECIS tetiklenmedigi icin gorunurluk/collision
	// HICBIR ZAMAN dogru degere getirilmezdi. Duzeltme: her tarama, KENDI hucresinin
	// (yeni VEYA tasinmis, degismemis de olsa) GUNCEL bIsLoaded durumunu HER aktore
	// KOSULSUZ uygular — boylece yeni/tasinmis aktorler bir sonraki degerlendirmede
	// KENDILIGINDEN dogru senkronize olur.
	auto RegisterActor = [&NewCells, this, Settings](AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return;
		}
		const FIntPoint CellCoord = SurvivalWorldPartition::GetCellForPosition(Actor->GetActorLocation(), Settings->CellSize);
		FWorldPartitionCell& Cell = NewCells.FindOrAdd(CellCoord);
		if (const FWorldPartitionCell* OldCell = Cells.Find(CellCoord))
		{
			Cell.bIsLoaded = OldCell->bIsLoaded;
		}
		Cell.ContainedActors.Add(Actor);

		Actor->SetActorHiddenInGame(!Cell.bIsLoaded);
		Actor->SetActorEnableCollision(Cell.bIsLoaded);
	};

	// Uc TActorIterator taramasi — Insaat/Uretim/Ruzgar (ABuildingBase ve alt siniflari),
	// NPC'ler, Lojistik dugumleri. Bu, BES sistemin (7/9/13/15/8) HICBIRINE dokunmadan
	// (kendi aktor siniflarina kayit-kodu EKLEMEDEN) kapsanmasini saglar.
	for (TActorIterator<ABuildingBase> It(World); It; ++It)
	{
		RegisterActor(*It);
	}
	for (TActorIterator<ANPCCharacter> It(World); It; ++It)
	{
		RegisterActor(*It);
	}
	for (TActorIterator<AStorageNode> It(World); It; ++It)
	{
		RegisterActor(*It);
	}

	Cells = MoveTemp(NewCells);
}

void UWorldPartitionHelper::EvaluateCellLoading()
{
	const USurvivalWorldPartitionSettings* Settings = GetDefault<USurvivalWorldPartitionSettings>();
	const AActor* PlayerPawn = FindPlayerPawn();
	if (!PlayerPawn)
	{
		return;
	}
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();

	// Inceleme bulgusu: histerezis sozlesmesi (UnloadRadius > LoadRadius) yalnizca yorumda
	// belgeleniyordu, hicbir yerde ZORLANMIYORDU — yanlis yapilandirilirsa (UnloadRadius<=
	// LoadRadius) sinirdaki hucreler HER degerlendirmede yuklenip-bosalir (titreme). Burada
	// GUVENLI bir taban ile kelepceleniyor.
	const float SafeUnloadRadius = FMath::Max(Settings->UnloadRadius, Settings->LoadRadius + 1.0f);

	for (TPair<FIntPoint, FWorldPartitionCell>& Pair : Cells)
	{
		const FVector CellCenter = SurvivalWorldPartition::GetCellCenter(Pair.Key, Settings->CellSize);
		const float Distance = FVector::Dist(PlayerLocation, CellCenter);

		if (!Pair.Value.bIsLoaded && SurvivalWorldPartition::ShouldCellBeLoaded(Distance, Settings->LoadRadius))
		{
			SetCellLoaded(Pair.Key, Pair.Value, true);
		}
		else if (Pair.Value.bIsLoaded && SurvivalWorldPartition::ShouldCellBeUnloaded(Distance, SafeUnloadRadius))
		{
			SetCellLoaded(Pair.Key, Pair.Value, false);
		}
	}
}

void UWorldPartitionHelper::SetCellLoaded(const FIntPoint& CellCoord, FWorldPartitionCell& Cell, bool bLoaded)
{
	Cell.bIsLoaded = bLoaded;
	for (auto It = Cell.ContainedActors.CreateIterator(); It; ++It)
	{
		AActor* Actor = It->Get();
		if (!IsValid(Actor))
		{
			It.RemoveCurrent();
			continue;
		}
		// YOK ETME/yeniden-DOGURMA yok — yalnizca gizle+collision kapat (bkz. WorldPartitionHelper.h
		// ADR notu: "hucre durumunu koru" boylece otomatik saglanir, bellek ayrimi degismez).
		Actor->SetActorHiddenInGame(!bLoaded);
		Actor->SetActorEnableCollision(bLoaded);
	}

	OnCellLoadStateChanged.Broadcast(CellCoord, bLoaded);
	UE_LOG(LogSurvival, Log, TEXT("WorldPartition hucre (%d,%d): %s"), CellCoord.X, CellCoord.Y, bLoaded ? TEXT("yuklendi") : TEXT("bosaltildi"));
}

bool UWorldPartitionHelper::IsPositionLoaded(const FVector& Position) const
{
	const USurvivalWorldPartitionSettings* Settings = GetDefault<USurvivalWorldPartitionSettings>();
	const FIntPoint CellCoord = SurvivalWorldPartition::GetCellForPosition(Position, Settings->CellSize);
	const FWorldPartitionCell* Cell = Cells.Find(CellCoord);
	return Cell ? Cell->bIsLoaded : true; // bilinmeyen/henuz-taranmamis hucre -> guvenli varsayilan
}

int32 UWorldPartitionHelper::GetLoadedCellCount() const
{
	int32 Count = 0;
	for (const TPair<FIntPoint, FWorldPartitionCell>& Pair : Cells)
	{
		if (Pair.Value.bIsLoaded)
		{
			++Count;
		}
	}
	return Count;
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi ile ayni gerekce) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdWorldPartitionDump(
		TEXT("worldpartition_dump"),
		TEXT("Dunya bolumlendirme durumunu logla"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UWorldPartitionHelper* Helper = World ? World->GetSubsystem<UWorldPartitionHelper>() : nullptr;
				if (!Helper)
				{
					return;
				}
				const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
				const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
				const FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

				UE_LOG(LogSurvival, Log,
					TEXT("WorldPartition: toplam-hucre=%d yuklu-hucre=%d oyuncu-hucresi-yuklu=%s"),
					Helper->GetTotalCellCount(), Helper->GetLoadedCellCount(),
					Helper->IsPositionLoaded(PlayerLocation) ? TEXT("EVET") : TEXT("hayir"));
			}));
}
