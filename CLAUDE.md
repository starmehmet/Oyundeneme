# CLAUDE.md — SurvivalGame (UE5)

Bu dosya bu depoda çalışan Claude Code oturumları için proje talimatlarıdır.
**Bu proje bir Unreal Engine 5 oyunudur** — üst klasördeki (Downloads/CLAUDE.md) veteriner chatbot talimatları BU PROJEYE UYGULANMAZ.

## Proje nedir

Üçüncü şahıs survival: aşırı hava koşullarında hayatta kalma + kendi kendine yeten endüstriyel yerleşim kurma. Ticari Steam hedefi. UE 5.8 (VS 2026 / MSVC **14.50** — 14.51 DEĞİL, ayrıntı: README kurulum bölümü), C++ (çekirdek sistemler) + Blueprint (içerik/görsellik). Tek runtime modül: `SurvivalGame`.

## Kritik dosyalar

- `Docs/MIMARI.md` — 28 sistemin tam mimarisi (sınıflar, veri akışı, ölçek riskleri). **Yeni sistem yazmadan önce ilgili bölümü oku.**
- `Docs/ILERLEME.md` — sistem-başına durum tablosu. **Her çalışma seansı sonunda güncelle** (durum işareti + günlük satırı).
- `Docs/YOL_HARITASI.md` — haftalık plan + karar kaydı (ADR). Önemli teknik kararları ADR tablosuna ekle.

## Mimari kurallar (MIMARI.md'den özet — çelişkide MIMARI.md kazanır)

