#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "HarvestNodeManager.generated.h"

class AHarvestNode;

/**
 * Sistem #29 — UProductionManager/UNPCManager ile AYNI kare-bolumleme iskeleti
 * (SurvivalProduction::ComputeBatchSize dogrudan yeniden kullanilir — yeni bir dilim-boyutu
 * formulu icat edilmez, bkz. Docs/MIMARI.md #29). SADECE TUKENMIS dugumleri takip eder —
 * dolu dugumler bu kumeye hic girmez, ProductionManager'in "TUM makineler her zaman tick'te"
 * modelinden daha ucuz bir alt kume; IsTickable() bekleyen tukenmis dugum yokken false doner.
 */
UCLASS()
class SURVIVALGAME_API UHarvestNodeManager : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UHarvestNodeManager, STATGROUP_Tickables);
	}

	/** AHarvestNode tukendiginde kendini kaydeder (bkz. HarvestNode.cpp::OnInteract_Implementation). */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void RegisterDepletedNode(AHarvestNode* Node);

	/** Manager'in kendi Tick'i (yeniden dogunca) VEYA dugumun EndPlay'i VEYA bir dev-komutu
	 * cagirir — DIKKAT: AHarvestNode::Respawn() bu fonksiyonu KENDISI cagirmaz (yeniden-giriş/
	 * dizi-degistirme cakismasini onlemek icin cagiran taraf iki adimi ayri ayri yapar). */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void UnregisterDepletedNode(AHarvestNode* Node);

	UFUNCTION(BlueprintPure, Category = "Harvest")
	int32 GetDepletedNodeCount() const { return DepletedNodes.Num(); }

	UFUNCTION(BlueprintPure, Category = "Harvest")
	double GetGameTime() const { return GameTime; }

private:
	UPROPERTY()
	TArray<TObjectPtr<AHarvestNode>> DepletedNodes;

	double GameTime = 0.0;
	int32 PartitionIndex = 0;

	// CLAUDE.md ornegi: "500 makine / 60 frame" — bir dongude TUM tukenmis dugumler en az bir kez kontrol edilir.
	static constexpr int32 FramesPerCycle = 60;
};
