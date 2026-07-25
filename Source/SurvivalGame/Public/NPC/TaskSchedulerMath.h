#pragma once

#include "CoreMinimal.h"
#include "NPC/TaskDefinition.h"

/**
 * Sistem #16 — Görev Planlayıcı: saf, durum tutmayan uygunluk/öncelik/seçim matematiği. Yan
 * etkisiz; birim testleri Private/Tests/TaskSchedulerMathTests.cpp.
 */
namespace SurvivalTaskScheduler
{
	inline bool IsNPCQualified(int32 NPCSkillLevel, int32 RequiredSkillLevel)
	{
		return NPCSkillLevel >= RequiredSkillLevel;
	}

	inline bool IsTaskAvailable(double AvailableAfterGameTime, double CurrentGameTime)
	{
		return CurrentGameTime >= AvailableAfterGameTime;
	}

	inline double ComputeBackoffAvailableTime(double CurrentGameTime, float BackoffDuration)
	{
		return CurrentGameTime + FMath::Max(0.0f, BackoffDuration);
	}

	/**
	 * Verilen görev listesinde, bu NPC'nin becerisinin karşıladığı VE şu an kullanılabilir
	 * (backoff süresi geçmiş) görevler arasında EN YÜKSEK öncelikliyi seçer — önceliklendirme,
	 * beceri-eşleştirme ve backoff-filtreleme TEK bir karar noktasında birleştirilir
	 * (`ProductionMath::DetermineBlockedState`'teki gibi küçük, saf, öncelikli karar fonksiyonu
	 * deseni). Uygun görev yoksa `INDEX_NONE` döner. Eşit öncelikte İLK (kuyruğa önce giren)
	 * görev kazanır (kararlı sıralama — açlık/adalet garantisi yok, basit ve öngörülebilir).
	 */
	inline int32 FindBestEligibleTaskIndex(const TArray<FTaskDefinition>& Tasks, int32 NPCSkillLevel, double CurrentGameTime)
	{
		int32 BestIndex = INDEX_NONE;
		float BestPriority = -TNumericLimits<float>::Max();

		for (int32 i = 0; i < Tasks.Num(); ++i)
		{
			const FTaskDefinition& Task = Tasks[i];
			if (!IsNPCQualified(NPCSkillLevel, Task.RequiredSkillLevel))
			{
				continue;
			}
			if (!IsTaskAvailable(Task.AvailableAfterGameTime, CurrentGameTime))
			{
				continue;
			}
			if (Task.Priority > BestPriority)
			{
				BestPriority = Task.Priority;
				BestIndex = i;
			}
		}
		return BestIndex;
	}
}
