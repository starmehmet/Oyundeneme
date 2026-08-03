// Sistem #17 birim testleri — kayit sikistirma/acma round-trip sadakati (SaveDataSerializer.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.Save"

#include "Misc/AutomationTest.h"
#include "Save/SaveDataSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveDataSerializerRoundTripTest,
	"SurvivalGame.Save.SaveDataSerializer.SikistirmaRoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveDataSerializerRoundTripTest::RunTest(const FString& Parameters)
{
	USurvivalSaveGame* Original = NewObject<USurvivalSaveGame>();
	Original->Payload.SaveVersion = 1;
	Original->Payload.TotalPlayTimeSeconds = 123.5f;
	Original->Payload.TotalGameSeconds = 9876.25;
	Original->Payload.PlayerPosition = FVector(100.0, -200.0, 50.5);
	Original->Payload.PlayerHealth = 42.0f;
	Original->Payload.PlayerBodyTemperature = 35.5f;

	FInventorySlot SlotA;
	SlotA.ItemID = TEXT("Odun");
	SlotA.Count = 5;
	SlotA.Durability = 1.0f;
	FInventorySlot SlotB;
	SlotB.ItemID = TEXT("Balta");
	SlotB.Count = 1;
	SlotB.Durability = 0.75f;
	Original->Payload.PlayerInventory = { SlotA, SlotB };

	const TArray<uint8> Compressed = SurvivalSave::CompressSaveObject(Original);
	TestTrue(TEXT("sikistirilmis bayt dizisi bos degil"), Compressed.Num() > 0);

	USaveGame* LoadedBase = SurvivalSave::DecompressSaveObject(Compressed);
	USurvivalSaveGame* Loaded = Cast<USurvivalSaveGame>(LoadedBase);
	TestNotNull(TEXT("geri yuklenen nesne doğru sinifta"), Loaded);
	if (!Loaded)
	{
		return false;
	}

	TestEqual(TEXT("SaveVersion sadik"), Loaded->Payload.SaveVersion, 1);
	TestEqual(TEXT("TotalPlayTimeSeconds sadik"), Loaded->Payload.TotalPlayTimeSeconds, 123.5f);
	TestEqual(TEXT("TotalGameSeconds sadik"), Loaded->Payload.TotalGameSeconds, 9876.25);
	TestEqual(TEXT("PlayerPosition sadik"), Loaded->Payload.PlayerPosition, FVector(100.0, -200.0, 50.5));
	TestEqual(TEXT("PlayerHealth sadik"), Loaded->Payload.PlayerHealth, 42.0f);
	TestEqual(TEXT("PlayerBodyTemperature sadik"), Loaded->Payload.PlayerBodyTemperature, 35.5f);
	TestEqual(TEXT("envanter satir sayisi sadik"), Loaded->Payload.PlayerInventory.Num(), 2);
	if (Loaded->Payload.PlayerInventory.Num() == 2)
	{
		TestEqual(TEXT("ilk slot ItemID sadik"), Loaded->Payload.PlayerInventory[0].ItemID, FName(TEXT("Odun")));
		TestEqual(TEXT("ilk slot Count sadik"), Loaded->Payload.PlayerInventory[0].Count, 5);
		TestEqual(TEXT("ikinci slot Durability sadik"), Loaded->Payload.PlayerInventory[1].Durability, 0.75f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveDataSerializerV2RoundTripTest,
	"SurvivalGame.Save.SaveDataSerializer.V2RoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveDataSerializerV2RoundTripTest::RunTest(const FString& Parameters)
{
	USurvivalSaveGame* Original = NewObject<USurvivalSaveGame>();
	Original->Payload.SaveVersion = 2;
	Original->Payload.TotalPlayTimeSeconds = 500.0f;
	Original->Payload.TotalGameSeconds = 12000.0;
	Original->Payload.PlayerPosition = FVector(1.0, 2.0, 3.0);
	Original->Payload.PlayerHealth = 80.0f;
	Original->Payload.PlayerBodyTemperature = 36.5f;
	Original->Payload.SnowDepth = 0.35f;

	FBuildingSaveData BSD;
	BSD.BuildingID = FName(TEXT("Firin"));
	BSD.GridCoord = FIntPoint(3, 5);
	BSD.Rotation = FRotator(0.0f, 90.0f, 0.0f);
	BSD.ActiveRecipeID = FName(TEXT("Ekmek"));
	BSD.MachineProgress = 2.5f;
	BSD.MachineEnergy = 75.0f;
	BSD.MachineState = 1;
	Original->Payload.Buildings.Add(BSD);

	FHarvestNodeSaveData HSD;
	HSD.ActorName = FName(TEXT("AgacDugumu_01"));
	HSD.RemainingHarvests = 3;
	HSD.bDepleted = false;
	HSD.DepletionGameTime = 0.0;
	Original->Payload.HarvestNodes.Add(HSD);

	FNPCSaveData NSD;
	NSD.ActorName = FName(TEXT("NPC_Isci_01"));
	NSD.Position = FVector(500.0, 600.0, 0.0);
	NSD.Health = 55.0f;
	NSD.Fatigue = 30.0f;
	NSD.Morale = 0.7f;
	NSD.BrainState = 2;
	Original->Payload.NPCs.Add(NSD);

	FTaskSaveData TSD;
	TSD.TaskID = FName(TEXT("GorevA"));
	TSD.Priority = 5.0f;
	TSD.WorkDuration = 10.0f;
	Original->Payload.PendingTasks.Add(TSD);

	Original->Payload.AudioVolumes.Add(ESoundCategory::Master, 0.8f);
	Original->Payload.AudioVolumes.Add(ESoundCategory::Music, 0.5f);

	const TArray<uint8> Compressed = SurvivalSave::CompressSaveObject(Original);
	TestTrue(TEXT("v2 sikistirilmis bayt dizisi bos degil"), Compressed.Num() > 0);

	USaveGame* LoadedBase = SurvivalSave::DecompressSaveObject(Compressed);
	USurvivalSaveGame* Loaded = Cast<USurvivalSaveGame>(LoadedBase);
	TestNotNull(TEXT("v2 nesne doğru sinifta"), Loaded);
	if (!Loaded)
	{
		return false;
	}

	TestEqual(TEXT("v2 SaveVersion"), Loaded->Payload.SaveVersion, 2);
	TestEqual(TEXT("v2 SnowDepth"), Loaded->Payload.SnowDepth, 0.35f);
	TestEqual(TEXT("v2 Buildings sayisi"), Loaded->Payload.Buildings.Num(), 1);
	if (Loaded->Payload.Buildings.Num() == 1)
	{
		TestEqual(TEXT("v2 BuildingID"), Loaded->Payload.Buildings[0].BuildingID, FName(TEXT("Firin")));
		TestEqual(TEXT("v2 GridCoord"), Loaded->Payload.Buildings[0].GridCoord, FIntPoint(3, 5));
		TestEqual(TEXT("v2 MachineProgress"), Loaded->Payload.Buildings[0].MachineProgress, 2.5f);
	}
	TestEqual(TEXT("v2 HarvestNodes sayisi"), Loaded->Payload.HarvestNodes.Num(), 1);
	TestEqual(TEXT("v2 NPCs sayisi"), Loaded->Payload.NPCs.Num(), 1);
	if (Loaded->Payload.NPCs.Num() == 1)
	{
		TestEqual(TEXT("v2 NPC Fatigue"), Loaded->Payload.NPCs[0].Fatigue, 30.0f);
	}
	TestEqual(TEXT("v2 PendingTasks sayisi"), Loaded->Payload.PendingTasks.Num(), 1);
	TestEqual(TEXT("v2 AudioVolumes sayisi"), Loaded->Payload.AudioVolumes.Num(), 2);

	const float* MasterVol = Loaded->Payload.AudioVolumes.Find(ESoundCategory::Master);
	TestNotNull(TEXT("v2 Master volume var"), MasterVol);
	if (MasterVol)
	{
		TestEqual(TEXT("v2 Master volume degeri"), *MasterVol, 0.8f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveDataSerializerCorruptionTest,
	"SurvivalGame.Save.SaveDataSerializer.BozukVeriKorumasi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSaveDataSerializerCorruptionTest::RunTest(const FString& Parameters)
{
	// Bos dizi -> null (cokme yok)
	TestNull(TEXT("bos dizi -> null"), SurvivalSave::DecompressSaveObject(TArray<uint8>()));

	// Rastgele/gecersiz bayt dizisi (gercek bir sikistirilmis akis DEGIL, imza da eslesmiyor)
	// -> null (cokme yok — inceleme bulgusu: motorun kendi sikistirma-cozme kodu boyle
	// veride GERCEKTEN cokebiliyordu, kendi zarf-dogrulamamiz bu yola hic ulasmadan eler)
	TArray<uint8> Garbage;
	Garbage.Init(0xFF, 64);
	TestNull(TEXT("bozuk/rastgele veri -> null"), SurvivalSave::DecompressSaveObject(Garbage));

	// GECERLI bir zarf uretilip TEK bir bayti bozulursa (CRC32 artik eslesmez) -> null (cokme yok)
	USurvivalSaveGame* ValidObject = NewObject<USurvivalSaveGame>();
	ValidObject->Payload.PlayerHealth = 77.0f;
	TArray<uint8> ValidEnvelope = SurvivalSave::CompressSaveObject(ValidObject);
	TestTrue(TEXT("gecerli zarf uretildi"), ValidEnvelope.Num() > 0);
	if (ValidEnvelope.Num() > 0)
	{
		TArray<uint8> Corrupted = ValidEnvelope;
		Corrupted.Last() ^= 0xFF; // son bayti ters cevir — CRC32 artik uyusmamali
		TestNull(TEXT("tek bayt bozulmus GECERLI zarf -> null (CRC32 yakaladi)"), SurvivalSave::DecompressSaveObject(Corrupted));

		TArray<uint8> Truncated = ValidEnvelope;
		Truncated.SetNum(Truncated.Num() - 4); // kesilmis (eksik) veri
		TestNull(TEXT("kesilmis GECERLI zarf -> null (uzunluk uyusmazligi yakaladi)"), SurvivalSave::DecompressSaveObject(Truncated));
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
