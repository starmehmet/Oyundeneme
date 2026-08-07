# Gerçek Başlangıç Haritası — Tasarım Belgesi

**Tarih:** 2026-08-07
**Durum:** Onaylandı (operatör), uygulama planı bekliyor
**İlgili:** `Docs/MIMARI.md` (Sistem #18 Dünya Bölümlendirme), `Docs/YOL_HARITASI.md` (Alpha kilometre taşı)

## Amaç

Alpha dağıtımı için `Content/Maps/Tests/TestPlayer` yerine geçecek, tasarlanmış bir başlangıç haritası üretmek. Alpha build 2026-08-07'de paketlendi ve çalıştığı doğrulandı, ancak cook edilen tek harita bir test sahnesiydi — oynanabilir ama tasarlanmış bir açılış deneyimi değil. Bu belge o boşluğu kapatır.

## Operatör kararları

Bu tasarım dört soruya verilen yanıtlar üzerine kurulu:

| Soru | Karar |
|---|---|
| "Gerçek" ne demek? | **Oynanış gerçek olsun** — görseller placeholder kalır |
| Ölçek | **Orta, ~4 km²** |
| İlk 15 dakika | **Yönlendirilmiş açılış** — sistemleri sırayla öğreten, temposu tasarlanmış |
| Arazi kabartması | **Olsun** — çözüm yolu araştırılıp seçildi (aşağıda) |

## Kısıtlar (doğrulanmış)

1. **Projede sıfır görsel varlık var.** 15 `uasset`'in tamamı Blueprint, Input ve DataTable; hiç static mesh, materyal, doku, iskelet mesh veya animasyon yok. Her şey Engine primitifi + varsayılan materyal olarak çizilir. Bu yüzden "gerçek" oynanış anlamına gelir, görsellik değil.
2. **`ALandscapeProxy::Import()` editör-only.** `LandscapeProxy.h:1418`, `#if WITH_EDITOR` bloğu içinde (blok satır 1326'da açılıyor). Arazi editör zamanında bir kez üretilir, `.umap`'e gömülür, pakete normal veri olarak gider — runtime maliyeti sıfırdır.
3. **`-ExecCmds` virgülle ayrılır**, noktalı virgülle değil (`ParseExecCommands.cpp:28`). Otomatik doğrulama script'leri bunu gözetmeli (bkz. ADR 2026-08-07).
4. **`profile_spawn_stress` render benchmark'ı değil** — aktörleri `UnloadRadius`'un ötesine doğurur (ADR 2026-08-07). Harita performansı ölçülürken kamera içerik alanında olmalı.

## Mimari

İş üç bağımsız parçaya bölünür. Parça 1 tek başına da değerli çıktı verir (gerçek arazi), 2 ve 3 üzerine gelir.

```
Parça 1: Arazi          HeightmapMath.h (saf)  →  LandscapeBuilder.cpp (editör aracı)  →  ALandscape
Parça 2: Materyal       M_Landscape (MCP MaterialTools, dokusuz, yükseklik+eğim karışımı)
Parça 3: İçerik         Başlangıç vadisi (elle tasarlanmış)  +  geniş dünya (tohumdan deterministik)
```

---

## Parça 1 — Arazi üretimi

### 1a. Saf matematik katmanı

**Dosya:** `Source/SurvivalGame/Public/World/HeightmapMath.h`
**Ad alanı:** `SurvivalHeightmap`

Projenin mevcut saf-fonksiyon desenini birebir tekrarlar (`TimeMath`, `SnowMath`, `WorldPartitionMath`, `HarvestMath`). Motor RNG'si kullanılmaz — tohumlanmış tam sayı hash'i ile tamamen deterministik.

```cpp
struct FHeightmapParams
{
    int32 Seed          = 1337;
    int32 Octaves       = 5;
    float Frequency     = 0.0025f;   // örnek/quad
    float Amplitude     = 1.0f;      // normalize edilmiş [0,1] çıktı ölçeği
    float Lacunarity    = 2.0f;      // oktav başına frekans çarpanı
    float Persistence   = 0.5f;      // oktav başına genlik çarpanı

    // Başlangıç vadisi düzleştirmesi
    FIntPoint FlattenCenter   = FIntPoint(1008, 1008);  // heightmap örnek uzayı
    float     FlattenRadius   = 120.0f;   // tam düz yarıçap (quad)
    float     FlattenFalloff  = 260.0f;   // düzden gürültüye geçiş sonu (quad)
    float     FlattenHeight   = 0.35f;    // normalize yükseklik [0,1]
};
```

Fonksiyonlar:

- `uint32 HashCoord(int32 X, int32 Y, int32 Seed)` — deterministik tam sayı hash
- `float ValueNoise2D(float X, float Y, int32 Seed)` — hash'lenmiş köşe değerleri arasında smoothstep interpolasyon
- `float SampleFBM(float X, float Y, const FHeightmapParams&)` — oktavların toplamı, `[0,1]`'e normalize
- `float ApplyFlattenMask(float InHeight, int32 X, int32 Y, const FHeightmapParams&)` — `FlattenRadius` içinde `FlattenHeight` döner, `FlattenRadius`→`FlattenFalloff` arasında smoothstep ile gürültüye geçer, dışında `InHeight`'i aynen döner
- `TArray<uint16> GenerateHeightmap(int32 SizeX, int32 SizeY, const FHeightmapParams&)` — `Import()`'un beklediği düz `uint16` dizisi; `[0,1]` → `[0,65535]` ölçeklenir

**Düzleştirme neden zorunlu:** yönlendirilmiş açılış oyuncunun düz zeminde başlamasını gerektirir. `UFoundationGrid` ızgara-yapışmalı inşaat kullanır (400 UU hücre, `GridCoordToWorld` düzlem varsayar); eğimli zeminde ilk barınak yerleştirmesi bozulur.

### 1b. Editör aracı

**Dosya:** `Source/SurvivalGame/Private/World/LandscapeBuilder.cpp`
**Tamamı `#if WITH_EDITOR` içinde.** Runtime'a hiçbir şey sızmaz.

Konsol komutu: `survival_generate_landscape [Seed]`

Akış:
1. `FHeightmapParams` kur (varsayılan + isteğe bağlı seed argümanı)
2. `GenerateHeightmap(2017, 2017, Params)` çağır
3. `ALandscape` spawn et, `SetActorScale3D` ile Z ölçeğini ayarla
4. `ALandscapeProxy::Import(...)` ile yükseklik verisini yükle
5. Sonucu `LogSurvival` ile raporla (üretilen bileşen sayısı, min/max yükseklik, süre)

**Build.cs:** `Landscape` modülü eklenir. Editör-only kullanım olduğundan `Target.bBuildEditor` koşuluyla `PrivateDependencyModuleNames`'e eklenir; paketlenmiş oyun hedefi bu bağımlılığı taşımaz.

### 1c. Ölçek matematiği

UE landscape geçerli bölünme kuralları (bileşen = altbölüm × altbölüm-quad):

| Değer | Seçim |
|---|---|
| Section size | 63 quad |
| Subsections | 2 × 2 |
| Bileşen başına quad | 126 |
| Bileşen sayısı | 16 × 16 = 256 |
| Toplam quad | 2016 × 2016 |
| **Vertex** | **2017 × 2017** |
| Quad boyutu | 100 UU |
| **Dünya boyutu** | **201 600 UU ≈ 2.016 km** kenar → **~4.06 km²** |

Landscape orijine göre ortalanır: `-100 800` … `+100 800` UU. Başlangıç vadisi orijinde.

**Yükseklik ölçeği.** UE landscape'te `uint16` yükseklik değeri orta nokta 32768'i sıfır düzlemi kabul eder; Z ölçeği 100 iken tam `uint16` aralığı ±25 600 UU'ya karşılık gelir. Hedeflenen kabartma **~60 m** (6 000 UU) tepe-vadi farkıdır — yürünebilir, inşa edilebilir, ama okunur bir siluet veren ölçek. Bunun için:

- `GenerateHeightmap` normalize `[0,1]` çıktıyı 32768 merkezli dar bir banda eşler, tam `uint16` aralığına değil: `Value = 32768 + (Normalized - 0.5) * HeightSpan`, `HeightSpan = 15 000` (≈ ±58 m, Z ölçeği 100'de)
- `FlattenHeight = 0.35f` bu bandın orta noktasının biraz altına düşer, yani başlangıç bölgesi çevresine göre **çukur bir vadi** olur — kasıtlı: vadi hem rüzgârdan korunmuş hissi verir hem de çevredeki sırtlar doğal yön bulma noktası olur.
- Landscape aktörünün Z ölçeği varsayılan 100'de bırakılır; yükseklik aralığı `HeightSpan` ile kontrol edilir, ölçekle değil (tek kontrol noktası, iki yerden ayarlanan gizli bağımlılık olmaz).

### 1d. Testler

**Dosya:** `Source/SurvivalGame/Private/Tests/HeightmapMathTest.cpp`
`EAutomationTestFlags_ApplicationContextMask` (serbest sabit) kullanılır — üye biçimi UE 5.8'de yok.

| Test | Doğruladığı |
|---|---|
| `Determinizm` | Aynı tohum + aynı koordinat → bit-birebir aynı çıktı, iki ayrı çağrıda |
| `TohumAyrimi` | Farklı tohum → belirgin farklı arazi (örnek kümesinde ortalama mutlak fark eşiği) |
| `DegerAraligi` | `GenerateHeightmap` çıktısının tamamı `[0, 65535]` içinde, taşma/sarma yok |
| `DuzlestirmeMaskesi` | `FlattenRadius` içindeki tüm örnekler birbirine eşit (tolerans içinde); `FlattenFalloff` dışında maske etkisiz |
| `GecisSurekliligi` | Düz bölge ile gürültülü bölge arasında ani sıçrama yok (komşu örnek farkı eşiği) |

---

## Parça 2 — Landscape materyali

**Varlık:** `Content/Materials/M_Landscape` (MCP `MaterialTools` ile oluşturulur)

Doku kullanmaz. İki girdiden renk karışımı:

- **Yükseklik** (world position Z) — alçak → yeşil düzlük, yüksek → beyaz zirve
- **Eğim** (vertex normal Z) — dik yüzeyler → kaya grisi, yükseklikten bağımsız

Karışım sırası: önce yükseklikten taban renk (`Lerp(yeşil, beyaz, saturate((Z - Z0) / Z1))`), sonra eğime göre kaya rengiyle üste karıştır (`Lerp(taban, gri, 1 - saturate(NormalZ ölçekli))`). Roughness sabit, Metallic 0.

**Risk ve yedek plan:** MCP `MaterialTools`'un düğüm grafiği düzenleme yeteneği yetersiz çıkarsa yedek, tek renkli sabit bir materyaldir. Arazi yine çalışır; yalnızca daha tekdüze görünür. Bu, Parça 1 veya 3'ü bloke etmez.

---

## Parça 3 — İçerik yerleşimi

### 3a. Başlangıç vadisi (elle tasarlanmış)

Yönlendirilmiş açılışın temposu. Koordinatlar orijin merkezli, düzleştirilmiş bölge içinde (yarıçap ≈ 12 000 UU).

| Mesafe | İçerik | Amaç |
|---|---|---|
| 0 | `PlayerStart` | Düz zemin, inşaat ızgarası çalışır |
| 15–20 m | 3–4 `Agac`, 2–3 `Kaya` | İlk döngü: topla → Balta + Kazma zanaatı |
| 25–40 m | 1 `MeyveAgaci`, 1 `BalikNoktasi`, `LifBitkisi` kümesi | Yiyecek + İp (ilk tüketim/zanaat zinciri) |
| 80–120 m | `DemirDamari`, `KomurDamari` — **spawn'dan görünür** | Bir sonraki hedefi gösteren çekim; demir-tier alet yolu (denge geçişi #2) |
| 200–400 m | `KilYatagi`, `KumYigini`, `BakirDamari`, `TuzYatagi` | Üretim zincirleri (tuğla, cam, külçe) |

Yerleştirme MCP `SceneTools` + `ProgrammaticToolset` ile toplu yapılır (Sistem #29'da 9 düğümün toplu yerleştirildiği aynı yöntem).

### 3b. Geniş dünya (tohumdan deterministik)

4 km²'nin geri kalanı `survival_generate_landscape` ile **aynı tohumdan** seyrek doldurulur: aynı düğüm tipleri, yoğunluk parametresiyle kontrol edilir, elle yerleştirme yok. Böylece yeniden üretilebilir ve versiyonlanabilir kalır.

Yön bulma noktaları görsel varlık gerektirmez — heightmap üreticisinin doğal olarak ürettiği belirgin tepe ve sırtlar bu işi görür. Düzleştirilmiş vadi, çevresindeki kabartmaya karşı zaten okunur bir "yuva" oluşturur.

### 3c. Harita varlığı

Yeni harita: `Content/Maps/BaslangicHaritasi.umap` (test haritası `Maps/Tests/TestPlayer` yerinde kalır, silinmez — otomasyon testleri ve mevcut PIE akışları ona bağlı).

Paketleme için `DefaultEngine.ini`'de `GameDefaultMap` yeni haritaya çevrilir. `EditorStartupMap` de aynı şekilde.

---

## Riskler ve azaltma

| Risk | Etki | Azaltma |
|---|---|---|
| **NavMesh** — 4 km²'lik bake pahalı; MCP-güdümlü PIE'de bake kilidi sorunu kayıtlı (ILERLEME.md Sistem #15) | NPC'ler hareket edemez | `NavMeshBoundsVolume` yalnızca başlangıç vadisini kapsar. Geniş dünyada NPC yok, dolayısıyla navmesh de gerekmiyor |
| **World Partition ölçeği** — 201 600 UU / 6400 UU hücre ≈ 961 hücre; `RebuildCellRegistry` kare-bölümlemesiz `TActorIterator` taraması yapıyor (Sistem #25'te "ileride gözden geçirilmeli" diye kayıtlı) | Kare süresi düşer | Harita hazır olunca standalone + CSV profiler ile ölçülür; eşik aşılırsa kare-bölümleme eklenir. Ölçmeden optimize edilmez |
| **MCP MaterialTools yetersizliği** | Materyal tekdüze | Sabit renkli yedek materyal; arazi ve oynanış etkilenmez |
| **Landscape bileşen sayısı** (256) | Editör/cook süresi artar | Standart UE "büyük harita" ölçeğinde; cook süresi ölçülüp kaydedilir |

## Kapsam dışı (bilinçli)

- **Görsel varlıklar** — mesh, doku, karakter modeli getirilmez. Operatör kararı: oynanış gerçek, görsel placeholder.
- **Biyomlar** — tek arazi tipi. Kar sistemi (Sistem #14) zaten global; biyom başına farklı hava ayrı bir tasarım işi.
- **Su** — göl/nehir yok. `BalikNoktasi` düğümü su gerektirmiyor (statik hasat düğümü).
- **Mağara/iç mekân** — yok.
- **Test haritasının silinmesi** — `TestPlayer` korunur.

## Başarı ölçütü

1. `survival_generate_landscape` çalışır, `ALandscape` üretilir, harita kaydedilir
2. `HeightmapMath` testleri geçer, mevcut 55 test regresyonsuz kalır
3. Paketlenmiş build yeni haritayla açılır, oyuncu düz zeminde başlar
4. Başlangıç vadisinde balta+kazma zanaatı ilk 15 dakikada tamamlanabilir, demir damarı spawn'dan görünür
5. Standalone + CSV profiler ile kare süresi ölçülür ve kaydedilir (hedef: iş yükü < 16 ms)
