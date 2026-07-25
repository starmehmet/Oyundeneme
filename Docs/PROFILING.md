# PROFİLİNG — Sistem #25

> Sistem #22'nin (`profile_spawn_stress`) ürettiği sahne + bu dosyanın checklist'i, düzenli
> (haftalık) bir performans nabzı tutmak için. Ölçüm metodolojisi ve bilinen bir tuzak
> `YOL_HARITASI.md`'nin KARAR KAYDI'nda ("stat fps PIE penceresi OS-odaksızken GÜVENİLMEZ").

## Araçlar

- **`profile_spawn_stress [Bina] [Makine]`** (`DevTools/ProfilingCommands.cpp`) — materyal harcamadan bina+makine doğurur, varsayılan 100+50.
- **`stat fps`** / **`stat unit`** — PIE penceresi **OS-odaklıyken** okunmalı (odaksızken editörün "Use Less CPU when in Background" throttle'ı yanıltıcı düşük sayılar verir).
- **`SURVIVAL_SCOPED_TIMER_WARN(Label, EşikMs)`** / **`SURVIVAL_SCOPED_TIMER(Label)`** (`DevTools/ScopedTimer.h`) — bir kod bloğunun süresini ölçer. Eşik verilirse aşıldığında `LogSurvival` Warning (verbosity'den bağımsız görünür); verilmezse Verbose (varsayılan sessiz, `Log LogSurvival Verbose` ile açılır). Unreal'in kendi `stat`/Insights altyapısının YERİNE değil, onu açmadan hızlı cevap almak için. **Yalnızca Development/DebugGame/Editor** — Shipping build'de `bUseLoggingInShipping` açık olmadığı sürece (bu projede açık DEĞİL) `UE_LOG` derleme zamanında elenir, araç sessizce hiçbir şey yazmaz (projedeki diğer tüm dev-konsol komutlarıyla aynı, zaten Shipping profiling aracı değil).
- **`logistics_visualize [0|1]`** (`DevTools/LogisticsVisualizer.cpp`) — lojistik ağını viewport'ta gösterir, görsel debug için.

## Haftalık checklist

1. `TestPlayer` haritasında PIE başlat, pencereyi **odakla**.
2. `stat fps` + `stat unit` aç.
3. `profile_spawn_stress 100 50` çalıştır, **~10 saniye bekle** (spawn-anı yığılması geçsin).
4. `stat unit` çıktısını (Frame/Game/Draw/GPU) not al, aşağıdaki tabloya bir satır ekle.
5. Frame süresi Game+Draw+GPU toplamından belirgin şekilde büyükse (>2×) önce pencere odağını kontrol et — bu şimdiye kadarki TEK yanlış-alarmın nedeniydi.
6. `Log LogSurvival Verbose` ile `SURVIVAL_SCOPED_TIMER` çıktılarını aç, `WorldPartitionHelper::RebuildCellRegistry` gibi periyodik sistemlerin süresini gözden geçir — eşiği aşan bir `Warning` zaten verbosity'den bağımsız görünür.
7. Önceki haftaya göre >%20 kötüleşme varsa kök nedeni araştır (bu oturumdaki FPS düşüşü araştırması `YOL_HARITASI.md`'de örnek metodoloji olarak duruyor).

## Regresyon karşılaştırma tablosu

| Tarih | Senaryo | FPS | Frame (ms) | Game (ms) | Draw (ms) | GPU (ms) | Not |
|-------|---------|-----|-----------|-----------|-----------|----------|-----|
| 2026-07-25 | `profile_spawn_stress 100 50`, pencere odaklı | 59.93 | 16.69 | 5.08 | 3.37 | 5.09 | İlk ölçüm — hedef karşılandı, bkz. `YOL_HARITASI.md` ADR |

**`SURVIVAL_SCOPED_TIMER` ile ilk gerçek ölçüm** (2026-07-25, TestPlayer haritası, mevcut sahne ölçeği): `WorldPartitionHelper::RebuildCellRegistry` — **0.03-0.06 ms/çağrı** (5ms uyarı eşiğinin çok altında, hiç Warning tetiklenmedi). Daha önce "kare-bölümlemesiz periyodik tarama, ölçülmedi" olarak dokümante edilmiş açık nokta böylece kapandı — bu sahne ölçeğinde kesinlikle darboğaz değil. Aktör sayısı çok büyürse (bkz. `YOL_HARITASI.md` ADR) tekrar ölçülmeli.

> Yeni ölçüm eklerken: aynı senaryoyu (`profile_spawn_stress 100 50`) kullan ki satırlar karşılaştırılabilir kalsın; farklı bir senaryo test ediyorsan "Senaryo" sütununa yaz.
