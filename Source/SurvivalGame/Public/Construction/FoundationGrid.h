#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FoundationGrid.generated.h"

class ABuildingBase;

/**
 * Sistem #7 — Izgara hücre işgalinin tek sahibi. MIMARI.md taslağındaki AConstructionManager
 * (GameStateBase) + AFoundationGrid (AActor) ikilisi YERİNE TEK bir UWorldSubsystem —
 * CLAUDE.md kuralı "Singleton = UGameInstanceSubsystem, elle yönetilen static manager yazma"
 * World-kapsamlı hâliyle burada uygulanıyor: level'a manuel bir grid/manager aktörü
 * yerleştirmeye gerek kalmaz, dünya var olur olmaz grid de var olur (bkz. ADR).
 *
 * Çarpışma doğrulaması BİLEREK ızgara hücre işgaline dayanır (fiziksel overlap/box
 * sorgusu DEĞİL) — MIMARI.md'nin kendi "Ölçeklenebilirlik Riski" notu bunu 500 bina
 * için ÇÖZÜM olarak öneriyor (O(n) çarpışma kutusu yerine O(1) TSet arama).
 */
UCLASS()
class SURVIVALGAME_API UFoundationGrid : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category = "Construction")
	FIntPoint WorldToGridCoord(const FVector& WorldPosition) const;

	UFUNCTION(BlueprintPure, Category = "Construction")
	FVector GridCoordToWorld(const FIntPoint& Coord, float ZHeight = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "Construction")
	bool IsCellOccupied(const FIntPoint& Coord) const { return OccupiedCells.Contains(Coord); }

	/** Hücrede kayıtlı bina (yoksa nullptr). */
	UFUNCTION(BlueprintPure, Category = "Construction")
	ABuildingBase* GetBuildingAt(const FIntPoint& Coord) const;

	/** Hücre zaten doluysa false döner (kaydetmez) — çağıran taraf önce IsCellOccupied ile de kontrol edebilir. */
	UFUNCTION(BlueprintCallable, Category = "Construction")
	bool RegisterBuilding(ABuildingBase* Building, const FIntPoint& Coord);

	UFUNCTION(BlueprintCallable, Category = "Construction")
	void UnregisterBuilding(const FIntPoint& Coord);

	UFUNCTION(BlueprintPure, Category = "Construction")
	float GetGridSize() const { return GridSize; }

	UFUNCTION(BlueprintPure, Category = "Construction")
	int32 GetOccupiedCellCount() const { return OccupiedCells.Num(); }

private:
	float GridSize = 400.0f;

	TSet<FIntPoint> OccupiedCells;

	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<ABuildingBase>> BuildingMap;
};
