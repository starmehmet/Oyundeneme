#pragma once

#include "CoreMinimal.h"
#include "StorageNodeType.generated.h"

/**
 * Sistem #8 — Lojistik ağındaki bir düğümün rolü. MachineInput/MachineOutput şimdilik
 * yalnızca VERİ (Sistem #9 Üretim Makineleri tüketecek) — bu pasoda hiçbir aktör bu iki
 * değeri kullanmıyor ama enum'un tamamı MIMARI.md ile birebir eşleşsin diye baştan tanımlı.
 */
UENUM(BlueprintType)
enum class EStorageNodeType : uint8
{
	Container,
	MachineInput,
	MachineOutput,
	Sink,
	Source
};
