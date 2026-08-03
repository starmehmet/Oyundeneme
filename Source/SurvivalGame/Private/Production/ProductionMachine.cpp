#include "Production/ProductionMachine.h"
#include "Production/ProductionMath.h"
#include "Production/ProductionRecipe.h"
#include "Production/ProductionRecipeDatabase.h"
#include "Production/ProductionManager.h"
#include "Inventory/InventoryComponent.h"
#include "Logistics/StorageNode.h"
#include "Logistics/StorageNodeType.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"

AProductionMachine::AProductionMachine()
{
	PrimaryActorTick.bCanEverTick = false; // UProductionManager kare-bolumlemeli tikler (AdvanceProduction)

	InputBuffer = CreateDefaultSubobject<UInventoryComponent>(TEXT("InputBuffer"));
	OutputBuffer = CreateDefaultSubobject<UInventoryComponent>(TEXT("OutputBuffer"));
}

void AProductionMachine::BeginPlay()
{
	Super::BeginPlay();

	CurrentEnergy = MaxEnergy;

	if (InputBuffer)
	{
		for (const TPair<FName, int32>& Pair : StartingInputItems)
		{
			InputBuffer->AddItem(Pair.Key, Pair.Value);
		}
	}

	UWorld* World = GetWorld();
	if (World)
	{
		if (UProductionManager* Manager = World->GetSubsystem<UProductionManager>())
		{
			Manager->RegisterMachine(this);
		}
	}

	// Faz 1 entegrasyon borcu: InputBuffer/OutputBuffer'i Sistem #8'in lojistik agina KAYIT
	// EDEN iki hafif AStorageNode proxy'si — kendi envanterlerini yaratmazlar,
	// BindExternalStorage ile GERCEK arabellege baglanirlar (tek veri kopyasi, cift-sayim
	// riski yok). MachineInput/MachineOutput enum degerleri Sistem #8'de bastan bu amac icin
	// tanimlanmisti (bkz. StorageNodeType.h yorumu) — burada ILK KEZ gercekten kullaniliyor.
	// AWindTurbine gibi InputBuffer/OutputBuffer'i hic KULLANMAYAN alt siniflar
	// ShouldRegisterLogisticsNodes()'u false donerek bunu atlar (bos arabellekler agi kirletmesin).
	//
	// Inceleme bulgusu (motor kaynagindan dogrulandi): SIRADAN SpawnActor, dunya BeginPlay
	// yapmis oldugunda AStorageNode::BeginPlay'i SENKRON tetikler — Network->RegisterNode->
	// RebuildAcceptorCache o anda NodeType'in HALA varsayilan Container oldugunu gorurdu
	// (SetNodeType SpawnActor'dan SONRA cagrilir, cache'i yeniden kurmaz). MachineOutput icin
	// bu, CanNodeTypeAccept farkli (Container=true, MachineOutput=false) oldugundan cikti
	// arabellegini kalici olarak "alici" diye yanlis onbelleklerdi (su an yalnizca
	// CanAcceptItem'in ayrica CANLI kontrolu sayesinde maskeleniyordu — kirilgan, tasarlanmamis
	// bir tesadüf). Duzeltme: SpawnActorDeferred+FinishSpawning ile NodeType, BeginPlay
	// CALISMADAN ONCE dogru degerine ayarlanir.
	if (World && ShouldRegisterLogisticsNodes())
	{
		const FTransform ProxyTransform(GetActorRotation(), GetActorLocation());

		InputLogisticsNode = World->SpawnActorDeferred<AStorageNode>(AStorageNode::StaticClass(), ProxyTransform, this);
		if (InputLogisticsNode)
		{
			InputLogisticsNode->BindExternalStorage(InputBuffer);
			InputLogisticsNode->SetNodeType(EStorageNodeType::MachineInput);
			InputLogisticsNode->FinishSpawning(ProxyTransform);
			InputLogisticsNode->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}

		OutputLogisticsNode = World->SpawnActorDeferred<AStorageNode>(AStorageNode::StaticClass(), ProxyTransform, this);
		if (OutputLogisticsNode)
		{
			OutputLogisticsNode->BindExternalStorage(OutputBuffer);
			OutputLogisticsNode->SetNodeType(EStorageNodeType::MachineOutput);
			OutputLogisticsNode->FinishSpawning(ProxyTransform);
			OutputLogisticsNode->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void AProductionMachine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UProductionManager* Manager = World->GetSubsystem<UProductionManager>())
		{
			Manager->UnregisterMachine(this);
		}
	}

	if (InputLogisticsNode)
	{
		InputLogisticsNode->Destroy();
		InputLogisticsNode = nullptr;
	}
	if (OutputLogisticsNode)
	{
		OutputLogisticsNode->Destroy();
		OutputLogisticsNode = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AProductionMachine::BeginConstruction(FName InBuildingID, const FBuildingDefinition& InDefinition, const FIntPoint& InGridCoord)
{
	Super::BeginConstruction(InBuildingID, InDefinition, InGridCoord);

	// Faz 1 entegrasyon borcu: insaat sistemi (Sistem #7) hangi tariflerin desteklendigini
	// VERI olarak tasir — AvailableRecipeIDs'e koda gomulu bir liste YAZILMAZ, DataTable
	// (DT_Buildings) tek dogruluk kaynagi kalir. AddUnique: ayni RecipeID iki kez eklenmesin
	// (BP alt sinifinin CDO'sunda ZATEN varsa).
	for (const FName& RecipeID : InDefinition.ProductionRecipeIDs)
	{
		AvailableRecipeIDs.AddUnique(RecipeID);
	}
}

void AProductionMachine::AdvanceProduction(double CurrentGameTime)
{
	const float Elapsed = static_cast<float>(CurrentGameTime - LastProductionUpdateTime);
	LastProductionUpdateTime = CurrentGameTime;
	Tick_Production(FMath::Max(0.0f, Elapsed));
}

bool AProductionMachine::SetActiveRecipe(FName RecipeID)
{
	if (RecipeID.IsNone())
	{
		ActiveRecipeID = NAME_None;
		Progress = 0.0f;
		State = EProductionState::Idle;
		return true;
	}

	if (!AvailableRecipeIDs.Contains(RecipeID))
	{
		UE_LOG(LogSurvivalProduction, Warning, TEXT("SetActiveRecipe: '%s' bu makinede desteklenmiyor"), *RecipeID.ToString());
		return false;
	}

	const UGameInstance* GI = GetGameInstance();
	const UProductionRecipeDatabase* DB = GI ? GI->GetSubsystem<UProductionRecipeDatabase>() : nullptr;
	FProductionRecipe Recipe;
	if (!DB || !DB->FindRecipe(RecipeID, Recipe))
	{
		UE_LOG(LogSurvivalProduction, Warning, TEXT("SetActiveRecipe: '%s' veritabaninda bulunamadi"), *RecipeID.ToString());
		return false;
	}

	ActiveRecipeID = RecipeID;
	Progress = 0.0f;
	UE_LOG(LogSurvivalProduction, Log, TEXT("Uretim tarifi secildi: '%s'"), *RecipeID.ToString());
	return true;
}

float AProductionMachine::GetProgress() const
{
	if (ActiveRecipeID.IsNone())
	{
		return 0.0f;
	}
	const UGameInstance* GI = GetGameInstance();
	const UProductionRecipeDatabase* DB = GI ? GI->GetSubsystem<UProductionRecipeDatabase>() : nullptr;
	FProductionRecipe Recipe;
	if (!DB || !DB->FindRecipe(ActiveRecipeID, Recipe))
	{
		return 0.0f;
	}
	return SurvivalProduction::ComputeProductionProgress(Progress, Recipe.ProductionTime);
}

void AProductionMachine::Tick_Production(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}
	if (ActiveRecipeID.IsNone())
	{
		State = EProductionState::Idle;
		return;
	}

	const UGameInstance* GI = GetGameInstance();
	const UProductionRecipeDatabase* DB = GI ? GI->GetSubsystem<UProductionRecipeDatabase>() : nullptr;
	FProductionRecipe Recipe;
	if (!DB || !DB->FindRecipe(ActiveRecipeID, Recipe))
	{
		State = EProductionState::Idle;
		return;
	}

	// Kare-bolumleme yuzunden TEK bir AdvanceProduction cagrisi BIRDEN FAZLA uretim donguzunu
	// kapsayabilir (makine seyrek guncelleniyorsa VEYA ProductionTime kisaysa) — eskiden fazla
	// sure basitce ATILIYORDU (inceleme bulgusu: Progress=0'a sifirlanip donguye devam
	// edilmiyordu). Simdi kaynaklar TUKENENE veya sure BITENE kadar birden fazla tamamlanmis
	// dongu islenir; gercek zaman hicbir zaman "kaybolmaz" (bkz. ProductionManager.h).
	//
	// MaxCyclesPerAdvance: veri hatasi (orn. ProductionTime=0 + devasa DeltaTime) tek bir
	// cagriyi sonsuz donguye sokmasin diye savunmaci bir tavan (InventoryComponent::AddItem'daki
	// "ilerleme yoksa dur" deseniyle ayni ruh) — tavana carpilirsa kalan sure Progress'e
	// eklenip bir sonraki AdvanceProduction'a GERCEKTEN birakilir (atilmaz).
	static constexpr int32 MaxCyclesPerAdvance = 1000;
	float RemainingTime = DeltaTime;
	int32 CyclesThisAdvance = 0;

	for (;;)
	{
		const bool bHasInputs = HasSufficientInputs(Recipe);
		const bool bHasOutputRoom = HasSufficientOutputRoom(Recipe);
		const bool bHasFuel = Recipe.EnergyPerSecond <= 0.0f || CurrentEnergy > 0.0f;

		State = SurvivalProduction::DetermineBlockedState(bHasInputs, bHasOutputRoom, bHasFuel);
		if (State != EProductionState::Running)
		{
			break; // bloke — kalan RemainingTime bu donguyu ilerletmez (bloke sure ilerleme sayilmaz)
		}

		const float TimeToComplete = FMath::Max(0.0f, Recipe.ProductionTime - Progress);
		if (RemainingTime < TimeToComplete)
		{
			// Bu dilim bir donguyu TAMAMLAMAYA yetmiyor — ilerlemeyi ekle, enerji tuket, dur.
			CurrentEnergy = FMath::Max(0.0f, CurrentEnergy - SurvivalProduction::ComputeEnergyConsumed(Recipe.EnergyPerSecond, RemainingTime));
			Progress += RemainingTime;
			break;
		}

		// Bu dilim (en azindan) bir donguyu TAMAMLIYOR.
		CurrentEnergy = FMath::Max(0.0f, CurrentEnergy - SurvivalProduction::ComputeEnergyConsumed(Recipe.EnergyPerSecond, TimeToComplete));
		RemainingTime -= TimeToComplete;
		Progress = 0.0f;
		CompleteProductionCycle(Recipe);

		if (RemainingTime <= 0.0f)
		{
			break;
		}
		if (++CyclesThisAdvance >= MaxCyclesPerAdvance)
		{
			Progress += RemainingTime; // kalan sure ATILMAZ — bir sonraki AdvanceProduction'da devam eder
			UE_LOG(LogSurvivalProduction, Warning,
				TEXT("Tick_Production: '%s' tek guncellemede %d dongu tavanina ulasti (ProductionTime cok kucuk olabilir) — kalan %.4fsn sonraki guncellemeye birakildi"),
				*ActiveRecipeID.ToString(), MaxCyclesPerAdvance, RemainingTime);
			break;
		}
	}
}

bool AProductionMachine::HasSufficientInputs(const FProductionRecipe& Recipe) const
{
	if (!InputBuffer)
	{
		return false;
	}
	for (const TPair<FName, int32>& Req : Recipe.GetAggregatedInputs())
	{
		if (!InputBuffer->HasItem(Req.Key, Req.Value))
		{
			return false;
		}
	}
	return true;
}

bool AProductionMachine::HasSufficientOutputRoom(const FProductionRecipe& Recipe) const
{
	if (!OutputBuffer)
	{
		return false;
	}
	// TUM ciktilarin TOPLAM agirligini TEK SEFERDE kontrol eder — satir satir bagimsiz
	// HasRoomFor cagirmak her satiri AYNI (henuz-eklenmemis) CurrentWeight'e karsi olcerdi;
	// 2+ satirli bir ciktida her satir TEK BASINA sigar gorunup toplam agirlik asilabilirdi
	// (inceleme bulgusu). GetAggregatedOutputs ayrica tekrarlanan ItemID'leri de toplar.
	return OutputBuffer->HasRoomForBatch(Recipe.GetAggregatedOutputs());
}

void AProductionMachine::CompleteProductionCycle(const FProductionRecipe& Recipe)
{
	// Atomik degil (StartCrafting'in aksine) — buraya HasSufficientInputs zaten dogruladiktan
	// SONRA gelinir (Tick_Production), yani "yarim tuketim" senaryosu StartCrafting'deki gibi
	// oyuncu-tetikledigi anlik bir cagriya karsi degil, kendi kontrolunden gecmis bir dongu
	// sonucudur — yine de RemoveItem'in donusu KONTROL EDILIR (savunmaci, ayni sinif hatanin
	// tekrarini onlemek icin, bkz. Sistem #6/#7/#8 inceleme bulgulari).
	for (const TPair<FName, int32>& In : Recipe.GetAggregatedInputs())
	{
		const int32 Removed = InputBuffer->RemoveItem(In.Key, In.Value);
		if (Removed < In.Value)
		{
			UE_LOG(LogSurvivalProduction, Warning,
				TEXT("Uretim dongusu: '%s' icin '%s' tam tuketilemedi (istenen %d, cikan %d)"),
				*ActiveRecipeID.ToString(), *In.Key.ToString(), In.Value, Removed);
		}
	}

	for (const FRecipeOutput& Out : Recipe.Outputs)
	{
		const int32 Accepted = OutputBuffer->AddItem(Out.ItemID, Out.Count);
		if (Accepted < Out.Count)
		{
			UE_LOG(LogSurvivalProduction, Warning,
				TEXT("Uretim dongusu: '%s' icin '%s' tam teslim edilemedi (uretilen %d, kabul %d) — kalani KAYIP"),
				*ActiveRecipeID.ToString(), *Out.ItemID.ToString(), Out.Count, Accepted);
		}
	}

	OnProductionCompleted.Broadcast(ActiveRecipeID);
	UE_LOG(LogSurvivalProduction, Log, TEXT("Uretim tamamlandi: '%s'"), *ActiveRecipeID.ToString());
}

void AProductionMachine::RestoreStateForLoad(FName InRecipeID, float InProgress, float InEnergy,
	EProductionState InState, const TArray<FInventorySlot>& InInput, const TArray<FInventorySlot>& InOutput)
{
	ActiveRecipeID = InRecipeID;
	Progress = InProgress;
	CurrentEnergy = InEnergy;
	State = InState;
	if (InputBuffer)
	{
		InputBuffer->RestoreSlots(InInput);
	}
	if (OutputBuffer)
	{
		OutputBuffer->RestoreSlots(InOutput);
	}
}
