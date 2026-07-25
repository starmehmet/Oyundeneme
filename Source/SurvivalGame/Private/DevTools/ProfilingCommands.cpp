// Sistem #22 (Dev araçları) — YOL_HARITASI.md'de "ERKEN yap" öncelikli. Bu dosya HENÜZ bir
// sınıf/subsystem barındırmıyor, yalnızca profiling için tek bir konsol komutu — Sistem #22
// gerçek kapsamıyla (weather_set/time_scale gibi diğer sistemlerin ZATEN sahip olduğu
// komutları tekilleştirme, lojistik görselleştirici) genişleyince bu klasör büyüyecek.

#include "Construction/BuildingBase.h"
#include "Construction/BuildingDatabase.h"
#include "Construction/BuildingDefinition.h"
#include "Construction/FoundationGrid.h"
#include "SurvivalGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// Insaat sisteminin (Sistem #7) gercek malzeme/gecerlilik kontrollerini KASITLI olarak
	// atlar — bu bir performans olcum araci, oyun akisi degil (ayni desen: diger sistemlerin
	// dev-tool konsol komutlari da gercek kisitlari atlamaz ama BU komutun TEK amaci FPS
	// stres testi oldugundan, yuzlerce ogeyi elle toplatmak orantisiz olurdu).
	bool SpawnStressBuilding(UWorld* World, UFoundationGrid* Grid, FName BuildingID,
		const FBuildingDefinition& Def, const FIntPoint& Coord)
	{
		if (Grid->IsCellOccupied(Coord))
		{
			return false;
		}
		const TSubclassOf<ABuildingBase> SpawnClass = Def.BuildingClass ? Def.BuildingClass.Get() : ABuildingBase::StaticClass();
		const FVector Location = Grid->GridCoordToWorld(Coord, 0.0f);
		ABuildingBase* NewBuilding = World->SpawnActor<ABuildingBase>(SpawnClass, Location, FRotator::ZeroRotator);
		if (!NewBuilding)
		{
			return false;
		}
		NewBuilding->BeginConstruction(BuildingID, Def, Coord);
		if (!Grid->RegisterBuilding(NewBuilding, Coord))
		{
			NewBuilding->Destroy();
			return false;
		}
		return true;
	}

	FAutoConsoleCommandWithWorldAndArgs GCmdProfileSpawnStress(
		TEXT("profile_spawn_stress"),
		TEXT("Performans olcumu icin materyal harcamadan bina+makine dogurur: profile_spawn_stress [BinaSayisi=100] [MakineSayisi=50]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World || !World->GetGameInstance())
				{
					return;
				}
				UBuildingDatabase* DB = World->GetGameInstance()->GetSubsystem<UBuildingDatabase>();
				UFoundationGrid* Grid = World->GetSubsystem<UFoundationGrid>();
				if (!DB || !Grid)
				{
					UE_LOG(LogSurvival, Warning, TEXT("profile_spawn_stress: BuildingDatabase/FoundationGrid bulunamadi"));
					return;
				}

				FBuildingDefinition PlainDef;
				FBuildingDefinition MachineDef;
				if (!DB->FindBuilding(TEXT("Depo"), PlainDef) || !DB->FindBuilding(TEXT("OdunFirini"), MachineDef))
				{
					UE_LOG(LogSurvival, Warning, TEXT("profile_spawn_stress: 'Depo'/'OdunFirini' DT_Buildings'te bulunamadi"));
					return;
				}

				// Inceleme bulgusu: weather_force'un kendi FMath::Clamp deseniyle AYNI —
				// kelepcelenmemis buyuk bir deger (ör. yanlislikla fazladan bir hane) tek
				// karede binlerce senkron SpawnActor'a yol acip editoru dondurebilirdi.
				constexpr int32 MaxStressCount = 2000;
				const int32 NumBuildings = Args.Num() > 0 ? FMath::Clamp(FCString::Atoi(*Args[0]), 0, MaxStressCount) : 100;
				const int32 NumMachines = Args.Num() > 1 ? FMath::Clamp(FCString::Atoi(*Args[1]), 0, MaxStressCount) : 50;

				// Mevcut test icerigiyle CAKISMAMAK icin uzak, gorunmez bir bolgeden baslar
				// (oyuncu spawn'indan binlerce UU otede) — genis bir satir/sutun izgarasinda
				// ilk BOS hucreyi arar. Inceleme bulgusu: Row/Column artik STATIC — komut ayni
				// PIE oturumunda TEKRAR cagrilirsa (tipik profiling akisi: dogur->olc->tekrar
				// dogur) taramanin HER SEFERINDE (0,0)'dan baslayip onceki tum spawn'lari
				// tekrar tekrar atlamasi (kumulatif spawn sayisiyla gittikce yavaslayan bir
				// dongu) onlenir.
				constexpr int32 StartColumn = 50;
				constexpr int32 Columns = 30;
				static int32 Row = 0;
				static int32 Column = 0;

				auto NextFreeCoord = [Grid]() -> FIntPoint
				{
					for (;;)
					{
						const FIntPoint Candidate(StartColumn + Column, Row);
						++Column;
						if (Column >= Columns)
						{
							Column = 0;
							++Row;
						}
						if (!Grid->IsCellOccupied(Candidate))
						{
							return Candidate;
						}
					}
				};

				int32 SpawnedBuildings = 0;
				for (int32 i = 0; i < NumBuildings; ++i)
				{
					if (SpawnStressBuilding(World, Grid, TEXT("Depo"), PlainDef, NextFreeCoord()))
					{
						++SpawnedBuildings;
					}
				}

				int32 SpawnedMachines = 0;
				for (int32 i = 0; i < NumMachines; ++i)
				{
					if (SpawnStressBuilding(World, Grid, TEXT("OdunFirini"), MachineDef, NextFreeCoord()))
					{
						++SpawnedMachines;
					}
				}

				UE_LOG(LogSurvival, Log, TEXT("profile_spawn_stress: %d/%d bina + %d/%d makine dogruldu (izgara hucre-basi=%d)"),
					SpawnedBuildings, NumBuildings, SpawnedMachines, NumMachines, Row * Columns + Column);
			}));
}
