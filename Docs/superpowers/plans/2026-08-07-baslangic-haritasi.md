# Gerçek Başlangıç Haritası — Uygulama Planı

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Alpha dağıtımı için, tasarlanmış arazi ve yönlendirilmiş açılışa sahip gerçek bir başlangıç haritası üretmek.

**Architecture:** Üç bağımsız katman. (1) Tohumlanmış, saf/test-edilebilir yükseklik matematiği bir editör-only konsol komutunu besler; komut `ALandscapeProxy::Import()` ile gerçek bir UE Landscape üretir ve `.umap`'e gömer — runtime maliyeti sıfır. (2) Dokusuz, yükseklik+eğim tabanlı landscape materyali. (3) Elle tasarlanmış başlangıç vadisi + aynı tohumdan deterministik geniş dünya içeriği.

**Tech Stack:** UE 5.8, C++ (`SurvivalGame` tek runtime modülü), `Landscape` modülü (yalnız editör), UE Automation birim testleri, MCP `unreal-mcp` köprüsü (varlık/sahne işlemleri).

## Global Constraints

Bunlar HER görev için geçerlidir; her görevin gereksinimlerine örtük olarak dahildir.

- **Kaynak dosya kodlaması UTF-8 with BOM.** `.h/.cpp/.cs` dosyaları BOM'suz yazılırsa MSVC Türkçe Windows'ta cp1254 okur ve `TEXT()` literalleri bozulur. Kod düzenleme seansı sonunda `powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1` çalıştır (Write/Edit araçları BOM'suz yazar).
- **Log kategorisi:** yalnızca `LogSurvival*` (`SurvivalGame.h`). `UE_LOG(LogTemp, ...)` yasak.
- **Otomasyon testi flag'i:** `EAutomationTestFlags_ApplicationContextMask` (serbest sabit). Üye biçimi `EAutomationTestFlags::ApplicationContextMask` UE 5.8'de YOK, derlemeyi kırar.
- **Test sonucu `TestResults\index.json`'dan okunur**, stdout'tan DEĞİL (`succeeded`/`failed`/`notRun`).
- **Gölgeleme yasağı:** yerel değişkene miras alınan üye adı verme (`Character`, `Owner`, `InputComponent`, `Controller`). UE C4458'i HATA olarak derler. `PlayerChar` gibi farklı ad kullan.
- **`-ExecCmds` VİRGÜLLE ayrılır**, noktalı virgülle değil (`ParseExecCommands.cpp:28`). `;` kullanılırsa UE tüm dizeyi tek komut sayar, kalan komutlar sessizce çalışmaz.
- **MCP ile oluşturulan varlıklar uçucudur** — hemen `save_assets` çağır, sonra diskten geri oku. Doğrulama sırası: yaz → kaydet → diskten oku.
- **Editör açıkken `Config/*.ini` düzenlemesi çalışan oturuma yansımaz** (config boot'ta okunur).
- **Derleme:** editör KAPALI olmalı (modül DLL kilidi). `& "C:\Users\atoly\Desktop\Unreal\UE_5.8\Engine\Build\BatchFiles\Build.bat" SurvivalGameEditor Win64 Development -project="C:\Users\atoly\Downloads\Oyun\SurvivalGame.uproject" -waitmutex`
- **Mevcut 55 test regresyonsuz kalmalı.**
- **`Content/Maps/Tests/TestPlayer` SİLİNMEZ** — otomasyon testleri ve mevcut PIE akışları ona bağlı.
- **Commit mesajlarında model tanımlayıcısı geçmez.**

---

## Dosya Yapısı

| Dosya | Sorumluluk |
|---|---|
| `Source/SurvivalGame/Public/World/HeightmapMath.h` | **Yeni.** Saf, durumsuz yükseklik matematiği: hash, değer gürültüsü, fBm, düzleştirme maskesi, heightmap üretimi. Motor RNG'si yok, tamamen deterministik. |
| `Source/SurvivalGame/Private/Tests/HeightmapMathTests.cpp` | **Yeni.** Yukarıdakinin 5 birim testi. |
| `Source/SurvivalGame/Private/World/LandscapeBuilder.cpp` | **Yeni.** Tamamı `#if WITH_EDITOR`. `survival_generate_landscape` konsol komutu; heightmap → `ALandscape`. |
| `Source/SurvivalGame/SurvivalGame.Build.cs` | **Değişiklik.** Editör derlemelerine `Landscape` bağımlılığı. |
| `Content/Maps/BaslangicHaritasi.umap` | **Yeni.** Asıl harita. |
| `Content/Materials/M_Landscape.uasset` | **Yeni.** Dokusuz yükseklik+eğim materyali. |
| `Config/DefaultEngine.ini` | **Değişiklik.** `GameDefaultMap` / `EditorStartupMap` yeni haritaya. |

---

### Task 1: Saf yükseklik matematiği (`HeightmapMath.h`)

**Files:**
- Create: `Source/SurvivalGame/Public/World/HeightmapMath.h`
- Test: `Source/SurvivalGame/Private/Tests/HeightmapMathTests.cpp`

**Interfaces:**
- Consumes: hiçbir şey (bu ilk görev, yalnız `CoreMinimal.h`)
- Produces: `SurvivalHeightmap` ad alanı —
  - `struct FHeightmapParams` (alanlar: `int32 Seed`, `int32 Octaves`, `float Frequency`, `float Lacunarity`, `float Persistence`, `FIntPoint FlattenCenter`, `float FlattenRadius`, `float FlattenFalloff`, `float FlattenHeight`)
  - `uint32 HashCoord(int32 X, int32 Y, int32 Seed)`
  - `float HashToUnit(uint32 H)`
  - `float ValueNoise2D(float X, float Y, int32 Seed)`
  - `float SampleFBM(float X, float Y, const FHeightmapParams& P)`
  - `float ApplyFlattenMask(float InHeight, int32 X, int32 Y, const FHeightmapParams& P)`
  - `TArray<uint16> GenerateHeightmap(int32 SizeX, int32 SizeY, const FHeightmapParams& P)`
  - `inline constexpr uint16 HeightMidpoint = 32768;`
  - `inline constexpr float HeightSpan = 15000.0f;`

- [ ] **Step 1: Testleri yaz (başarısız olacaklar)**

`Source/SurvivalGame/Private/Tests/HeightmapMathTests.cpp` oluştur:

```cpp
// Baslangic haritasi arazi uretimi — saf yukseklik matematigi birim testleri (HeightmapMath.h).
// Calistirma: Editor -> Session Frontend -> Automation -> "SurvivalGame.World"

#include "Misc/AutomationTest.h"
#include "World/HeightmapMath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDeterminizmTest,
	"SurvivalGame.World.HeightmapMath.Determinizm",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDeterminizmTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 4242;

	// Ayni tohum + ayni koordinat -> bit-birebir ayni sonuc (iki ayri cagri)
	for (int32 i = 0; i < 32; ++i)
	{
		const float X = static_cast<float>(i) * 13.7f;
		const float Y = static_cast<float>(i) * 7.3f;
		TestEqual(TEXT("SampleFBM deterministik"), SampleFBM(X, Y, P), SampleFBM(X, Y, P));
	}

	// Tum heightmap de deterministik
	const TArray<uint16> A = GenerateHeightmap(64, 64, P);
	const TArray<uint16> B = GenerateHeightmap(64, 64, P);
	TestEqual(TEXT("heightmap uzunlugu ayni"), A.Num(), B.Num());
	bool bIdentical = (A.Num() == B.Num());
	for (int32 i = 0; bIdentical && i < A.Num(); ++i)
	{
		bIdentical = (A[i] == B[i]);
	}
	TestTrue(TEXT("heightmap bit-birebir ayni"), bIdentical);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapTohumAyrimiTest,
	"SurvivalGame.World.HeightmapMath.TohumAyrimi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapTohumAyrimiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P1; P1.Seed = 1;
	FHeightmapParams P2; P2.Seed = 2;
	// Duzlestirme bu testi bozmasin: maskeyi kapsam disina it
	P1.FlattenRadius = 0.0f; P1.FlattenFalloff = 0.0f;
	P2.FlattenRadius = 0.0f; P2.FlattenFalloff = 0.0f;

	double TotalDiff = 0.0;
	const int32 Samples = 256;
	for (int32 i = 0; i < Samples; ++i)
	{
		const float X = static_cast<float>(i) * 11.0f;
		const float Y = static_cast<float>(i) * 5.0f;
		TotalDiff += FMath::Abs(SampleFBM(X, Y, P1) - SampleFBM(X, Y, P2));
	}
	const double MeanDiff = TotalDiff / static_cast<double>(Samples);

	// Farkli tohum belirgin farkli arazi vermeli (ayni cikti = tohum kullanilmiyor demek)
	TestTrue(TEXT("farkli tohum -> belirgin fark"), MeanDiff > 0.02);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDegerAraligiTest,
	"SurvivalGame.World.HeightmapMath.DegerAraligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDegerAraligiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 99;

	// SampleFBM her zaman [0,1]
	bool bInUnitRange = true;
	for (int32 i = 0; i < 512; ++i)
	{
		const float V = SampleFBM(static_cast<float>(i) * 3.1f, static_cast<float>(i) * 2.7f, P);
		bInUnitRange = bInUnitRange && (V >= 0.0f) && (V <= 1.0f);
	}
	TestTrue(TEXT("SampleFBM [0,1] icinde"), bInUnitRange);

	// GenerateHeightmap ciktisi uint16 araliginda ve dogru uzunlukta
	const TArray<uint16> H = GenerateHeightmap(48, 32, P);
	TestEqual(TEXT("heightmap uzunlugu SizeX*SizeY"), H.Num(), 48 * 32);

	// Hedeflenen bant: HeightMidpoint +/- HeightSpan/2 icinde kalmali (tasma/sarma yok)
	const int32 Lo = HeightMidpoint - FMath::CeilToInt(HeightSpan * 0.5f) - 1;
	const int32 Hi = HeightMidpoint + FMath::CeilToInt(HeightSpan * 0.5f) + 1;
	bool bInBand = true;
	for (int32 i = 0; i < H.Num(); ++i)
	{
		bInBand = bInBand && (static_cast<int32>(H[i]) >= Lo) && (static_cast<int32>(H[i]) <= Hi);
	}
	TestTrue(TEXT("heightmap hedef bant icinde"), bInBand);

	// Bozuk girdi cokmez, bos doner
	TestEqual(TEXT("SizeX=0 -> bos"), GenerateHeightmap(0, 32, P).Num(), 0);
	TestEqual(TEXT("negatif boyut -> bos"), GenerateHeightmap(-4, 32, P).Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapDuzlestirmeMaskesiTest,
	"SurvivalGame.World.HeightmapMath.DuzlestirmeMaskesi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapDuzlestirmeMaskesiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 7;
	P.FlattenCenter = FIntPoint(100, 100);
	P.FlattenRadius = 20.0f;
	P.FlattenFalloff = 50.0f;
	P.FlattenHeight = 0.35f;

	// Yaricap ICINDE: her zaman tam olarak FlattenHeight (insaat izgarasi duz zemin ister)
	bool bFlat = true;
	for (int32 dy = -20; dy <= 20; ++dy)
	{
		for (int32 dx = -20; dx <= 20; ++dx)
		{
			if (FMath::Sqrt(static_cast<float>(dx * dx + dy * dy)) > P.FlattenRadius) { continue; }
			const float Raw = SampleFBM(static_cast<float>(100 + dx), static_cast<float>(100 + dy), P);
			const float Masked = ApplyFlattenMask(Raw, 100 + dx, 100 + dy, P);
			bFlat = bFlat && FMath::IsNearlyEqual(Masked, P.FlattenHeight, KINDA_SMALL_NUMBER);
		}
	}
	TestTrue(TEXT("yaricap icinde tam duz"), bFlat);

	// Falloff DISINDA: maske hic etki etmemeli
	const int32 FarX = 100 + 200;
	const float RawFar = SampleFBM(static_cast<float>(FarX), 100.0f, P);
	TestEqual(TEXT("falloff disinda maske etkisiz"), ApplyFlattenMask(RawFar, FarX, 100, P), RawFar);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeightmapGecisSurekliligiTest,
	"SurvivalGame.World.HeightmapMath.GecisSurekliligi",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHeightmapGecisSurekliligiTest::RunTest(const FString& Parameters)
{
	using namespace SurvivalHeightmap;

	FHeightmapParams P;
	P.Seed = 11;
	P.FlattenCenter = FIntPoint(0, 0);
	P.FlattenRadius = 20.0f;
	P.FlattenFalloff = 60.0f;
	P.FlattenHeight = 0.35f;

	// Duz bolgeden gurultulu bolgeye gecerken ani sicrama olmamali:
	// komsu ornekler arasindaki fark kucuk kalmali (duvar/ucurum olusmaz)
	float MaxStep = 0.0f;
	float Prev = ApplyFlattenMask(SampleFBM(0.0f, 0.0f, P), 0, 0, P);
	for (int32 X = 1; X <= 100; ++X)
	{
		const float Cur = ApplyFlattenMask(SampleFBM(static_cast<float>(X), 0.0f, P), X, 0, P);
		MaxStep = FMath::Max(MaxStep, FMath::Abs(Cur - Prev));
		Prev = Cur;
	}
	TestTrue(TEXT("komsu ornekler arasi sicrama sinirli"), MaxStep < 0.25f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

- [ ] **Step 2: Testleri derleyip başarısız olduklarını gör**

Editörü kapat, sonra:

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Build/BatchFiles/Build.bat" SurvivalGameEditor Win64 Development -project="C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -waitmutex
```

Beklenen: DERLEME HATASI — `World/HeightmapMath.h` bulunamıyor.

- [ ] **Step 3: `HeightmapMath.h`'i yaz**

`Source/SurvivalGame/Public/World/HeightmapMath.h` oluştur:

```cpp
#pragma once

#include "CoreMinimal.h"

/**
 * Baslangic haritasi arazi uretimi — saf, durum tutmayan yukseklik matematigi.
 * Tum fonksiyonlar yan etkisiz; birim testleri Private/Tests/HeightmapMathTests.cpp.
 *
 * Motor RNG'si KULLANILMAZ (FMath::Rand / FRandomStream yok) — yukseklik yalnizca
 * (X, Y, Seed) uclusunun tam sayi hash'inden turer. Boylece ayni tohum her zaman ayni
 * araziyi verir: uretim yeniden calistirilabilir, sonuc versiyonlanabilir ve test edilebilir.
 *
 * Cikti uint16 heightmap'tir — ALandscapeProxy::Import()'un bekledigi format.
 */
namespace SurvivalHeightmap
{
	/** UE landscape'te bu deger sifir duzlemidir; altindaki degerler cukur, ustundekiler tepe. */
	inline constexpr uint16 HeightMidpoint = 32768;

	/**
	 * Kullanilan uint16 bandinin genisligi. Tam uint16 araligi (65535) Z olcegi 100'de
	 * +/-25 600 UU'ya karsilik gelir ki bu oynanamayacak kadar dik bir arazi verir.
	 * 15000 -> yaklasik +/-58 m tepe-vadi farki: yurunebilir, insa edilebilir, ama okunur
	 * bir siluet. Yukseklik TEK BURADAN kontrol edilir — Landscape aktorunun Z olcegi
	 * varsayilan 100'de birakilir ki iki yerden ayarlanan gizli bir bagimlilik olusmasin.
	 */
	inline constexpr float HeightSpan = 15000.0f;

	struct FHeightmapParams
	{
		int32 Seed        = 1337;
		int32 Octaves     = 5;
		float Frequency   = 0.0025f;  // ornek/quad
		float Lacunarity  = 2.0f;     // oktav basina frekans carpani
		float Persistence = 0.5f;     // oktav basina genlik carpani

		/**
		 * Baslangic vadisi duzlestirmesi. Yonlendirilmis acilis oyuncunun DUZ zeminde
		 * baslamasini gerektirir: UFoundationGrid izgara-yapismali insaat kullanir
		 * (400 UU hucre, GridCoordToWorld duzlem varsayar), egimli zeminde ilk barinak
		 * yerlestirmesi bozulur.
		 */
		FIntPoint FlattenCenter  = FIntPoint(1008, 1008);  // heightmap ornek uzayi (2017/2)
		float     FlattenRadius  = 120.0f;   // tam duz yaricap (quad)
		float     FlattenFalloff = 260.0f;   // duzden gurultuye gecisin bittigi yaricap (quad)
		float     FlattenHeight  = 0.35f;    // normalize yukseklik [0,1] — orta noktanin altinda: cukur vadi
	};

	/** Deterministik tam sayi hash — (X, Y, Seed) -> dagilimi iyi 32-bit deger. */
	inline uint32 HashCoord(int32 X, int32 Y, int32 Seed)
	{
		uint32 H = static_cast<uint32>(X) * 374761393u;
		H += static_cast<uint32>(Y) * 668265263u;
		H += static_cast<uint32>(Seed) * 362437u;
		H = (H ^ (H >> 13)) * 1274126177u;
		return H ^ (H >> 16);
	}

	/** Hash'i [0,1] araligina esler. */
	inline float HashToUnit(uint32 H)
	{
		return static_cast<float>(H) * (1.0f / 4294967295.0f);
	}

	/** Izgara koselerindeki hash degerleri arasinda smoothstep interpolasyonu. */
	inline float ValueNoise2D(float X, float Y, int32 Seed)
	{
		const int32 X0 = FMath::FloorToInt(X);
		const int32 Y0 = FMath::FloorToInt(Y);
		const float FracX = X - static_cast<float>(X0);
		const float FracY = Y - static_cast<float>(Y0);

		// smoothstep: dogrusal interpolasyonun birakacagi izgara artefaktlarini yumusatir
		const float SX = FracX * FracX * (3.0f - 2.0f * FracX);
		const float SY = FracY * FracY * (3.0f - 2.0f * FracY);

		const float V00 = HashToUnit(HashCoord(X0,     Y0,     Seed));
		const float V10 = HashToUnit(HashCoord(X0 + 1, Y0,     Seed));
		const float V01 = HashToUnit(HashCoord(X0,     Y0 + 1, Seed));
		const float V11 = HashToUnit(HashCoord(X0 + 1, Y0 + 1, Seed));

		return FMath::Lerp(FMath::Lerp(V00, V10, SX), FMath::Lerp(V01, V11, SX), SY);
	}

	/** Fraktal Brown hareketi: azalan genlikte artan frekansli oktavlarin toplami, [0,1]'e normalize. */
	inline float SampleFBM(float X, float Y, const FHeightmapParams& P)
	{
		float Freq = P.Frequency;
		float Amp = 1.0f;
		float Sum = 0.0f;
		float Norm = 0.0f;

		for (int32 Octave = 0; Octave < P.Octaves; ++Octave)
		{
			// Her oktav farkli bir tohum turevi kullanir; ayni tohumla ayni izgaraya
			// dusen oktavlar birbirini guclendirip yapay desen olusturmasin.
			Sum += ValueNoise2D(X * Freq, Y * Freq, P.Seed + Octave * 7919) * Amp;
			Norm += Amp;
			Freq *= P.Lacunarity;
			Amp *= P.Persistence;
		}

		return Norm > 0.0f ? FMath::Clamp(Sum / Norm, 0.0f, 1.0f) : 0.0f;
	}

	/**
	 * Baslangic vadisini duzler. FlattenRadius icinde TAM olarak FlattenHeight doner;
	 * FlattenRadius -> FlattenFalloff arasinda smoothstep ile gurultuye gecer (ani duvar
	 * olusmasin); disinda girdiyi aynen doner.
	 */
	inline float ApplyFlattenMask(float InHeight, int32 X, int32 Y, const FHeightmapParams& P)
	{
		const float DX = static_cast<float>(X - P.FlattenCenter.X);
		const float DY = static_cast<float>(Y - P.FlattenCenter.Y);
		const float Dist = FMath::Sqrt(DX * DX + DY * DY);

		if (Dist <= P.FlattenRadius)
		{
			return P.FlattenHeight;
		}
		if (Dist >= P.FlattenFalloff || P.FlattenFalloff <= P.FlattenRadius)
		{
			return InHeight;
		}

		const float T = (Dist - P.FlattenRadius) / (P.FlattenFalloff - P.FlattenRadius);
		const float S = T * T * (3.0f - 2.0f * T);
		return FMath::Lerp(P.FlattenHeight, InHeight, S);
	}

	/**
	 * Tam heightmap uretir — ALandscapeProxy::Import()'un bekledigi duz uint16 dizisi
	 * (satir-oncelikli, Y * SizeX + X). Gecersiz boyutta bos dizi doner (cokmez).
	 */
	inline TArray<uint16> GenerateHeightmap(int32 SizeX, int32 SizeY, const FHeightmapParams& P)
	{
		TArray<uint16> Out;
		if (SizeX <= 0 || SizeY <= 0)
		{
			return Out;
		}

		Out.SetNumUninitialized(SizeX * SizeY);
		for (int32 Y = 0; Y < SizeY; ++Y)
		{
			for (int32 X = 0; X < SizeX; ++X)
			{
				const float Noise = SampleFBM(static_cast<float>(X), static_cast<float>(Y), P);
				const float Shaped = ApplyFlattenMask(Noise, X, Y, P);
				const float Centered = static_cast<float>(HeightMidpoint) + (Shaped - 0.5f) * HeightSpan;
				Out[Y * SizeX + X] = static_cast<uint16>(FMath::Clamp(FMath::RoundToInt(Centered), 0, 65535));
			}
		}
		return Out;
	}
}
```

- [ ] **Step 4: BOM ekle ve derle**

```bash
powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Build/BatchFiles/Build.bat" SurvivalGameEditor Win64 Development -project="C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -waitmutex
```

Beklenen: `Result: Succeeded`

- [ ] **Step 5: Testleri çalıştır ve geçtiklerini doğrula**

```bash
rm -rf TestResults
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -ExecCmds="Automation RunTests SurvivalGame; Quit" -ReportOutputPath="C:/Users/atoly/Downloads/Oyun/TestResults" -nullrhi -unattended -nop4 -nosplash
```

Sonucu `TestResults/index.json`'dan oku. Beklenen: `succeeded=60, failed=0, notRun=0` (55 mevcut + 5 yeni).

- [ ] **Step 6: Commit**

```bash
git add Source/SurvivalGame/Public/World/HeightmapMath.h Source/SurvivalGame/Private/Tests/HeightmapMathTests.cpp
git commit -m "Arazi uretimi: saf yukseklik matematigi (HeightmapMath) + 5 birim testi"
```

---

### Task 2: Editör aracı — `survival_generate_landscape`

**Files:**
- Create: `Source/SurvivalGame/Private/World/LandscapeBuilder.cpp`
- Modify: `Source/SurvivalGame/SurvivalGame.Build.cs`

**Interfaces:**
- Consumes: Task 1'in `SurvivalHeightmap::FHeightmapParams`, `GenerateHeightmap`, `HeightMidpoint`, `HeightSpan`
- Produces: `survival_generate_landscape [Seed]` konsol komutu. Sonraki görevler bunu editörde çağırır; C++ API dışa açmaz.

**Neden bu boyutlar:** Section 63 quad, 2×2 altbölüm → bileşen başına 126 quad. 16×16 = 256 bileşen → 2016 quad → **2017×2017 vertex**. 100 UU/quad ile 201 600 UU ≈ 2.016 km kenar ≈ **4.06 km²**.

- [ ] **Step 1: `Build.cs`'e `Landscape` bağımlılığını ekle**

`Source/SurvivalGame/SurvivalGame.Build.cs` içindeki `PrivateDependencyModuleNames.AddRange(...)` bloğundan SONRA ekle:

```csharp
		// Landscape yalnizca editor-only arazi ureticisi (LandscapeBuilder.cpp) tarafindan
		// kullanilir — ALandscapeProxy::Import() WITH_EDITOR icindedir. Paketlenmis oyun
		// hedefi bu bagimliligi tasimaz; arazi zaten .umap'e gomulu veri olarak gider.
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("Landscape");
		}
```

- [ ] **Step 2: `LandscapeBuilder.cpp`'i yaz**

`Source/SurvivalGame/Private/World/LandscapeBuilder.cpp` oluştur:

```cpp
// Baslangic haritasi arazi ureticisi — editor-only.
//
// ALandscapeProxy::Import() WITH_EDITOR icindedir (LandscapeProxy.h:1418, blok satir 1326'da
// aciliyor), dolayisiyla bu dosyanin TAMAMI editor derlemelerine ozeldir. Arazi editor
// zamaninda BIR KEZ uretilir ve .umap'e gomulur; pakete normal veri olarak gider, runtime
// maliyeti sifirdir.
//
// Cagri sirasi motorun kendi "New Landscape" akisindan alinmistir
// (LandscapeEditorDetailCustomization_NewLandscape.cpp:1180-1245).

#include "SurvivalGame.h"

#if WITH_EDITOR

#include "World/HeightmapMath.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeImportHelper.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// Gecerli UE landscape bolunmesi: bilesen = altbolum x altbolum-quad.
	constexpr int32 QuadsPerSection = 63;
	constexpr int32 SectionsPerComponent = 2;   // 2x2 altbolum
	constexpr int32 ComponentCount = 16;        // 16x16 bilesen
	constexpr int32 QuadsPerComponent = SectionsPerComponent * QuadsPerSection;  // 126
	constexpr int32 LandscapeVerts = ComponentCount * QuadsPerComponent + 1;     // 2017

	void GenerateLandscape(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: gecerli bir World yok"));
			return;
		}

		SurvivalHeightmap::FHeightmapParams Params;
		Params.FlattenCenter = FIntPoint(LandscapeVerts / 2, LandscapeVerts / 2);
		if (Args.Num() > 0)
		{
			Params.Seed = FCString::Atoi(*Args[0]);
		}

		const double StartTime = FPlatformTime::Seconds();

		TArray<uint16> HeightData = SurvivalHeightmap::GenerateHeightmap(LandscapeVerts, LandscapeVerts, Params);
		if (HeightData.Num() != LandscapeVerts * LandscapeVerts)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: heightmap uretilemedi"));
			return;
		}

		// Motorun bekledigi kapsayicilar. Bos FGuid = taban katman (duzenleme katmani yok).
		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));

		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		MaterialLayerDataPerLayers.Add(FGuid(), TArray<FLandscapeImportLayerInfo>());

		// Landscape orijine gore ORTALANIR: baslangic vadisi (duzlestirme merkezi) orijine
		// dusmeli ki icerik koordinatlari okunur kalsin.
		const FVector Scale(100.0, 100.0, 100.0);
		const FVector Offset(
			-ComponentCount * QuadsPerComponent * Scale.X * 0.5,
			-ComponentCount * QuadsPerComponent * Scale.Y * 0.5,
			0.0);

		ALandscape* NewLandscape = World->SpawnActor<ALandscape>(Offset, FRotator::ZeroRotator);
		if (!NewLandscape)
		{
			UE_LOG(LogSurvival, Warning, TEXT("survival_generate_landscape: ALandscape spawn edilemedi"));
			return;
		}

		NewLandscape->SetActorRelativeScale3D(Scale);
		NewLandscape->StaticLightingLOD = FMath::DivideAndRoundUp(
			FMath::CeilLogTwo((LandscapeVerts * LandscapeVerts) / (2048 * 2048) + 1), static_cast<uint32>(2));

		NewLandscape->Import(
			FGuid::NewGuid(),
			0, 0,
			LandscapeVerts - 1, LandscapeVerts - 1,
			SectionsPerComponent, QuadsPerSection,
			HeightDataPerLayers,
			nullptr,
			MaterialLayerDataPerLayers,
			ELandscapeImportAlphamapType::Additive,
			TArrayView<const FLandscapeLayer>());

		ULandscapeInfo* Info = NewLandscape->GetLandscapeInfo();
		if (Info)
		{
			Info->UpdateLayerInfoMap(NewLandscape);
		}

		const double Elapsed = FPlatformTime::Seconds() - StartTime;
		UE_LOG(LogSurvival, Log,
			TEXT("survival_generate_landscape: tohum=%d vertex=%dx%d bilesen=%dx%d kenar=%.0f UU sure=%.2f sn"),
			Params.Seed, LandscapeVerts, LandscapeVerts, ComponentCount, ComponentCount,
			ComponentCount * QuadsPerComponent * Scale.X, Elapsed);
		UE_LOG(LogSurvival, Log,
			TEXT("survival_generate_landscape: harita HENUZ KAYDEDILMEDI — save_assets cagir"));
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdGenerateLandscape(
		TEXT("survival_generate_landscape"),
		TEXT("Baslangic haritasi arazisini uretir: survival_generate_landscape [Tohum=1337]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(&GenerateLandscape));
}

#endif // WITH_EDITOR
```

- [ ] **Step 3: BOM ekle ve derle**

```bash
powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Build/BatchFiles/Build.bat" SurvivalGameEditor Win64 Development -project="C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -waitmutex
```

Beklenen: `Result: Succeeded`. Derleme hatası verirse ilk şüpheli include yolları — `Landscape.h`, `LandscapeProxy.h`, `LandscapeInfo.h` `Landscape` modülünün `Classes/` klasöründedir; hata alırsan `Public/` yollarını dene ve engine kaynağından doğrula.

- [ ] **Step 4: Regresyon testi**

```bash
rm -rf TestResults
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -ExecCmds="Automation RunTests SurvivalGame; Quit" -ReportOutputPath="C:/Users/atoly/Downloads/Oyun/TestResults" -nullrhi -unattended -nop4 -nosplash
```

`TestResults/index.json` → beklenen `succeeded=60, failed=0`.

- [ ] **Step 5: Commit**

```bash
git add Source/SurvivalGame/Private/World/LandscapeBuilder.cpp Source/SurvivalGame/SurvivalGame.Build.cs
git commit -m "Arazi uretimi: survival_generate_landscape editor komutu (2017x2017, ~4 km2)"
```

---

### Task 3: Harita oluşturma + arazi üretimi

**Files:**
- Create: `Content/Maps/BaslangicHaritasi.umap` (MCP ile)

**Interfaces:**
- Consumes: Task 2'nin `survival_generate_landscape` komutu
- Produces: içinde gerçek bir `ALandscape` bulunan, diske kaydedilmiş `BaslangicHaritasi` haritası

- [ ] **Step 1: Editörü aç ve MCP oturumunu hazırla**

```bash
rm -f "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/.mcp-session-id"
```

Editörü başlat, MCP sunucusunun 8000 portunda dinlemesini bekle. Bağlantıyı doğrula:

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Tool "list_toolsets"
```

- [ ] **Step 2: `TestPlayer`'ı kopyalayarak yeni haritayı oluştur**

Kopyalama, hazır ışıklandırma/gökyüzü/`WorldSettings` override'ını korur; sıfırdan harita bunları kaybeder.

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.asset.AssetTools" -Tool "duplicate" -ArgsJson '{"source_asset_path":"/Game/Maps/Tests/TestPlayer","destination_folder_path":"/Game/Maps","destination_asset_name":"BaslangicHaritasi"}'
```

Şema alan adlarını reddederse `describe_toolset` ile gerçek adları oku — tahmin etme.

- [ ] **Step 3: Kaydet ve diskten doğrula**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.asset.AssetTools" -Tool "save_assets" -ArgsJson '{"asset_paths":[]}'
```

```bash
ls -la "C:/Users/atoly/Downloads/Oyun/Content/Maps/BaslangicHaritasi.umap"
```

Beklenen: dosya diskte var. (MCP varlıkları kaydedilene kadar uçucudur.)

- [ ] **Step 4: Haritayı yükle**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.scene.SceneTools" -Tool "load_level" -ArgsJson '{"level_path":"/Game/Maps/BaslangicHaritasi"}'
```

- [ ] **Step 5: Test aktörlerini temizle**

Kopyalanan haritada `TestPlayer`'dan kalan stres/test aktörleri var. `find_actors` ile `AHarvestNode`, `AContainerActor`, `AProductionMachine`, `AStorageNode`, `ANPCCharacter`, `ABuildingBase` örneklerini listele ve sil. `PlayerStart`, ışıklar, gökyüzü, `WorldSettings` ve `NavMeshBoundsVolume` KALSIN.

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.scene.SceneTools" -Tool "find_actors" -ArgsJson '{"actor_type":"/Script/SurvivalGame.HarvestNode"}'
```

- [ ] **Step 6: Araziyi üret**

Editör konsolunda (Window → Output Log → Cmd):

```
survival_generate_landscape 1337
```

Beklenen log: `survival_generate_landscape: tohum=1337 vertex=2017x2017 bilesen=16x16 kenar=201600 UU sure=... sn`

- [ ] **Step 7: Kaydet ve doğrula**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.asset.AssetTools" -Tool "save_assets" -ArgsJson '{"asset_paths":[]}'
```

Haritanın boyutu belirgin şekilde büyümüş olmalı (landscape verisi gömüldü):

```bash
ls -la "C:/Users/atoly/Downloads/Oyun/Content/Maps/BaslangicHaritasi.umap"
```

Ayrıca sahnede landscape gerçekten var mı:

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.scene.SceneTools" -Tool "find_actors" -ArgsJson '{"actor_type":"/Script/Landscape.Landscape"}'
```

- [ ] **Step 8: Commit**

Git LFS `.umap` izliyor; büyük dosya normaldir.

```bash
git add Content/Maps/BaslangicHaritasi.umap
git commit -m "Baslangic haritasi: arazi uretildi (tohum 1337, 2017x2017, ~4 km2)"
```

---

### Task 4: Landscape materyali

**Files:**
- Create: `Content/Materials/M_Landscape.uasset` (MCP ile)

**Interfaces:**
- Consumes: Task 3'ün haritası ve `ALandscape` aktörü
- Produces: `ALandscape::LandscapeMaterial`'a atanmış `M_Landscape`

**Yedek plan:** MCP `MaterialTools` düğüm grafiği düzenlemeyi desteklemiyorsa, tek renkli sabit bir materyal yap ve devam et. Arazi ve oynanış etkilenmez; yalnızca görünüm tekdüze olur. Bu görev Task 5'i BLOKE ETMEZ.

- [ ] **Step 1: Materyal araçlarının yeteneğini oku**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Tool "describe_toolset" -ArgsJson '{"toolset_name":"editor_toolset.toolsets.material.MaterialTools"}'
```

Düğüm ekleme/bağlama araçları var mı, önce buna bak. Yoksa doğrudan yedek plana geç.

- [ ] **Step 2: Materyali oluştur**

`/Game/Materials/M_Landscape`. Hedef grafik (doku YOK, saf matematik):

- **Taban renk**: `Lerp(YesilDuzluk, BeyazZirve, saturate((WorldPositionZ - Z0) / (Z1 - Z0)))`
  `Z0 = -3000`, `Z1 = 4000` (HeightSpan 15000 ve Z ölçeği 100 ile uyumlu bant)
- **Kaya karisimi**: `Lerp(TabanRenk, KayaGri, saturate((1 - VertexNormalWS.Z) * 3))`
  Dik yüzeyler kaya olur, yükseklikten bağımsız.
- Roughness sabit `0.85`, Metallic `0`.

Renkler: Yeşil `(0.16, 0.34, 0.12)`, Kaya `(0.36, 0.35, 0.33)`, Kar `(0.92, 0.94, 0.97)`.

- [ ] **Step 3: Kaydet, materyali landscape'e ata, tekrar kaydet**

`ObjectTools.set_properties` ile `ALandscape` aktörünün `LandscapeMaterial` özelliğini `/Game/Materials/M_Landscape.M_Landscape` yap, sonra `save_assets`.

- [ ] **Step 4: Görsel doğrulama**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "EditorToolset.EditorAppToolset" -Tool "CaptureViewport" -ArgsJson '{}'
```

Ekran görüntüsünde arazi kabartması ve renk ayrımı görünmeli. Görünmüyorsa yedek plana geç, dokümante et, devam et.

- [ ] **Step 5: Commit**

```bash
git add Content/Materials/M_Landscape.uasset Content/Maps/BaslangicHaritasi.umap
git commit -m "Baslangic haritasi: dokusuz yukseklik+egim landscape materyali"
```

---

### Task 5: Başlangıç vadisi içeriği

**Files:**
- Modify: `Content/Maps/BaslangicHaritasi.umap` (MCP ile aktör yerleştirme)

**Interfaces:**
- Consumes: Task 3'ün arazisi (orijin çevresi düz, yarıçap 120 quad = 12 000 UU)
- Produces: oynanabilir yönlendirilmiş açılış

**Tempo tablosu.** Koordinatlar orijin merkezli, Z arazi yüzeyine oturtulur (`SceneTools` dünya trace'i ile yüzey Z'si bulunur).

| Mesafe | Aktör | ID | Adet | Amaç |
|---|---|---|---|---|
| 0 | `PlayerStart` | — | 1 | Düz zemin, inşaat ızgarası çalışır |
| 1 500–2 000 UU | `AHarvestNode` | `Agac` | 4 | İlk döngü: Odun → Balta |
| 1 500–2 000 UU | `AHarvestNode` | `Kaya` | 3 | Taş → Kazma |
| 2 500–4 000 UU | `AHarvestNode` | `MeyveAgaci` | 2 | Yiyecek |
| 2 500–4 000 UU | `AHarvestNode` | `BalikNoktasi` | 1 | Yiyecek (denge geçişi #2'de min 2 verim) |
| 2 500–4 000 UU | `AHarvestNode` | `LifBitkisi` | 3 | İp → ilerleyen tarifler |
| 8 000–12 000 UU | `AHarvestNode` | `DemirDamari` | 2 | **Spawn'dan görünür** — demir-tier alet çekimi |
| 8 000–12 000 UU | `AHarvestNode` | `KomurDamari` | 2 | Eritme yakıtı |
| 20 000–40 000 UU | `AHarvestNode` | `KilYatagi`, `KumYigini`, `BakirDamari`, `TuzYatagi` | 2'şer | Üretim zincirleri (tuğla, cam, külçe) |

- [ ] **Step 1: PlayerStart'ı orijine taşı**

`find_actors` ile `PlayerStart`'ı bul, `ActorTools.set_actor_transform` ile konumunu `(0, 0, ZeminZ + 100)` yap. `ZeminZ`'yi `SceneTools` dünya trace'i ile ölç — tahmin etme.

- [ ] **Step 2: Yakın halka düğümlerini yerleştir (Agac, Kaya)**

`ProgrammaticToolset` ile toplu yerleştir (Sistem #29'da 9 düğümün toplu yerleştirildiği aynı yöntem). Her düğüm için: XY'yi tablodan seç → o XY'de aşağı doğru dünya trace'i at → yüzey Z'sine oturt → `AHarvestNode` spawn et → `NodeID` özelliğini ata.

- [ ] **Step 3: Orta halka düğümlerini yerleştir (MeyveAgaci, BalikNoktasi, LifBitkisi)**

Aynı yöntem, 2 500–4 000 UU halkası.

- [ ] **Step 4: Uzak halka düğümlerini yerleştir (Demir, Komur, Kil, Kum, Bakir, Tuz)**

Aynı yöntem. **Demir ve kömür spawn'dan görünür açıda olmalı** — yerleştirdikten sonra kamerayı PlayerStart'a koyup `CaptureViewport` ile görünürlüğü GÖZLE doğrula. Görünmüyorsa XY'yi ayarla.

- [ ] **Step 5: NavMeshBoundsVolume'ü başlangıç vadisiyle sınırla**

Hacmi orijin merkezli, ~30 000 UU kenarlı bir kutuya ayarla. 4 km²'nin tamamını kapsatma — bake pahalı ve ILERLEME.md'de MCP-güdümlü PIE'de bake kilidi sorunu kayıtlı. Geniş dünyada NPC yok, navmesh de gerekmiyor.

- [ ] **Step 6: Kaydet ve diskten doğrula**

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.asset.AssetTools" -Tool "save_assets" -ArgsJson '{"asset_paths":[]}'
```

```bash
powershell -File "C:/Users/atoly/.claude/skills/unreal-engine-dev/scripts/ue-tool.ps1" -Toolset "editor_toolset.toolsets.scene.SceneTools" -Tool "find_actors" -ArgsJson '{"actor_type":"/Script/SurvivalGame.HarvestNode"}'
```

Beklenen: **25** hasat düğümü (tempo tablosu: 4 Agac + 3 Kaya + 2 MeyveAgaci + 1 BalikNoktasi + 3 LifBitkisi + 2 DemirDamari + 2 KomurDamari + 8 uzak halka = 25). Task 3 Step 5 kopyalanan haritadaki eski düğümleri sildiği için sayı tam olmalı; fazlaysa temizlik eksik kalmış demektir.

- [ ] **Step 7: PIE'de yönlendirilmiş açılışı canlı doğrula**

PIE başlat. Doğrulanacaklar:
1. Oyuncu düz zeminde başlıyor (düşmüyor, arazinin içinde sıkışmıyor)
2. `E` ile ağaç toplanıyor, `craft_start Balta_Tarifi` çalışıyor
3. `build_place Barinak` düz zeminde başarılı
4. Demir damarı spawn noktasından görülebiliyor

- [ ] **Step 8: Commit**

```bash
git add Content/Maps/BaslangicHaritasi.umap
git commit -m "Baslangic haritasi: yonlendirilmis acilis icerigi (25 hasat dugumu, tempo tablosu)"
```

---

### Task 6: Geniş dünya seyrek dağılımı

**Files:**
- Modify: `Source/SurvivalGame/Private/World/LandscapeBuilder.cpp`

**Interfaces:**
- Consumes: Task 1'in `SurvivalHeightmap::HashCoord` (aynı deterministik hash), Task 5'in yerleşim deseni
- Produces: `survival_populate_world [Tohum] [Yogunluk]` konsol komutu

**Neden komut, elle yerleştirme değil:** 4 km²'yi elle doldurmak binlerce MCP çağrısı demek; deterministik bir komut hem yeniden üretilebilir hem versiyonlanabilir.

- [ ] **Step 1: `survival_populate_world` komutunu ekle**

`LandscapeBuilder.cpp`'in `#if WITH_EDITOR` bloğu içine, `GenerateLandscape`'ten sonra ekle. Mantık:

- Haritayı `Yogunluk` UU aralıklı bir ızgaraya böl (varsayılan 25 000 UU)
- Her ızgara hücresi için `HashCoord(CellX, CellY, Seed)` ile deterministik olarak: düğüm konulacak mı, hangi tip, hücre içinde hangi ofsette
- **Başlangıç vadisini ATLA** (orijinden 15 000 UU içindeki hücreler — Task 5 orayı elle tasarladı, üzerine yazma)
- Her konumda aşağı dünya trace'i ile yüzey Z'sini bul, `AHarvestNode` spawn et, `NodeID` ata
- Kaç düğüm yerleştiğini `LogSurvival` ile raporla

Tip dağılımı (hash'in mod'una göre): `Agac` %30, `Kaya` %25, `LifBitkisi` %10, `MeyveAgaci` %10, `DemirDamari` %8, `KomurDamari` %7, `BakirDamari` %4, `KilYatagi` %3, `KumYigini` %2, `TuzYatagi` %1.

- [ ] **Step 2: BOM ekle, derle, regresyon testi**

```bash
powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Build/BatchFiles/Build.bat" SurvivalGameEditor Win64 Development -project="C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -waitmutex
```

```bash
rm -rf TestResults
```

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -ExecCmds="Automation RunTests SurvivalGame; Quit" -ReportOutputPath="C:/Users/atoly/Downloads/Oyun/TestResults" -nullrhi -unattended -nop4 -nosplash
```

`index.json` → `succeeded=60, failed=0`.

- [ ] **Step 3: Haritayı doldur**

Editörde `BaslangicHaritasi` yüklüyken konsolda:

```
survival_populate_world 1337 25000
```

Sonra `save_assets`, sonra `find_actors` ile toplam düğüm sayısını doğrula.

- [ ] **Step 4: Başlangıç vadisinin bozulmadığını doğrula**

Task 5'in 25 elle yerleştirilmiş düğümü hâlâ yerinde mi, üzerine yazılmış mı? Orijin çevresinde `find_actors` sonuçlarını kontrol et.

- [ ] **Step 5: Commit**

```bash
git add Source/SurvivalGame/Private/World/LandscapeBuilder.cpp Content/Maps/BaslangicHaritasi.umap
git commit -m "Baslangic haritasi: geniz dunya seyrek dagilimi (deterministik, tohum 1337)"
```

---

### Task 7: Haritayı devreye al + doğrula + ölç

**Files:**
- Modify: `Config/DefaultEngine.ini`
- Modify: `Docs/ILERLEME.md`, `Docs/YOL_HARITASI.md`

**Interfaces:**
- Consumes: Task 3–6'nın tamamlanmış haritası
- Produces: yeni haritayla paketlenmiş, doğrulanmış ve ölçülmüş Alpha build

- [ ] **Step 1: Varsayılan haritayı değiştir**

`Config/DefaultEngine.ini`:

```ini
GameDefaultMap=/Game/Maps/BaslangicHaritasi.BaslangicHaritasi
EditorStartupMap=/Game/Maps/BaslangicHaritasi.BaslangicHaritasi
```

`Maps/Tests/TestPlayer` SİLİNMEZ — otomasyon testleri ve mevcut PIE akışları ona bağlı.

- [ ] **Step 2: Editörü kapat ve paketle**

Editör açıkken `Config/*.ini` değişikliği çalışan oturuma yansımaz; ayrıca UAT modül DLL kilidine takılır.

```bash
"C:/Users/atoly/Desktop/Unreal/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project="C:/Users/atoly/Downloads/Oyun/SurvivalGame.uproject" -noP4 -platform=Win64 -clientconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory="C:/Users/atoly/Downloads/Oyun/Packaged" -nocompileeditor -utf8output
```

Sonucu UAT'ın kendi log'undan oku (stdout redirect'i boş kalabilir):
`C:\Users\atoly\AppData\Roaming\Unreal Engine\AutomationTool\Logs\...\Log.txt` → `BUILD SUCCESSFUL` ve `ExitCode=0` ara.

- [ ] **Step 3: Paketlenmiş build'i doğrula**

Paketlenmiş oyun log/CSV'yi exe'nin YANINA yazar (`Packaged/Windows/SurvivalGame/Saved/`), `%LOCALAPPDATA%`'ya değil.

```bash
"C:/Users/atoly/Downloads/Oyun/Packaged/Windows/SurvivalGame.exe" -windowed -resx=1600 -resy=900 -ExecCmds="r.DynamicRes.OperationMode 0,t.MaxFPS 0" -csvCaptureFrames=900 -unattended -nosplash
```

Log'da doğrula (`Packaged/Windows/SurvivalGame/Saved/Logs/SurvivalGame.log`):
- `0` adet `Fatal error` / `Critical error` / `Assertion failed`
- `HarvestNodeDatabase hazir: 11 dugum tanimi yuklendi`
- Yüklenen haritanın `BaslangicHaritasi` olduğu

- [ ] **Step 4: Performansı ölç ve kaydet**

CSV'yi `Packaged/Windows/SurvivalGame/Saved/Profiling/CSV/` altından oku, son 500 karenin ortalamasını al: `FrameTime`, `GameThreadTime`, `GPUTime`, `PhysicalUsedMB`.

**World Partition riski burada ölçülür:** 201 600 UU harita / 6400 UU hücre ≈ 961 hücre. `GameThreadTime` önceki 2.86 ms'e göre belirgin arttıysa (>6 ms), `WorldPartitionHelper::RebuildCellRegistry`'nin kare-bölümlemesiz `TActorIterator` taraması suçludur (Sistem #25'in açık notu) — kare-bölümleme ekle. **Ölçmeden optimize etme.**

- [ ] **Step 5: Dokümantasyonu güncelle**

`Docs/ILERLEME.md`: günlük tablosuna satır ekle — üretilen arazi (tohum, boyut), yerleştirilen düğüm sayısı, paketlenmiş ölçümler, World Partition sonucu.

`Docs/YOL_HARITASI.md`: Alpha kilometre taşı satırındaki "tek cook edilen haritanın `Maps/Tests/TestPlayer` olması" açık konusunu kapat.

- [ ] **Step 6: Commit**

```bash
git add Config/DefaultEngine.ini Docs/ILERLEME.md Docs/YOL_HARITASI.md
git commit -m "Baslangic haritasi devreye alindi: varsayilan harita degisti, paketlenmis build dogrulandi"
```

---

## Öz-inceleme notları

**Spec kapsamı.** Spec'in her bölümü bir göreve eşleniyor: Parça 1a→Task 1, 1b/1c→Task 2, 1d→Task 1 Step 1, Parça 2→Task 4, Parça 3a→Task 5, 3b→Task 6, 3c→Task 3 + Task 7. Riskler: navmesh→Task 5 Step 5, World Partition→Task 7 Step 4, materyal yedek planı→Task 4 başlığı, landscape bileşen sayısı→Task 7 Step 2 (cook süresi log'dan okunur).

**Tip tutarlılığı.** `FHeightmapParams` alan adları Task 1'de tanımlandı, Task 2 ve Task 6 aynı adları kullanıyor (`Seed`, `FlattenCenter`). `HashCoord` imzası Task 1'de sabitlendi, Task 6 aynısını çağırıyor. `survival_generate_landscape` ve `survival_populate_world` komut adları geçtikleri her yerde aynı.

**Bilinçli belirsizlik bırakılan tek yer:** Task 4'ün MCP `MaterialTools` yetenekleri. Bu, önceden bilinemez (şema çalışma zamanında okunur) — bu yüzden görevin başına açık bir yedek plan ve "Task 5'i bloke etmez" notu konuldu.
