#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"
#include "HAL/PreprocessorHelpers.h"
#include "SurvivalGame.h"

/**
 * Sistem #22/#25 (Dev araçları / Performans profiling) — Unreal'in kendi `SCOPE_CYCLE_COUNTER`/
 * `TRACE_CPUPROFILER_EVENT_SCOPE` altyapısını (Insights'a besler) TEKRARLAMAZ, onlar zaten var
 * ve daha güçlü. Bu, Insights açmadan "bu blok kaç ms sürdü" sorusuna hızlı/geçici cevap veren
 * HAFİF bir tamamlayıcı — süre `LogSurvival` Verbose'ta loglanır (varsayılan sessiz, `Log
 * LogSurvival Verbose` ile açılır), `ThresholdMs` verilirse aşıldığında verbosity'den BAĞIMSIZ
 * (Verbose kapalıyken bile) Warning basar (yüksek-frekanslı yollarda sessiz kalıp yalnızca
 * "yavaşlayınca" konuşur — bkz. CLAUDE.md Logging kuralları).
 *
 * İNCELEME BULGUSU (motor kaynağından doğrulandı): bu yalnızca Development/DebugGame/Editor
 * için geçerli. Şipping build'de `UE_LOG` (Warning DAHİL, yalnız Fatal hariç) `NO_LOGGING`
 * altında DERLEME ZAMANINDA elenir (`LogMacros.h`); proje `Target.cs`'lerinde
 * `bUseLoggingInShipping` açılmadığı için (motor varsayılanı) bu araç Shipping'de SESSİZCE
 * hiçbir şey yazmaz — tıpkı projedeki diğer tüm dev-konsol komutları gibi zaten yalnızca
 * Development/Editor'da kullanılması beklenir, Shipping profiling için DEĞİLDİR.
 */
class FSurvivalScopedTimer
{
public:
	explicit FSurvivalScopedTimer(const TCHAR* InLabel, float InThresholdMs = 0.0f)
		: Label(InLabel)
		, ThresholdMs(InThresholdMs)
		, StartSeconds(FPlatformTime::Seconds())
	{
	}

	~FSurvivalScopedTimer()
	{
		const double ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
		if (ThresholdMs > 0.0f && ElapsedMs > static_cast<double>(ThresholdMs))
		{
			UE_LOG(LogSurvival, Warning, TEXT("SCOPED_TIMER '%s': %.3f ms (esik %.3f ms asildi)"), Label, ElapsedMs, ThresholdMs);
		}
		else
		{
			UE_LOG(LogSurvival, Verbose, TEXT("SCOPED_TIMER '%s': %.3f ms"), Label, ElapsedMs);
		}
	}

private:
	const TCHAR* Label;
	float ThresholdMs;
	double StartSeconds;
};

// Not: UE_JOIN __LINE__ kullanır (motorun kendi TRACE_CPUPROFILER_EVENT_SCOPE'uyla aynı kısıt)
// — aynı FİZİKSEL satırda (';' ile ayrılmış) iki çağrı aynı değişken adını üretip derleme
// hatası verir. Her çağrı kendi satırında olmalı (zaten normal kullanım).

/** Kapsam sonunda süreyi Verbose loglar (varsayılan sessiz). */
#define SURVIVAL_SCOPED_TIMER(Label) \
	FSurvivalScopedTimer UE_JOIN(SurvivalScopedTimer_, __LINE__)(TEXT(Label))

/** Kapsam sonunda süre ThresholdMs'i aşarsa Warning loglar (verbosity'den bağımsız görünür — yalnızca Development/Editor, bkz. sınıf yorumu). */
#define SURVIVAL_SCOPED_TIMER_WARN(Label, ThresholdMs) \
	FSurvivalScopedTimer UE_JOIN(SurvivalScopedTimer_, __LINE__)(TEXT(Label), ThresholdMs)
