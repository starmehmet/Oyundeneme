#include "Logistics/LogisticsNetwork.h"
#include "Logistics/LogisticsMath.h"
#include "Logistics/LogisticsSettings.h"
#include "Logistics/StorageNode.h"
#include "Inventory/InventoryComponent.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"

void ULogisticsNetwork::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogSurvivalLogistics, Log, TEXT("LogisticsNetwork hazir"));
}

bool ULogisticsNetwork::IsTickable() const
{
	if (IsTemplate())
	{
		return false;
	}
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World && World->IsGameWorld() && World->HasBegunPlay();
}

UWorld* ULogisticsNetwork::GetTickableGameObjectWorld() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetWorld() : nullptr;
}

void ULogisticsNetwork::RegisterNode(AStorageNode* Node)
{
	if (!Node || Nodes.Contains(Node))
	{
		return;
	}
	Nodes.Add(Node);
	RebuildAcceptorCache();
	UE_LOG(LogSurvivalLogistics, Log, TEXT("Dugum kaydedildi (toplam %d)"), Nodes.Num());
}

void ULogisticsNetwork::UnregisterNode(AStorageNode* Node)
{
	if (Nodes.Remove(Node) > 0)
	{
		RebuildAcceptorCache();
		UE_LOG(LogSurvivalLogistics, Log, TEXT("Dugum kaydi silindi (toplam %d)"), Nodes.Num());
	}
}

void ULogisticsNetwork::RebuildAcceptorCache()
{
	AcceptorCache.Reset();
	for (const TObjectPtr<AStorageNode>& Node : Nodes)
	{
		if (Node && SurvivalLogistics::CanNodeTypeAccept(Node->GetNodeType()))
		{
			AcceptorCache.Add(Node);
		}
	}
}

AStorageNode* ULogisticsNetwork::FindBestDestination(FName ItemID, const FVector& FromLocation, const AStorageNode* ExcludeNode) const
{
	AStorageNode* Best = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();

	for (const TWeakObjectPtr<AStorageNode>& WeakNode : AcceptorCache)
	{
		AStorageNode* Node = WeakNode.Get();
		if (!Node || Node == ExcludeNode || !Node->CanAcceptItem(ItemID))
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(FromLocation, Node->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Node;
		}
	}
	return Best;
}

bool ULogisticsNetwork::RequestTransport(FName ItemID, int32 Count, AStorageNode* Source, AStorageNode* Destination)
{
	if (!Source || ItemID.IsNone() || Count <= 0)
	{
		return false;
	}

	if (!Destination)
	{
		Destination = FindBestDestination(ItemID, Source->GetActorLocation(), Source);
	}
	// Destination == Source: Source ACIKCA hedef olarak verilmisse hala reddedilir (kendine
	// tasima anlamsiz) — otomatik-cozum artik Source'u ExcludeNode ile HARIC TUTUYOR, bu yuzden
	// bu dal artik yalniz acik-hedef-Source durumunu yakalar (inceleme bulgusu once burayi
	// otomatik-cozumde de tetikliyordu, Source alici tipiyse rota HICBIR ZAMAN calismiyordu).
	// CanAcceptItem: acikca verilen bir Destination da (auto-cozumlenen gibi) alici TIPINDE
	// olmali — aksi halde ItemID sadece-uretici bir duguma sessizce teslim edilebilirdi.
	if (!Destination || Destination == Source || !Destination->CanAcceptItem(ItemID))
	{
		UE_LOG(LogSurvivalLogistics, Warning, TEXT("RequestTransport: '%s' icin hedef bulunamadi"), *ItemID.ToString());
		return false;
	}

	UInventoryComponent* SourceStorage = Source->GetStorage();
	if (!SourceStorage || !SourceStorage->HasItem(ItemID, Count))
	{
		return false; // yetersiz stok — hicbir sey tuketilmez
	}

	const int32 Removed = SourceStorage->RemoveItem(ItemID, Count);
	if (Removed < Count)
	{
		// Savunmaci: HasItem az once yeterli dedi, yine de kismi cikarsa geri ekle (StartCrafting'deki desen).
		if (Removed > 0)
		{
			SourceStorage->AddItem(ItemID, Removed);
		}
		UE_LOG(LogSurvivalLogistics, Warning,
			TEXT("RequestTransport: '%s' cikarma yarim kaldi (istenen %d, cikan %d) — islem geri alindi"),
			*ItemID.ToString(), Count, Removed);
		return false;
	}

	const ULogisticsSettings* Settings = GetDefault<ULogisticsSettings>();
	// FVector::Dist UE5 LWC'de double doner; ComputeTransportTime (diger tum saf math
	// fonksiyonlari gibi) float alir — bu oyunun mesafe olceginde (UU onlarca-bin) kayipsiz,
	// ama daralma ORTUK degil ACIKCA yapilmali (inceleme bulgusu).
	const float Distance = static_cast<float>(FVector::Dist(Source->GetActorLocation(), Destination->GetActorLocation()));
	const float TransportTime = SurvivalLogistics::ComputeTransportTime(Distance, Settings->TransportSpeed);

	FTransportRequest NewRequest;
	NewRequest.ItemID = ItemID;
	NewRequest.Count = Count;
	NewRequest.Source = Source;
	NewRequest.Destination = Destination;
	NewRequest.ElapsedTime = 0.0f;
	NewRequest.TransportTime = TransportTime;
	ActiveTransports.Add(NewRequest);

	UE_LOG(LogSurvivalLogistics, Log, TEXT("Tasima istegi: %dx '%s' (%.0f UU, %.1fsn) — kuyrukta %d"),
		Count, *ItemID.ToString(), Distance, TransportTime, ActiveTransports.Num());
	return true;
}

