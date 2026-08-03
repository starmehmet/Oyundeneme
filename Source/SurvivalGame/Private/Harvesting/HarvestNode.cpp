#include "Harvesting/HarvestNode.h"
#include "Harvesting/HarvestNodeDatabase.h"
#include "Harvesting/HarvestNodeManager.h"
#include "Harvesting/HarvestMath.h"
#include "Player/PlayerCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

AHarvestNode::AHarvestNode()
{
	InteractionPrompt = FText::FromString(TEXT("Topla"));
}

void AHarvestNode::BeginPlay()
{
	Super::BeginPlay();

	const UHarvestNodeDatabase* Database = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHarvestNodeDatabase>() : nullptr;
	FHarvestNodeDefinition Definition;
	if (!Database || !Database->FindNodeDefinition(NodeID, Definition))
	{
		// Inceleme bulgusu (majör): bu dalda yalnizca bDepleted=true/RemainingHarvests=0 set edilip
		// gizleme/collision-kapama VE Manager kaydi atlanirsa, dugum GORUNUR+CARPISMALI ama
		// etkilesilemez kalir VE Manager'in yeniden-dogma kuyruguna hic girmedigi icin ASLA
		// kendini toparlayamaz (gecersiz NodeID duzelmeden Respawn() de ayni FindNodeDefinition'i
		// tekrar basarisiz kilar). Duzeltme: bu, gecerli bir tanimi olmayan, kalici olarak bozuk bir
		// dugum — normal tukenme yoluyla (OnInteract_Implementation) AYNI gorsel/collision durumuna
		// getirilir, Manager'a KAYDEDILMEZ (kayit yeniden-dogma suresi gerektirir, burada gecerli
		// bir RespawnSeconds yok).
		UE_LOG(LogSurvival, Warning,
			TEXT("AHarvestNode '%s': NodeID '%s' DT_HarvestNodes'ta bulunamadi — kalici olarak devre disi birakiliyor"),
			*GetName(), *NodeID.ToString());
		RemainingHarvests = 0;
		bDepleted = true;
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		return;
	}

	RemainingHarvests = FMath::Max(1, Definition.HarvestsBeforeDepletion);
	if (!Definition.InteractionPrompt.IsEmpty())
	{
		InteractionPrompt = Definition.InteractionPrompt;
	}
}

void AHarvestNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bDepleted)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHarvestNodeManager* Manager = World->GetSubsystem<UHarvestNodeManager>())
			{
				Manager->UnregisterDepletedNode(this);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool AHarvestNode::CanInteract_Implementation(APlayerCharacter* Interactor) const
{
	return Super::CanInteract_Implementation(Interactor) && !bDepleted;
}

void AHarvestNode::OnInteract_Implementation(APlayerCharacter* Interactor)
{
	if (!Interactor || bDepleted)
	{
		return;
	}

	UInventoryComponent* PlayerInventory = Interactor->GetInventoryComponent();
	if (!PlayerInventory)
	{
		return;
	}

	const UHarvestNodeDatabase* Database = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHarvestNodeDatabase>() : nullptr;
	FHarvestNodeDefinition Definition;
	if (!Database || !Database->FindNodeDefinition(NodeID, Definition))
	{
		return;
	}

	int32 NormalizedMin = 0;
	int32 NormalizedMax = 0;
	SurvivalHarvest::NormalizeYieldRange(Definition.YieldCountMin, Definition.YieldCountMax, NormalizedMin, NormalizedMax);
	const int32 RequestedYield = FMath::RandRange(NormalizedMin, NormalizedMax);

	// Inceleme bulgusu (majör): AddItem'i ON-KONTROLSUZ cagirip yalnizca donus degerine gore sarj
	// karari vermek, KISMI kabul durumunda (0 < Eklenen < Istenen) GERCEK, kalici bir envanter
	// kazanci birakirken sarjin hic dusmemesine izin veriyordu — envanterde kismi yer birakilarak
	// ayni dugumden sinirsiz toplama yapilabilen bir ciftlik hatasi. Duzeltme: ProductionMachine'in
	// "once HasRoomFor ile on-kontrol, sonra commit et" desenini burada da uygula.
	if (!PlayerInventory->HasRoomFor(Definition.YieldItemID, RequestedYield))
	{
		UE_LOG(LogSurvival, Verbose,
			TEXT("AHarvestNode '%s': envanterde yer yok (%s x%d) — dugum TUKETILMEDI"),
			*GetName(), *Definition.YieldItemID.ToString(), RequestedYield);
		return;
	}

	const int32 ActuallyAdded = PlayerInventory->AddItem(Definition.YieldItemID, RequestedYield);
	if (ActuallyAdded < RequestedYield)
	{
		// HasRoomFor yalnizca AGIRLIGA bakar (bkz. InventoryComponent.h) — nadir bir kenar durumda
		// ("agirlik uygun ama slotlar dolu") yine de kismi kabul olabilir; bu artik risk projenin
		// diger cagiranlarinda (StartCrafting/ConfirmPlacement/RequestTransport) da kabul edilmis.
		// Bu durumda da dugum TUKETILMEZ — malzeme sessizce kaybolmaz.
		UE_LOG(LogSurvival, Verbose,
			TEXT("AHarvestNode '%s': envanterde tam yer yok (%d/%d %s eklendi) — dugum TUKETILMEDI"),
			*GetName(), ActuallyAdded, RequestedYield, *Definition.YieldItemID.ToString());
		return;
	}

	--RemainingHarvests;
	UE_LOG(LogSurvival, Log, TEXT("AHarvestNode '%s': %d adet %s toplandi (kalan sarj: %d)"),
		*GetName(), ActuallyAdded, *Definition.YieldItemID.ToString(), RemainingHarvests);

	if (RemainingHarvests <= 0)
	{
		bDepleted = true;
		CachedRespawnSeconds = Definition.RespawnSeconds;
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		if (UWorld* World = GetWorld())
		{
			if (UHarvestNodeManager* Manager = World->GetSubsystem<UHarvestNodeManager>())
			{
				DepletionGameTime = Manager->GetGameTime();
				Manager->RegisterDepletedNode(this);
			}
		}
	}
}

void AHarvestNode::Respawn()
{
	const UHarvestNodeDatabase* Database = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHarvestNodeDatabase>() : nullptr;
	FHarvestNodeDefinition Definition;
	RemainingHarvests = (Database && Database->FindNodeDefinition(NodeID, Definition))
		? FMath::Max(1, Definition.HarvestsBeforeDepletion)
		: 1;

	bDepleted = false;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	UE_LOG(LogSurvival, Log, TEXT("AHarvestNode '%s': yeniden dogdu (sarj: %d)"), *GetName(), RemainingHarvests);
}

void AHarvestNode::RestoreStateForLoad(int32 InHarvests, bool bInDepleted, double InDepletionTime)
{
	RemainingHarvests = InHarvests;
	bDepleted = bInDepleted;
	DepletionGameTime = InDepletionTime;

	SetActorHiddenInGame(bDepleted);
	SetActorEnableCollision(!bDepleted);

	if (bDepleted)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHarvestNodeManager* Manager = World->GetSubsystem<UHarvestNodeManager>())
			{
				Manager->RegisterDepletedNode(this);
			}
		}
	}
}
