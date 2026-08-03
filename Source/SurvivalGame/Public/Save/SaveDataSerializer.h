#pragma once

#include "CoreMinimal.h"
#include "Save/SaveDataTypes.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Crc.h"

/**
 * Sistem #17 — Kayıt nesnesi <-> sıkıştırılmış bayt dizisi dönüşümü. Tam olarak "saf" değil
 * (motor API'leri çağırıyor) ama YAN ETKİSİZ ve deterministik — round-trip sadakati
 * Private/Tests/SaveDataSerializerTests.cpp'de test edilir.
 *
 * `UGameplayStatics::SaveGameToMemory`/`LoadGameFromMemory` motorun KENDİ `USaveGame`
 * serileştirmesi (`GameplayStatics.cpp`'den doğrulandı: `FMemoryWriter`+
 * `FObjectAndNameAsStringProxyArchive` — elle yeniden yazılmadı). GERÇEK sıkıştırma
 * `FArchiveSaveCompressedProxy`/`FArchiveLoadCompressedProxy` (zlib, `Core` modülünün
 * kendi API'si) ile bu ham baytların ÜZERİNE eklenir — DoD'nin "kayıtları sıkıştır"
 * maddesi + Ölçeklenebilirlik Riski'nin önerdiği çözüm.
 *
 * İNCELEME BULGUSU (motor kaynağından doğrulandı, `Archive.cpp::SerializeCompressedNew`):
 * `FArchiveLoadCompressedProxy`, bozuk/rastgele baytlarda `Decompressor.GetError()` ile
 * "hata" DÖNMEZ — başlık etiketi eşleşmezse `UE_LOGF(..., Fatal, ...)` ile DOĞRUDAN ÇÖKER
 * (`LowLevelFatalError`), ayrıca kısalmış veride ayrı bir `check()` de çökebilir. Yani bu
 * motor API'si "güvenilir/kendi ürettiğimiz" veri için tasarlanmış, GÜVENİLMEYEN keyfi
 * baytlara karşı DAYANIKLI DEĞİL. Çözüm: kendi zarfımızı (magic+CRC32+uzunluk) ekliyoruz;
 * `DecompressSaveObject` motorun sıkıştırma-çözme kodunu YALNIZCA bu zarf TAM olarak
 * doğrulandıktan SONRA çağırır — CRC32 eşleşmesi, baytların `CompressSaveObject`'in
 * ÜRETTİĞİ baytlarla AYNI olduğunu garanti eder, bu yüzden bu noktadan sonra motor
 * çağrısı güvenlidir.
 */
namespace SurvivalSave
{
	namespace Private
	{
		// 'S','V','G','1' baytlarindan olusan sabit bir imza — zarfin bizim formatimiz
		// oldugunu (yanlis dosya/format degil) hizlica dogrulamak icin.
		constexpr uint32 SaveEnvelopeMagic = 0x31474D53;
		constexpr int64 SaveEnvelopeHeaderSize = sizeof(uint32) + sizeof(uint32) + sizeof(int32); // magic+crc+uzunluk
	}

	/** `SaveObject` başarısız serileştirilirse veya sıkıştırma sonucu boşsa BOŞ dizi döner
	 * (çağıran taraf bunu hata sayabilir). */
	inline TArray<uint8> CompressSaveObject(USaveGame* SaveObject)
	{
		if (!SaveObject)
		{
			return TArray<uint8>();
		}

		TArray<uint8> RawBytes;
		if (!UGameplayStatics::SaveGameToMemory(SaveObject, RawBytes) || RawBytes.Num() == 0)
		{
			return TArray<uint8>();
		}

		TArray<uint8> CompressedBytes;
		{
			FArchiveSaveCompressedProxy Compressor(CompressedBytes, NAME_Zlib);
			Compressor << RawBytes;
			Compressor.Flush();
		}
		if (CompressedBytes.Num() == 0)
		{
			return TArray<uint8>();
		}

		TArray<uint8> Envelope;
		FMemoryWriter Writer(Envelope);
		uint32 Magic = Private::SaveEnvelopeMagic;
		uint32 Crc = FCrc::MemCrc32(CompressedBytes.GetData(), CompressedBytes.Num());
		int32 PayloadLength = CompressedBytes.Num();
		Writer << Magic;
		Writer << Crc;
		Writer << PayloadLength;
		Writer.Serialize(CompressedBytes.GetData(), CompressedBytes.Num());
		return Envelope;
	}

	/**
	 * Bozuk/eksik veriye karşı GERÇEKTEN korumalı (yukarıdaki inceleme notuna bakınız):
	 * imza uyuşmazlığı, uzunluk uyuşmazlığı veya CRC32 uyuşmazlığı — üçü de motor
	 * sıkıştırma-çözme koduna HİÇ ULAŞMADAN `nullptr` döner, çökme YOK.
	 */
	inline USaveGame* DecompressSaveObject(const TArray<uint8>& EnvelopeBytes)
	{
		if (EnvelopeBytes.Num() < Private::SaveEnvelopeHeaderSize)
		{
			return nullptr;
		}

		FMemoryReader Reader(EnvelopeBytes);
		uint32 Magic = 0;
		uint32 StoredCrc = 0;
		int32 PayloadLength = 0;
		Reader << Magic;
		Reader << StoredCrc;
		Reader << PayloadLength;

		if (Magic != Private::SaveEnvelopeMagic)
		{
			return nullptr; // yanlis dosya / taninmayan format
		}
		if (PayloadLength < 0 || EnvelopeBytes.Num() != Private::SaveEnvelopeHeaderSize + PayloadLength)
		{
			return nullptr; // kesilmis/eklenmis veri — beyan edilen uzunluk GERCEK boyutla TAM eslesmeli
		}

		TArray<uint8> CompressedBytes;
		CompressedBytes.Append(EnvelopeBytes.GetData() + Private::SaveEnvelopeHeaderSize, PayloadLength);

		if (FCrc::MemCrc32(CompressedBytes.GetData(), CompressedBytes.Num()) != StoredCrc)
		{
			return nullptr; // bit-hatasi/kismi yazim — icerik bozulmus
		}

		// Bu noktada CompressedBytes, CompressSaveObject'in URETTIGI baytlarla CRC32
		// eslesmesi yoluyla dogrulandi — motorun sikistirma-cozme kodu artik GUVENLE cagirilabilir.
		FArchiveLoadCompressedProxy Decompressor(CompressedBytes, NAME_Zlib);
		TArray<uint8> RawBytes;
		Decompressor << RawBytes;
		if (Decompressor.GetError() || RawBytes.Num() == 0)
		{
			return nullptr;
		}

		return UGameplayStatics::LoadGameFromMemory(RawBytes);
	}

	inline void MigrateSaveData(FGameSaveData& Data)
	{
		constexpr int32 CurrentSaveVersion = 2;
		// v1 -> v2: yeni alanlar (Buildings, Weather, Snow, Resources, HarvestNodes, NPCs,
		// PendingTasks, AudioVolumes) UPROPERTY varsayilanlariyla zaten bos/sifir gelir —
		// ek donusum gerekmiyor (ekleme-tabanli genisleme).
		Data.SaveVersion = CurrentSaveVersion;
	}
}