- **Deterministik simülasyon çekirdeği:** Üretim, lojistik, kaynak, zanaat mantığı saf/test-edilebilir C++ sınıflarında (subsystem'ler). Actor'lar yalnızca görsellik + etkileşim kabuğu.
- **Veri tabanlı içerik:** Denge değerleri asla koda gömülmez — `Content/Data/DataTables/` (DT_Items, DT_Recipes, DT_Machines, DT_Structures) ve `DataAssets/` kullanılır.
- **Singleton = UGameInstanceSubsystem.** Elle yönetilen static manager yazma.
- **Sistemler arası iletişim = delegate broadcast.** Doğrudan bağımlılık (include zinciri) kurma; abone ol.
- **Tick disiplini:** Yeni her sürekli-çalışan sistem frame bölümlemeli (ör. 500 makine / 60 frame). Sınırsız per-frame O(n) döngü ekleme.
- **Trace kanalları sabit:** `ECC_GameTraceChannel1` = Interaction, `ECC_GameTraceChannel2` = ConstructionPlacement. Yenisini eklersen `Config/DefaultEngine.ini` + buraya yaz.
- **Log:** `LogSurvival*` kategorilerini kullan (`SurvivalGame.h`). `UE_LOG(LogTemp, ...)` yasak.
- **Kaynak dosya kodlaması:** Tüm `.h/.cpp/.cs` dosyaları **UTF-8 with BOM** olmalı — BOM'suz UTF-8'i MSVC Türkçe Windows'ta cp1254 okur, `TEXT()` literalleri bozulur. Kod düzenleme seansı sonunda `powershell -ExecutionPolicy Bypass -File Tools\add-bom.ps1` çalıştır (Write/Edit araçları BOM'suz yazar).
- **Actor rotasyonunda oku-değiştir-yaz YASAK:** `GetActorRotation()` pitch'i [-90,+90]'a normalize eder; sürekli dönen şeyler (güneş vb.) rotasyonu her zaman MUTLAK yazmalı (bkz. `DayNightCycle.cpp` ADR).
- **Yerel değişkenlere miras alınan üye adları verme:** UE, C4458 (gölgeleme) uyarısını HATA olarak derler. Sık tuzaklar: `Character` (`AController::Character`), `Owner` (`AActor::Owner`), `InputComponent` (`AActor::InputComponent`), `Controller` (`APawn::Controller`). Yerel için `PlayerChar` gibi farklı ad kullan.
- **IMC eşlemeleri `DefaultKeyMappings.Mappings`'e yazılır:** UE 5.8'de `InputMappingContext.Mappings` deprecated ve runtime tarafından OKUNMUYOR (`GetMappings()` yalnızca `DefaultKeyMappings.Mappings` döndürür). Deprecated diziye yazılan eşleme sessizce yok sayılır (bkz. ADR 2026-07-23).
- **Editör otomasyonu (MCP):** Editör açıkken varlık oluşturma/özellik atama/PIE kontrolü, `.mcp.json`'daki `unreal-mcp` sunucusuyla (`ModelContextProtocol` + `EditorToolset` plugin'leri, port 8000, AutoStart açık) programatik yapılabilir. Claude Code oturumu sunucuyu görmüyorsa doğrudan HTTP/JSON-RPC ile konuşulabilir (`initialize` → `tools/call`; SSE yanıt formatına dikkat). Kısıtlar ADR'de.
- **MCP ile oluşturulan varlıklar UÇUCUDUR — hemen `save_assets` çağır:** kaydedilmemiş varlık editör kapanınca yok olur. Doğrulama sırası: yaz → `save_assets` → diskten geri oku (önce doğrulayıp sonra kaydetme, kaydın başarısını kanıtlamaz).
- **Editör açıkken `Config/*.ini` düzenlemesi çalışan oturuma YANSIMAZ** (config boot'ta okunur). PIE davranışını o oturumda değiştirmek için harita `WorldSettings` override'ı kullan (örn. `DefaultGameMode`) ya da editörü yeniden başlat.
- **Kullanıcı makineyi aktif kullanırken sentetik girdi (SetForegroundWindow + SendInput) GÖNDERME:** odak yarışına girer, kanıt değeri sıfırdır. Oyun-içi doğrulama için pawn transform kaydı + kullanıcının kendi eliyle test kombinasyonunu kullan (bkz. Sistem 2 günlüğü).

## Klasör sahipliği

- `Source/SurvivalGame/Public|Private/<Sistem>/` — her sistemin kodu kendi klasöründe kalır; sistemler arası dosya taşıma yapma.
- Blueprint'ler `Content/Blueprints/<Sistem>/`, isimlendirme: `BP_`, `WBP_` (widget), `DT_`, `DA_`, `SM_`, `SK_`, `M_`, `MI_`, `T_`, `A_`.
- Test haritaları `Content/Maps/Tests/Test<Sistem>.umap`.

## Test beklentileri

- Saf mantık (grid, kuyruk, bütçe hesabı) → UE Automation birim testi `Private/Tests/` altına.
- Sistem entegrasyonu → `Docs/MIMARI.md`'deki entegrasyon test senaryolarını uygula.
- Derleme doğrulaması: UBT ile: `& "<UE_5.8_PATH>\Engine\Build\BatchFiles\Build.bat" SurvivalGameEditor Win64 Development -project="<repo>\SurvivalGame.uproject"`.
- Otomasyon testi çalıştırma: `& "<UE_5.8_PATH>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<repo>\SurvivalGame.uproject" -ExecCmds="Automation RunTests <Filtre>; Quit" -TestExit="Automation Test Queue Empty" -ReportOutputPath="<repo>\TestResults" -nullrhi -unattended -nop4 -nosplash`. Sonucu **stdout'tan değil** `TestResults\index.json` (`succeeded`/`failed`/`notRun`) dosyasından oku — editör asıl log'u `Saved\Logs\<Proje>.log`'a yazar, arka plan komutunun stdout'u SDK doğrulama gibi ön adımlarla kesilebilir.
- `IMPLEMENT_*_AUTOMATION_TEST` flag'lerinde `EAutomationTestFlags_ApplicationContextMask` (serbest sabit) kullan — `EAutomationTestFlags::ApplicationContextMask` (üye) UE 5.8'de yok, derlemeyi kırar (enum class'a taşındı).
- Test çalıştırmadıysan geçti deme.

## Çalışma kuralları

- Mümkün olan en küçük değişikliği yap; yeni katman/framework ekleme.
- Bir sistemi bitirince `Docs/ILERLEME.md` durumunu ve günlük tablosunu güncelle.
- Commit'ler Türkçe veya İngilizce olabilir; tek konu = tek commit.
- `Binaries/`, `Intermediate/`, `Saved/` asla commit edilmez (.gitignore hazır).
- İlk .uasset commit'inden önce Git LFS kurulmalı: `git lfs track "*.uasset" "*.umap"`.
