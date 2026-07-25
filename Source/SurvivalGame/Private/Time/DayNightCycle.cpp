#include "Time/DayNightCycle.h"
#include "Time/TimeKeeper.h"
#include "Time/TimeMath.h"
#include "Time/TimeSettings.h"
#include "SurvivalGame.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"

void UDayNightCycle::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// "Sun" tag'li isik oncelikli; yoksa ilk directional light
	ADirectionalLight* Fallback = nullptr;
	for (TActorIterator<ADirectionalLight> It(&InWorld); It; ++It)
	{
		if (It->ActorHasTag(TEXT("Sun")))
		{
			SunLight = *It;
			break;
		}
		if (!Fallback)
		{
			Fallback = *It;
		}
	}
	if (!SunLight.IsValid() && Fallback)
	{
		SunLight = Fallback;
	}

	if (SunLight.IsValid())
	{
		SunYaw = SunLight->GetActorRotation().Yaw;
		UE_LOG(LogSurvival, Log, TEXT("DayNightCycle: gunes bulundu (%s, yaw %.1f)"),
			*SunLight->GetName(), SunYaw);
	}
}

void UDayNightCycle::RegisterSunLight(ADirectionalLight* InSunLight)
{
	SunLight = InSunLight;
	if (InSunLight)
	{
		SunYaw = InSunLight->GetActorRotation().Yaw;
	}
	bWarnedNoSun = false;
}

void UDayNightCycle::GetSanitizedSunTimes(double& OutSunrise, double& OutSunset) const
{
	const UTimeSettings* Settings = GetDefault<UTimeSettings>();
	OutSunrise = Settings->SunriseMinute;
	OutSunset = Settings->SunsetMinute;

	// TimeMath on kosulu: 0 < Sunrise < Sunset < 1440. Bozuk config sessiz kalici
	// geceye dusurur — onar ve bir kez uyar.
	if (!(OutSunrise > 0.0 && OutSunrise < OutSunset && OutSunset < SurvivalTime::MinutesPerDay))
	{
		if (!bWarnedBadSunTimes)
		{
			UE_LOG(LogSurvivalWeather, Warning,
				TEXT("TimeSettings gecersiz (Sunrise=%.0f Sunset=%.0f) — varsayilan 360/1080 kullaniliyor. Project Settings > Game > Survival Time'i duzeltin."),
				OutSunrise, OutSunset);
			bWarnedBadSunTimes = true;
		}
		OutSunrise = 360.0;
		OutSunset = 1080.0;
	}
}

bool UDayNightCycle::IsDaytimeNow() const
{
	const UWorld* World = GetWorld();
	const UTimeKeeper* Keeper = (World && World->GetGameInstance())
		? World->GetGameInstance()->GetSubsystem<UTimeKeeper>()
		: nullptr;
	if (!Keeper)
	{
		return true;
	}
	double Sunrise, Sunset;
	GetSanitizedSunTimes(Sunrise, Sunset);
	return SurvivalTime::IsDaytime(Keeper->GetMinuteOfDayFloat(), Sunrise, Sunset);
}

void UDayNightCycle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const UWorld* World = GetWorld();
	const UTimeKeeper* Keeper = (World && World->GetGameInstance())
		? World->GetGameInstance()->GetSubsystem<UTimeKeeper>()
		: nullptr;
	if (!Keeper)
	{
		return;
	}

	UpdateSun(Keeper->GetMinuteOfDayFloat());
}

void UDayNightCycle::UpdateSun(double MinuteOfDayF)
{
	double Sunrise, Sunset;
	GetSanitizedSunTimes(Sunrise, Sunset);

	// Gunduz/gece olaylari — isik yokken de calisir (oyun mantigi isiga bagli degil).
	// Ilk degerlendirme: durum yayini YAPILIR (OnDayNightStateChanged), kenar yayini yapilmaz.
	const bool bDay = SurvivalTime::IsDaytime(MinuteOfDayF, Sunrise, Sunset);
	if (!bWasDaytime.IsSet())
	{
		bWasDaytime = bDay;
		OnDayNightStateChanged.Broadcast(bDay);
	}
	else if (*bWasDaytime != bDay)
	{
		bWasDaytime = bDay;
		OnDayNightStateChanged.Broadcast(bDay);
		if (bDay)
		{
			UE_LOG(LogSurvival, Log, TEXT("Gundogumu (dakika %.0f)"), MinuteOfDayF);
			OnSunrise.Broadcast();
		}
		else
		{
			UE_LOG(LogSurvival, Log, TEXT("Gunbatimi (dakika %.0f)"), MinuteOfDayF);
			OnSunset.Broadcast();
		}
	}

	if (!SunLight.IsValid())
	{
		if (!bWarnedNoSun)
		{
			UE_LOG(LogSurvival, Warning,
				TEXT("DayNightCycle: sahnede directional light yok — gunes guncellenmiyor. 'Sun' tag'li isik ekleyin."));
			bWarnedNoSun = true;
		}
		return;
	}

	// MUTLAK rotasyon yaz — asla GetActorRotation ile geri okuyup degistirme:
	// FQuat::Rotator pitch'i [-90,+90]'a normalize eder, oku-degistir-yaz deseni
	// -90 sonrasi her karede aynalanip gunesi titretir (inceleme bulgusu, bkz. ADR).
	const double Pitch = SurvivalTime::ComputeSunPitchDegrees(MinuteOfDayF, Sunrise, Sunset);
	SunLight->SetActorRotation(FRotator(Pitch, SunYaw, 0.0));

	const double Factor = SurvivalTime::ComputeDaylightFactor(
		MinuteOfDayF, Sunrise, Sunset, GetDefault<UTimeSettings>()->TransitionMinutes);
	const float Lux = FMath::Lerp(
		GetDefault<UTimeSettings>()->NightIntensityLux,
		GetDefault<UTimeSettings>()->DayIntensityLux,
		static_cast<float>(Factor));
	if (ULightComponent* LightComp = SunLight->GetLightComponent())
	{
		LightComp->SetIntensity(Lux);
	}
}
