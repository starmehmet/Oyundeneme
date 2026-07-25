#pragma once

#include "CoreMinimal.h"

// Ozel trace kanallari — Config/DefaultEngine.ini ile eslesir (CLAUDE.md kurali)
constexpr ECollisionChannel ECC_Interaction = ECC_GameTraceChannel1;
constexpr ECollisionChannel ECC_ConstructionPlacement = ECC_GameTraceChannel2;

// Proje geneli log kategorileri — FGameLogger bunlarin ustune kurulur (Docs/MIMARI.md #23)
DECLARE_LOG_CATEGORY_EXTERN(LogSurvival, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalProduction, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalLogistics, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalWeather, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalSave, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalAudio, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalUI, Log, All);
