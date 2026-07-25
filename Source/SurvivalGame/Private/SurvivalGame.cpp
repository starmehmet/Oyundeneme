#include "SurvivalGame.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSurvival);
DEFINE_LOG_CATEGORY(LogSurvivalProduction);
DEFINE_LOG_CATEGORY(LogSurvivalLogistics);
DEFINE_LOG_CATEGORY(LogSurvivalWeather);
DEFINE_LOG_CATEGORY(LogSurvivalSave);
DEFINE_LOG_CATEGORY(LogSurvivalAudio);
DEFINE_LOG_CATEGORY(LogSurvivalUI);

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, SurvivalGame, "SurvivalGame");
