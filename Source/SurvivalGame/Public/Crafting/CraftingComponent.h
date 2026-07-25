#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

class UInventoryComponent;

/**
 * Sistem #6 — Aktif bir zanaat işi. Kimlik RecipeID (FName) — URecipeDatabase üzerinden
 * çözülür, iş kendisi tarif kopyası TUTMAZ (Sistem #4/#5'teki tek-doğruluk-kaynağı deseni).
 */
USTRUCT(BlueprintType)
struct FCraftingJob
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FName RecipeID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ElapsedTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingProgress, int32, JobIndex, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingCompleted, FName, RecipeID, bool, bSuccess);

/**
 * Sistem #6 — Tarif doğrulama + zamanlı üretim kuyruğu. Aynı aktördeki
 * UInventoryComponent'i kullanır (malzeme tüketimi/üretim/iade).
 *
 * StartCrafting malzemeleri HEMEN tüketir (iş kuyruğa girdiği an) — CancelCrafting
 * bunları GERİ İADE eder. Bu, "malzeme tüketimi ne zaman gerçekleşir" belirsizliğini
 * ortadan kaldırır: kuyruktaki her iş için malzeme garanti ayrılmıştır.
 */
UCLASS(ClassGroup = (Crafting), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

	/** Envanterde yeterli malzeme var mı VE kuyrukta yer var mı? */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool CanCraftRecipe(FName RecipeID) const;

	/** Malzemeleri hemen tüketir, işi kuyruğa ekler. Başarılıysa true. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool StartCrafting(FName RecipeID);

	/** İşi iptal eder, tüketilen malzemeleri iade eder. Başarılıysa true. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool CancelCrafting(int32 JobIndex);

	UFUNCTION(BlueprintPure, Category = "Crafting")
	const TArray<FCraftingJob>& GetActiveJobs() const { return ActiveJobs; }

	UFUNCTION(BlueprintPure, Category = "Crafting")
	int32 GetActiveJobCount() const { return ActiveJobs.Num(); }

	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnCraftingProgress OnCraftingProgress;

	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnCraftingCompleted OnCraftingCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void CompleteJob(int32 JobIndex);

	UPROPERTY(EditDefaultsOnly, Category = "Crafting", meta = (ClampMin = "1"))
	int32 MaxQueueSize = 16;

	UPROPERTY()
	TArray<FCraftingJob> ActiveJobs;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> OwnerInventory;
};
