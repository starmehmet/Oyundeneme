#pragma once

#include "CoreMinimal.h"
#include "Logistics/StorageNodeType.h"

/**
 * Sistem #8 — Lojistik Ağı: saf, durum tutmayan düğüm-uygunluğu/taşıma-süresi mantığı.
 * Yan etkisiz; birim testleri Private/Tests/LogisticsMathTests.cpp.
 */
namespace SurvivalLogistics
{
	/**
	 * Bu düğüm TİPİ öğe kabul edebilir mi? Gerçek kapasite/ağırlık kontrolü teslimat anında
	 * UInventoryComponent::AddItem'in döndürdüğü gerçek-kabul-edilen miktardan anlaşılır —
	 * burası yalnızca "bu düğüm TÜRÜ prensipte bir alıcı mı" sorusuna cevap verir.
	 */
	inline bool CanNodeTypeAccept(EStorageNodeType NodeType)
	{
		switch (NodeType)
		{
		case EStorageNodeType::Source:
		case EStorageNodeType::MachineOutput:
			return false;
		default:
			return true;
		}
	}

	/** Mesafe/hız → taşıma süresi (sn). Speed<=0 ise anlık (0) döner — sıfıra bölme yok. */
	inline float ComputeTransportTime(float Distance, float Speed)
	{
		if (Speed <= 0.0f)
		{
			return 0.0f;
		}
		return FMath::Max(0.0f, Distance) / Speed;
	}

	/** İlerleme oranı [0,1]. TransportTime<=0 anlık teslimat demektir — her zaman tamamlanmış sayılır. */
	inline float ComputeTransportProgress(float ElapsedTime, float TransportTime)
	{
		if (TransportTime <= 0.0f)
		{
			return 1.0f;
		}
		return FMath::Clamp(ElapsedTime / TransportTime, 0.0f, 1.0f);
	}

	inline bool IsTransportComplete(float ElapsedTime, float TransportTime)
	{
		return ElapsedTime >= TransportTime;
	}
}
