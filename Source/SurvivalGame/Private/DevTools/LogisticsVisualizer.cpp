// Sistem #22 (Dev araçları) — MIMARI.md'nin "lojistik görselleştirici" maddesi. Gerçek çizim
// mantığı Sistem #8'in kendi dosyasında yaşıyor (ULogisticsNetwork::DrawDebugVisualization,
// düğüm/aktif-taşıma verisine private erişim gerektiriyor) — bu dosya yalnızca dev-araç
// yüzeyini (konsol komutu) DevTools altında topluyor, ProfilingCommands.cpp ile aynı gerekçe.
//
// "weather_set"/"time_scale" (MIMARI'nin bu sistem için orijinal notu) AYRI YAZILMADI — Sistem
// #1 (time_sethour/time_settimescale) ve Sistem #11 (weather_force) bunları zaten karşılıyor;
// aynı işi farklı isimle tekrar etmek "sistemler arası paralel/kopya sistem" yasağına girerdi.

#include "Logistics/LogisticsNetwork.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	FAutoConsoleCommandWithWorldAndArgs GCmdLogisticsVisualize(
		TEXT("logistics_visualize"),
		TEXT("Lojistik agini viewport'ta gorsellestir (dugumler+aktif tasimalar): logistics_visualize [0|1] (arguman yoksa ac/kapa)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
				ULogisticsNetwork* Network = GI ? GI->GetSubsystem<ULogisticsNetwork>() : nullptr;
				if (!Network)
				{
					return;
				}

				const bool bNewState = Args.Num() > 0 ? (FCString::Atoi(*Args[0]) != 0) : !Network->IsVisualizationEnabled();
				Network->SetVisualizationEnabled(bNewState);
				UE_LOG(LogSurvivalLogistics, Log, TEXT("logistics_visualize: %s"), bNewState ? TEXT("acik") : TEXT("kapali"));
			}));
}
