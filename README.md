# Survival Settlement (çalışma adı)

Üçüncü şahıs survival: aşırı hava koşulları + kendi kendine yeten endüstriyel yerleşim. Unreal Engine 5.8, C++ + Blueprint. Hedef: ticari Steam yayını.

## Mevcut durum (2026-07-23)

| | |
|---|---|
| ✅ Sistem 1 — Zaman | `TimeKeeper`/`DayNightCycle`/`GameClock`; 3/3 otomasyon testi; PIE'de güneş gerçek zamanlı dönüyor |
| ✅ Sistem 2 — Oyuncu Kontrolü | Enhanced Input + üçüncü şahıs kamera; 4/4 test; PIE'de WASD/fare/zıplama/zoom canlı doğrulandı |
| ⏭️ Sırada | Sistem 3 — Etkileşim Çerçevesi ([ILERLEME.md](Docs/ILERLEME.md)) |

**Oyunu denemek için:** editörü aç (başlangıç haritası `Maps/Tests/TestPlayer` otomatik yüklenir) → **Play** → viewport'a tıkla → WASD yürüyüş, fare kamera, Space zıplama, tekerlek zoom (150–600 kelepçeli), Shift+F1 fare bırakma. Karakter şimdilik görünmez kapsül (mesh içerik fazında gelecek).

**Otomasyon testleri:**
```bash
powershell -Command "& 'C:\Users\atoly\Desktop\Unreal\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'C:\Users\atoly\Downloads\Oyun\SurvivalGame.uproject' -ExecCmds='Automation RunTests SurvivalGame; Quit' -TestExit='Automation Test Queue Empty' -ReportOutputPath='C:\Users\atoly\Downloads\Oyun\TestResults' -nullrhi -unattended -nop4 -nosplash"
```
Sonuç `TestResults\index.json` içinde (`succeeded`/`failed`) — stdout'a bakma.

## Hızlı başlangıç

1. **Gereksinimler:**
   - UE 5.8.x
   - Visual Studio 2026, **Desktop development with C++** workload
   - **MSVC v145 — spesifik olarak 14.50, 14.51 DEĞİL.** VS2026'nın varsayılan/güncel toolset'i (14.51+) ile UE 5.8 derlenmez: `<hash_map>` kaldırılmış, `nvtesslib` (3rd party) derleme hatası verir; Epic'in düzeltmesi 5.8 release dalına backport edilmedi. Individual Components'ta **"x64/x86 için MSVC Derleme Araçları v14.50"**'yi ayrıca işaretle (14.51 zaten workload'la geliyor, kaldırma — sadece 14.50'yi ekle).
   - **.NET Framework 4.6.2 (veya üzeri) targeting pack** — Individual Components'ta ".NET Framework" ara, ekle. Yoksa `SwarmInterface` modülü NetFxSDK bulamayıp derlemeyi `RulesError` ile durdurur.
   - Windows 11 SDK (herhangi bir güncel sürüm, örn. 10.0.26100.x) — sorun çıkarmadı.
   - Git + Git LFS
2. MSVC 14.50 kurulduktan sonra UBT'yi ona **sabitle** (yoksa "en yeni" 14.51'i seçer ve build 1. maddedeki hatayla düşer): `%APPDATA%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml` içine
   ```xml
   <?xml version="1.0" encoding="utf-8" ?>
   <Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
     <WindowsPlatform>
       <CompilerVersion>14.50.35717</CompilerVersion>
     </WindowsPlatform>
   </Configuration>
   ```
   Tam sürüm numarası `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\` altındaki klasör adından alınır — makineden makineye değişebilir, kontrol et.
3. `SurvivalGame.uproject` → sağ tık → **Generate Visual Studio project files**.
   - UE sürümün farklıysa `.uproject` içindeki `EngineAssociation` değerini düzelt.
4. `SurvivalGame.sln` aç → `SurvivalGameEditor` hedefini `Development Editor | Win64` ile derle.
5. `.uproject` çift tık → editor açılır.

**Doğrulanmış ortam** (2026-07-23): UE 5.8.0, MSVC 14.50.35717, Windows SDK 10.0.26100.0, NetFxSDK 4.6.2 — temiz derleme, `SurvivalGame.*` otomasyon testleri 4/4 PASSED, PIE canlı oynanış doğrulaması (hareket + kamera + gün/gece).

## Editör otomasyonu (MCP köprüsü)

Proje, Unreal Editor'ü AI ajanlarına açan Epic'in deneysel **ModelContextProtocol** + **EditorToolset** plugin'lerini kullanır (`.uproject`'te etkin):

- Kök dizindeki [.mcp.json](.mcp.json), editördeki MCP sunucusunu (`http://127.0.0.1:8000/mcp`) Claude Code'a tanıtır.
- Sunucu editörle birlikte otomatik başlar (Editor Preferences → Model Context Protocol → **Auto Start Server** açık). Elle: Output Log → Cmd → `ModelContextProtocol.StartServer`.
- Bu köprüyle varlık oluşturma, özellik atama, harita düzenleme, PIE başlatma/durdurma ve log okuma programatik yapılabilir — Sistem 2'nin tüm editör varlıkları (`IA_*`, `IMC_Default`, `BP_*`, `TestPlayer`) böyle üretildi.
- Bilinen tuzaklar ve araç kısıtları: [YOL_HARITASI.md](Docs/YOL_HARITASI.md) ADR tablosu (2026-07-23 kayıtları).

## Depo haritası

| Yol | İçerik |
|-----|--------|
| `Source/SurvivalGame/` | C++ modülü — sistem başına klasör (Public/Private ayna yapısı) |
| `Content/` | Blueprint, harita, veri tabloları, asset'ler |
| `Config/` | Engine/Game/Input ayarları (Enhanced Input, trace kanalları) |
| `Docs/MIMARI.md` | 28 sistemin tam mimari tasarımı |
| `Docs/ILERLEME.md` | **Canlı ilerleme takibi** — hangi sistem ne durumda |
| `Docs/YOL_HARITASI.md` | 12→16 haftalık plan + karar kaydı |
| `Tests/` | Entegrasyon/birim/fonksiyonel test varlıkları |
| `Tools/` | DataTable üretici, config doğrulayıcı (CLI) |

## Geliştirme akışı

1. `Docs/ILERLEME.md`'de sıradaki ⬜ sistemi bul.
2. `Docs/MIMARI.md`'de o sistemin bölümünü oku (sınıflar + veri akışı + testler).
3. Kodu `Source/SurvivalGame/<Public|Private>/<Sistem>/` altına yaz.
4. Testleri geçir, `ILERLEME.md` durumunu güncelle, commit et.

## Sürüm kontrolü

```bash
git lfs install
git lfs track "*.uasset" "*.umap"   # ilk asset commit'inden ÖNCE
```

`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` gitignore'dadır — asla commit etmeyin.