void ULogisticsNetwork::Tick(float DeltaTime)
{
	// Geriye dogru: CompleteTransport RemoveAt yapacagi icin ileri index kaymasini onler (CraftingComponent deseni).
	for (int32 i = ActiveTransports.Num() - 1; i >= 0; --i)
	{
		ActiveTransports[i].ElapsedTime += DeltaTime;
		if (SurvivalLogistics::IsTransportComplete(ActiveTransports[i].ElapsedTime, ActiveTransports[i].TransportTime))
		{
			CompleteTransport(i);
		}
	}

	if (bVisualizationEnabled)
	{
		DrawDebugVisualization(DeltaTime);
	}
}

void ULogisticsNetwork::DrawDebugVisualization(float DeltaTime) const
{
	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// Inceleme bulgusu (kritik, motor kaynagindan dogrulandi): LifeTime=0.0f "tek kare" ANLAMINA
	// GELMEZ — DrawDebugHelpers.cpp::GetDebugLineLifeTime, LifeTime<=0 oldugunda cizimi
	// ULineBatchComponent::DefaultLifeTime'a (1.0 saniye SABIT) dusurur, ve her cagri ONCEKINI
	// SILMEDEN yeni segmentler EKLER — 60 FPS'te ~60 ust uste binen kopya birikir (kargo kuresi
	// "iz" gibi gorunur). Dogru "bu tick'e ozel" davranisi icin LifeTime = DeltaTime verilir:
	// bir sonraki tick'in cizimi tam bu tick'in cizimi solmadan hemen once/o sirada devreye girer.
	const float VisualLifeTime = FMath::Max(DeltaTime, UE_KINDA_SMALL_NUMBER);

	for (const TObjectPtr<AStorageNode>& Node : Nodes)
	{
		if (!IsValid(Node))
		{
			continue;
		}

		FColor Color = FColor::White;
		FString TypeLabel = TEXT("?");
		switch (Node->GetNodeType())
		{
		case EStorageNodeType::Source:        Color = FColor::Green;   TypeLabel = TEXT("Source");        break;
		case EStorageNodeType::Sink:          Color = FColor::Red;     TypeLabel = TEXT("Sink");          break;
		case EStorageNodeType::Container:     Color = FColor::Yellow;  TypeLabel = TEXT("Container");     break;
		case EStorageNodeType::MachineInput:  Color = FColor::Cyan;    TypeLabel = TEXT("MachineInput");  break;
		case EStorageNodeType::MachineOutput: Color = FColor::Magenta; TypeLabel = TEXT("MachineOutput"); break;
		}

		const FVector Location = Node->GetActorLocation();
		DrawDebugSphere(World, Location, 30.0f, 12, Color, false, VisualLifeTime, 0, 2.0f);

		FString StockSummary = TEXT("bos");
		if (const UInventoryComponent* Storage = Node->GetStorage())
		{
			const TArray<FInventorySlot>& Slots = Storage->GetSlots();
			if (Slots.Num() > 0)
			{
				StockSummary = FString::Printf(TEXT("%s x%d"), *Slots[0].ItemID.ToString(), Slots[0].Count);
				if (Slots.Num() > 1)
				{
					StockSummary += FString::Printf(TEXT(" (+%d)"), Slots.Num() - 1);
				}
			}
		}

		// Inceleme bulgusu (majör, motor kaynagindan dogrulandi): TestBaseActor=nullptr verilirse
		// DrawDebugString tum etiketleri AYNI aktore (World->GetWorldSettings()) bagliyor —
		// AHUD::AddDebugText aktor-basina 128 etiketle sinirli (r.DebugSafeZone.
		// MaxDebugTextStringsPerActor), sinir asilinca fazlasi SESSIZCE cizilmez. Her duguma
		// KENDI aktorunu (Node) TestBaseActor olarak vermek her duguma kendi 128-etiket
		// butcesini ayirir, coguşmayi onler.
		DrawDebugString(World, Location + FVector(0.0, 0.0, 60.0), FString::Printf(TEXT("%s\n%s"), *TypeLabel, *StockSummary),
			Node, Color, VisualLifeTime, false);
	}

	for (const FTransportRequest& Transport : ActiveTransports)
	{
		AStorageNode* Source = Transport.Source.Get();
		const AStorageNode* Destination = Transport.Destination.Get();
		if (!IsValid(Source) || !IsValid(Destination))
		{
			continue; // dugum tasima surerken yikilmis olabilir (bkz. sinif yorumu) — bu tur sessizce atlanir
		}

		const FVector SourceLoc = Source->GetActorLocation();
		const FVector DestLoc = Destination->GetActorLocation();
		DrawDebugLine(World, SourceLoc, DestLoc, FColor::Orange, false, VisualLifeTime, 0, 1.5f);

		const float Progress = Transport.TransportTime > 0.0f
			? FMath::Clamp(Transport.ElapsedTime / Transport.TransportTime, 0.0f, 1.0f)
			: 0.0f;
		const FVector CargoLoc = FMath::Lerp(SourceLoc, DestLoc, Progress);
		DrawDebugSphere(World, CargoLoc, 15.0f, 8, FColor::Orange, false, VisualLifeTime, 0, 3.0f);
		// TestBaseActor = Source: taşımanın kaynak düğümünün kendi 128-etiket bütçesini kullanır
		// (Source zaten kendi node-etiketiyle bu bütçeyi paylaşıyor, aynı düğümden aynı anda
		// birkaç taşıma çıksa bile 128 sınırına ulaşmak gerçekçi değil).
		DrawDebugString(World, CargoLoc + FVector(0.0, 0.0, 30.0),
			FString::Printf(TEXT("%s x%d"), *Transport.ItemID.ToString(), Transport.Count), Source, FColor::Orange, VisualLifeTime, false);
	}
}

