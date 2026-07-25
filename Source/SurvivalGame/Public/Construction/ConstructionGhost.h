#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ConstructionGhost.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/**
 * Sistem #7 — Yerleştirme önizleme aktörü ("hayalet"). Yalnızca VERİ seviyesinde
 * geçerlilik taşır (bIsValid, BlueprintReadOnly — MCP/PIE doğrulaması için, Sistem #3'teki
 * InteractionCount deseniyle aynı). Kırmızı/yeşil dinamik materyal renk-geri bildirimi
 * bu pasoda YAZILMADI — henüz materyal/mesh içeriği yok; içerik fazında (Sistem #24)
 * eklenebilir (bkz. ADR).
 *
 * Mesh çarpışmasız (NoCollision) — önizleme dünyayla fiziksel olarak etkileşmez,
 * kendi kendini "dolu hücre" olarak göstermesin diye.
 */
UCLASS()
class SURVIVALGAME_API AConstructionGhost : public AActor
{
	GENERATED_BODY()

public:
	AConstructionGhost();

	/** Önizlemeyi verilen konum/geçerliliğe göre günceller (mesh, konum, ızgara hücresi, bayrak). */
	void UpdatePreview(UStaticMesh* PreviewMesh, const FVector& WorldLocation, const FIntPoint& InGridCoord, bool bValid);

	UFUNCTION(BlueprintPure, Category = "Construction")
	bool IsValidPlacement() const { return bIsValid; }

	UFUNCTION(BlueprintPure, Category = "Construction")
	FIntPoint GetGridCoord() const { return GridCoord; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Construction")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Construction")
	FIntPoint GridCoord = FIntPoint::ZeroValue;
};
