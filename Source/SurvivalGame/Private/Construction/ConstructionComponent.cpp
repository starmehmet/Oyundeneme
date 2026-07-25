#include "Construction/ConstructionComponent.h"
#include "Construction/ConstructionGhost.h"
#include "Construction/FoundationGrid.h"
#include "Construction/BuildingDatabase.h"
#include "Construction/BuildingBase.h"
#include "Inventory/InventoryComponent.h"
#include "Weather/SnowAccumulation.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	UFoundationGrid* GetGrid(UWorld* World)
	{
		return World ? World->GetSubsystem<UFoundationGrid>() : nullptr;
	}

	UBuildingDatabase* GetBuildingDatabase(UWorld* World)
	{
		return (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<UBuildingDatabase>() : nullptr;
	}

	USnowAccumulation* GetSnowSim(UWorld* World)
	{
		return (World && World->GetGameInstance()) ? World->GetGameInstance()->GetSubsystem<USnowAccumulation>() : nullptr;
	}
}

UConstructionComponent::UConstructionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UConstructionComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerInventory = GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!OwnerInventory)
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("ConstructionComponent: sahibinde UInventoryComponent yok — insaat calismayacak"));
	}
}

bool UConstructionComponent::EvaluatePlacementValidity(FName BuildingID, const FIntPoint& GridCoord) const
{
	UWorld* World = GetWorld();
	const UFoundationGrid* Grid = GetGrid(World);
	if (!Grid || Grid->IsCellOccupied(GridCoord))
	{
		return false;
	}

	// Sistem #14: asiri kar insaati engeller (tek-nokta kontrol — hem ghost onizlemesi hem
	// ConfirmPlacement AYNI EvaluatePlacementValidity'den gecer, grid-doluluk kontroluyle ayni
	// yerde).
	const USnowAccumulation* SnowSim = GetSnowSim(World);
	if (SnowSim && SnowSim->IsConstructionBlocked())
	{
		return false;
	}

	const UBuildingDatabase* DB = GetBuildingDatabase(World);
	FBuildingDefinition Def;
	if (!DB || !DB->FindBuilding(BuildingID, Def) || !OwnerInventory)
	{
		return false;
	}

	for (const TPair<FName, int32>& Req : Def.GetAggregatedRequirements())
	{
		if (!OwnerInventory->HasItem(Req.Key, Req.Value))
		{
			return false;
		}
	}
	return true;
}

bool UConstructionComponent::StartPlacement(FName BuildingID)
{
	if (BuildingID.IsNone())
	{
		return false;
	}

	UWorld* World = GetWorld();
	FBuildingDefinition Def;
	const UBuildingDatabase* DB = GetBuildingDatabase(World);
	if (!DB || !DB->FindBuilding(BuildingID, Def))
	{
		UE_LOG(LogSurvival, Warning, TEXT("StartPlacement: bilinmeyen BuildingID '%s'"), *BuildingID.ToString());
		return false;
	}

	CancelPlacement(); // onceki aktif yerlestirme varsa temizle

	SpawnedGhost = World ? World->SpawnActor<AConstructionGhost>() : nullptr;
	PendingBuildingID = BuildingID;
	return SpawnedGhost != nullptr;
}

void UConstructionComponent::UpdateGhostPosition(const FVector& WorldLocation)
{
	if (!SpawnedGhost)
	{
		return;
	}

	UWorld* World = GetWorld();
	UFoundationGrid* Grid = GetGrid(World);
	if (!Grid)
	{
		return;
	}

	const FIntPoint GridCoord = Grid->WorldToGridCoord(WorldLocation);
	const FVector SnappedLocation = Grid->GridCoordToWorld(GridCoord, WorldLocation.Z);
	const bool bValid = EvaluatePlacementValidity(PendingBuildingID, GridCoord);

	const UBuildingDatabase* DB = GetBuildingDatabase(World);
	FBuildingDefinition Def;
	UStaticMesh* PreviewMesh = (DB && DB->FindBuilding(PendingBuildingID, Def)) ? Def.Mesh.LoadSynchronous() : nullptr;

	SpawnedGhost->UpdatePreview(PreviewMesh, SnappedLocation, GridCoord, bValid);
}