void ULogisticsNetwork::CompleteTransport(int32 Index)
{
	if (!ActiveTransports.IsValidIndex(Index))
	{
		return;
	}

	const FTransportRequest Request = ActiveTransports[Index]; // kopya — RemoveAt sonrasi referans gecersiz olmasin
	ActiveTransports.RemoveAt(Index);

	AStorageNode* Dest = Request.Destination.Get();
	if (Dest && Dest->GetStorage())
	{
		const int32 Accepted = Dest->GetStorage()->AddItem(Request.ItemID, Request.Count);
		if (Accepted < Request.Count)
		{
			// Hedef dolu/agirsa kalan miktar KAYIP olur, kaynaga iade EDILMEZ (bkz. ADR) —
			// gercek "geri bas/bekle" (belt backup) davranisi bu pasonun kapsaminda degil.
			UE_LOG(LogSurvivalLogistics, Warning,
				TEXT("Teslimat: '%s' icin hedef yalniz %d/%d kabul etti — kalani KAYIP"),
				*Request.ItemID.ToString(), Accepted, Request.Count);
		}
	}
	else
	{
		UE_LOG(LogSurvivalLogistics, Warning,
			TEXT("Teslimat: '%s' hedefi artik gecersiz (yok edilmis olabilir) — %d adet KAYIP"),
			*Request.ItemID.ToString(), Request.Count);
	}

	OnTransportDelivered.Broadcast(Request.ItemID);
	UE_LOG(LogSurvivalLogistics, Log, TEXT("Teslimat tamamlandi: %dx '%s'"), Request.Count, *Request.ItemID.ToString());
}

// ---- Konsol komutu: PIE/dev dogrulamasi icin (Sistem #22'nin one alinan kismi, craft_start ile ayni desen) ----

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdLogisticsRequest(
		TEXT("logistics_request"),
		TEXT("Ag uzerinden tasima iste: logistics_request <ItemID> <Adet>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || !World->GetGameInstance() || Args.Num() < 2)
				{
					UE_LOG(LogSurvivalLogistics, Warning, TEXT("Kullanim: logistics_request <ItemID> <Adet>"));
					return;
				}
				ULogisticsNetwork* Network = World->GetGameInstance()->GetSubsystem<ULogisticsNetwork>();
				if (!Network)
				{
					return;
				}

				const FName ItemID(*Args[0]);
				const int32 Count = FCString::Atoi(*Args[1]);

				// Bu ItemID'den yeterli stogu olan ILK dugumu Kaynak olarak kullan (basit dev-arac secimi).
				AStorageNode* FoundSource = nullptr;
				for (TActorIterator<AStorageNode> It(World); It; ++It)
				{
					if (It->GetStorage() && It->GetStorage()->HasItem(ItemID, Count))
					{
						FoundSource = *It;
						break;
					}
				}
				if (!FoundSource)
				{
					UE_LOG(LogSurvivalLogistics, Warning,
						TEXT("logistics_request: '%s' icin yeterli stoklu dugum bulunamadi"), *Args[0]);
					return;
				}

				const bool bRequested = Network->RequestTransport(ItemID, Count, FoundSource);
				UE_LOG(LogSurvivalLogistics, Log, TEXT("logistics_request '%s' x%d: %s"),
					*Args[0], Count, bRequested ? TEXT("kuyruga eklendi") : TEXT("basarisiz"));
			}));
}
