#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Construction/BuildingDefinition.h"
#include "BuildingBase.generated.h"

class UStaticMeshComponent;

/**
 * Sistem #7 — Yerleşmiş bina aktörü. MIMARI.md'de AProductionMachine (Sistem #9) bu
 * sınıftan türeyecek şekilde tasarlanmış — bu yüzden inşa/kayıt mantığı burada değil
 * UConstructionComponent'te tutulur, ABuildingBase yalnızca "yerleşmiş bir binanın
 * durumunu taşıma + yıkılabilme" sorumluluğunu üstlenir (alt sınıflar bunu bozmadan
 * genişletebilsin diye BeginConstruction/Demolish virtual).
 *
 * İnşaat ANLIK tamamlanır (iskele/zamanla-inşa YOK) — BeginConstruction çağrıldığı an
 * bIsConstructed=true, ConstructionProgress=1.0f olur. DoD'de zaman kavramı yok;
 * zamanla-inşa CraftingComponent'in FCraftingJob/tick desenini kopyalardı (bkz. ADR,
 * Sistem #6'daki ACraftingStation kapsam-dışı kararına paralel). Definition::ConstructionTime
 * veri olarak duruyor, gelecekte tüketilebilir.
 *
 * NotPlaceable — YALNIZCA UConstructionComponent::ConfirmPlacement üzerinden SpawnActor ile
 * var olur, editörde Place Actors panelinden elle sürüklenemez. Bunun nedeni: GridCoord yalnızca
 * BeginConstruction çağrılınca anlamlı bir değer alır; elle yerleştirilmiş (BeginConstruction
 * hiç çağrılmamış) bir örnekte Demolish() varsayılan GridCoord'u ((0,0)) grid'den siler ve
 * o hücrede GERÇEKTEN duran başka bir binanın kaydını bozar (inceleme bulgusu — kök nedeni
 * ortadan kaldırmak, bir bayrakla etrafını sarmaktan daha temiz).
 */
UCLASS(NotPlaceable)
class SURVIVALGAME_API ABuildingBase : public AActor
{
	GENERATED_BODY()

public:
	ABuildingBase();

	/** Grid'e kaydedilmeden HEMEN önce UConstructionComponent tarafından çağrılır. */
	virtual void BeginConstruction(FName InBuildingID, const FBuildingDefinition& InDefinition, const FIntPoint& InGridCoord);

	/**
	 * Grid'den kendini çıkarır (varsa) ve aktörü yok eder. Malzeme iadesi YAPILMAZ (bkz. ADR).
	 * Bilerek E-etkileşimine BAĞLANMADI (kazayla bina yıkma riski, gerçek yıkım UX'i henüz
	 * tasarlanmadı) — `build_demolish` konsol komutu (dev-tool) + bu public API PIE
	 * doğrulaması için yeterli; içerik fazında özel bir yıkım aracı/modu eklenebilir.
	 */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	virtual void Demolish();

	UFUNCTION(BlueprintPure, Category = "Construction")
	FName GetBuildingID() const { return BuildingID; }

	UFUNCTION(BlueprintPure, Category = "Construction")
	FIntPoint GetGridCoord() const { return GridCoord; }

	UFUNCTION(BlueprintPure, Category = "Construction")
	bool IsConstructed() const { return bIsConstructed; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Construction")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	FName BuildingID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	float ConstructionProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	bool bIsConstructed = false;
};
