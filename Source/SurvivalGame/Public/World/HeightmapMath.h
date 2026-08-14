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
	 * +/-25 600 UU'ya karsilik gelir. 60000 -> tepe +23 437 UU, vadi (FlattenHeight=0.35)
	 * -7031 UU: ~300 m dikey aralik, GERCEK ucurumlar (vadi duvarlari ~45 derece, gurultu
	 * tepeleri sarp). Tasma yok: tepe fBm=1.0 -> 32768+30000=62768 < 65535; taban fBm=0 ->
	 * 32768-30000=2768 > 0. Yukseklik TEK BURADAN kontrol edilir — Landscape aktorunun Z
	 * olcegi varsayilan 100'de birakilir ki iki yerden ayarlanan gizli bir bagimlilik olusmasin.
	 * NOT: Arazi bu degerle yeniden uretilirse mevcut icerigin Z'si bozulur —
	 * survival_reground_nodes komutu tum dugumleri + PlayerStart'i yeni yuzeye oturtur.
	 */
	inline constexpr float HeightSpan = 60000.0f;

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