bool UConstructionComponent::ConfirmPlacement()
{
	if (!SpawnedGhost)
	{
		return false;
	}

	const FIntPoint GridCoord = SpawnedGhost->GetGridCoord();
	const FName BuildingID = PendingBuildingID;
	const FVector SpawnLocation = SpawnedGhost->GetActorLocation();

	if (!EvaluatePlacementValidity(BuildingID, GridCoord))
	{
		UE_LOG(LogSurvival, Warning,
			TEXT("ConfirmPlacement: '%s' hucre (%d,%d) icin gecersiz (dolu/malzeme yetersiz) — ghost yerinde kaldi"),
			*BuildingID.ToString(), GridCoord.X, GridCoord.Y);
		return false;
	}

	UWorld* World = GetWorld();
	UBuildingDatabase* DB = GetBuildingDatabase(World);
	FBuildingDefinition Def;
	if (!DB || !DB->FindBuilding(BuildingID, Def))
	{
		// EvaluatePlacementValidity az once ayni DB'yi basariyla kullandi — bu dal normalde
		// erisilemez, ama diger tum GetBuildingDatabase() cagri noktalarinin hepsi ayni korumayi
		// yapiyor (inceleme bulgusu: burasi tutarsiz sekilde korumasizdi).
		CancelPlacement();
		return false;
	}

	// Atomik tuketim: StartCrafting'deki rollback deseniyle ayni (Sistem #6, inceleme bulgusu) —
	// herhangi bir kalem eksik cikarsa o ana kadar tuketilenler GERI EKLENIR.
	TArray<TPair<FName, int32>> Consumed;
	for (const TPair<FName, int32>& Req : Def.GetAggregatedRequirements())
	{
		const int32 Removed = OwnerInventory->RemoveItem(Req.Key, Req.Value);
		Consumed.Add(TPair<FName, int32>(Req.Key, Removed));
		if (Removed < Req.Value)
		{
			for (const TPair<FName, int32>& C : Consumed)
			{
				if (C.Value > 0)
				{
					OwnerInventory->AddItem(C.Key, C.Value);
				}
			}
			UE_LOG(LogSurvival, Warning,
				TEXT("ConfirmPlacement: '%s' icin malzeme tuketimi yarim kaldi — islem geri alindi"),
				*BuildingID.ToString());
			CancelPlacement();
			return false;
		}
	}

	UFoundationGrid* Grid = GetGrid(World);
	// Faz 1 entegrasyon borcu: Def.BuildingClass BOS ise (mevcut satirlarin TAMAMI) taban
	// ABuildingBase spawn edilir — geriye-uyumlu. Doluysa (ör. AProductionMachine) o alt sinif
	// spawn edilir; alt sinifin kendi BeginPlay'i (UProductionManager kaydi) ve BeginConstruction
	// override'i (AvailableRecipeIDs) KENDILIGINDEN calisir, burada ekstra kod GEREKMEZ.
	const TSubclassOf<ABuildingBase> SpawnClass = Def.BuildingClass ? Def.BuildingClass.Get() : ABuildingBase::StaticClass();
	ABuildingBase* NewBuilding = World->SpawnActor<ABuildingBase>(SpawnClass, SpawnLocation, FRotator::ZeroRotator);
	bool bPlaced = false;
	if (NewBuilding)
	{
		NewBuilding->BeginConstruction(BuildingID, Def, GridCoord);
		bPlaced = Grid && Grid->RegisterBuilding(NewBuilding, GridCoord);
		if (!bPlaced)
		{
			// Teorik olarak imkansiz (EvaluatePlacementValidity az once hucreyi bos buldu, ayni
			// senkron cagri icinde baska hicbir sey dolduramaz) — yine de savunmaci: en azindan
			// yetim aktoru yok et, sessizce sahnede birakma.
			UE_LOG(LogSurvival, Warning, TEXT("ConfirmPlacement: '%s' grid kaydi basarisiz — aktor yok edildi"),
				*BuildingID.ToString());
			NewBuilding->Destroy();
		}
	}

	if (!bPlaced)
	{
		// Bina yerlesmedi (spawn basarisiz VEYA grid kaydi basarisiz) — az once tuketilen
		// malzemeler GERI IADE edilir. Rollback yukarida yalnizca "malzeme yetersiz" dalinda
		// vardi, bu dal atlanmisti — oyuncu hicbir sey almadan malzeme kaybedebiliyordu
		// (inceleme bulgusu). CancelCrafting'deki "iade eksikse logla" deseniyle ayni.
		for (const TPair<FName, int32>& C : Consumed)
		{
			if (C.Value > 0)
			{
				const int32 Accepted = OwnerInventory->AddItem(C.Key, C.Value);
				if (Accepted < C.Value)
				{
					UE_LOG(LogSurvival, Warning,
						TEXT("ConfirmPlacement: '%s' basarisiz yerlesim sonrasi '%s' tam iade edilemedi (istenen %d, kabul %d)"),
						*BuildingID.ToString(), *C.Key.ToString(), C.Value, Accepted);
				}
			}
		}
	}

	UE_LOG(LogSurvival, Log, TEXT("Insaat: '%s' hucre (%d,%d) - %s"),
		*BuildingID.ToString(), GridCoord.X, GridCoord.Y, bPlaced ? TEXT("basarili") : TEXT("basarisiz"));

	CancelPlacement();
	return bPlaced;
}

void UConstructionComponent::CancelPlacement()
{
	if (SpawnedGhost)
	{
		SpawnedGhost->Destroy();
		SpawnedGhost = nullptr;
	}
	PendingBuildingID = NAME_None;
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi, craft_start ile ayni desen) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdBuildPlace(
		TEXT("build_place"),
		TEXT("Bina yerlestir (oyuncunun onune): build_place <BuildingID>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || Args.Num() < 1)
				{
					UE_LOG(LogSurvival, Warning, TEXT("Kullanim: build_place <BuildingID>"));
					return;
				}
				const APlayerController* PC = World->GetFirstPlayerController();
				APawn* Pawn = PC ? PC->GetPawn() : nullptr;
				UConstructionComponent* Construction = Pawn ? Pawn->FindComponentByClass<UConstructionComponent>() : nullptr;
				if (!Construction || !Pawn)
				{
					UE_LOG(LogSurvival, Warning, TEXT("build_place: ConstructionComponent bulunamadi"));
					return;
				}

				const FName BuildingID(*Args[0]);
				if (!Construction->StartPlacement(BuildingID))
				{
					UE_LOG(LogSurvival, Log, TEXT("build_place '%s': basarisiz (BuildingID gecersiz)"), *Args[0]);
					return;
				}

				const FVector PlaceLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 300.0f;
				Construction->UpdateGhostPosition(PlaceLocation);
				const bool bPlaced = Construction->ConfirmPlacement();
				UE_LOG(LogSurvival, Log, TEXT("build_place '%s': %s"),
					*Args[0], bPlaced ? TEXT("basarili") : TEXT("basarisiz (hucre dolu/malzeme yetersiz)"));
			}));

	// Yikim dev-araci: oyuncunun onundeki hucrede bina varsa yikar. E-etkilesimi ile YIKIM
	// KASITLI OLARAK bagli degil (kazayla bina yikma riski) — bkz. BuildingBase.h yorumu / ADR.
	FAutoConsoleCommandWithWorldAndArgs GCmdBuildDemolish(
		TEXT("build_demolish"),
		TEXT("Oyuncunun onundeki hucredeki binayi yikar: build_demolish"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}
				const APlayerController* PC = World->GetFirstPlayerController();
				const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
				const UFoundationGrid* Grid = GetGrid(World);
				if (!Pawn || !Grid)
				{
					UE_LOG(LogSurvival, Warning, TEXT("build_demolish: pawn/grid bulunamadi"));
					return;
				}

				const FVector TargetLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 300.0f;
				const FIntPoint GridCoord = Grid->WorldToGridCoord(TargetLocation);
				ABuildingBase* Target = Grid->GetBuildingAt(GridCoord);
				if (!Target)
				{
					UE_LOG(LogSurvival, Log, TEXT("build_demolish: hucre (%d,%d) bos"), GridCoord.X, GridCoord.Y);
					return;
				}

				const FName DemolishedID = Target->GetBuildingID();
				Target->Demolish();
				UE_LOG(LogSurvival, Log, TEXT("build_demolish: '%s' hucre (%d,%d) yikildi"),
					*DemolishedID.ToString(), GridCoord.X, GridCoord.Y);
			}));
}
