#include "Player/PlayerCharacter.h"
#include "Player/CameraManager.h"
#include "Interaction/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Crafting/CraftingComponent.h"
#include "Construction/ConstructionComponent.h"
#include "Player/HealthComponent.h"
#include "UI/HUDController.h"
#include "Save/SaveGameManager.h"
#include "Weather/WeatherSimulation.h"
#include "SurvivalGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/GameInstance.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Karakter mesh'i değil kontrolör yaw'ı döner; boom o rotasyonu takip eder.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CameraManagerComp = CreateDefaultSubobject<UCameraManager>(TEXT("CameraManager"));
	InteractionComp = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	CraftingComp = CreateDefaultSubobject<UCraftingComponent>(TEXT("Crafting"));
	ConstructionComp = CreateDefaultSubobject<UConstructionComponent>(TEXT("Construction"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	HUDControllerComp = CreateDefaultSubobject<UHUDController>(TEXT("HUDController"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraManagerComp->RegisterCameraRig(CameraBoom, FollowCamera);
	HealthComp->OnDeath.AddDynamic(this, &APlayerCharacter::HandleDeath);
}

void APlayerCharacter::HandleDeath()
{
	UE_LOG(LogSurvival, Log, TEXT("Oyuncu oldu — son kayit noktasina donuluyor"));
	UGameInstance* GI = GetGameInstance();
	USaveGameManager* SaveManager = GI ? GI->GetSubsystem<USaveGameManager>() : nullptr;
	if (!SaveManager || !SaveManager->RevertToLastSave())
	{
		// Inceleme bulgusu (kritik): donulecek bir kayit YOKSA (oyunun ilk olumu), oyuncuyu
		// kalici/iyilesemez 0-HP durumunda birakmak SOFT-LOCK olur — TakeDamage/Heal ikisi de
		// IsDead() true iken kalici olarak no-op olur, hicbir sistem bunu kurtaramaz. Donulecek
		// bir kontrol noktasi yoksa, tam canla devam ettir (bu ayrica bir sonraki otomatik
		// kaydetmenin 0-HP bir "olu" anlik goruntuyu meshru kontrol noktasi sanip kaydetmesini
		// de yapisal olarak imkansiz kilar — health hicbir Tick sinirini 0'da gecmiyor).
		UE_LOG(LogSurvival, Warning, TEXT("Oyuncu oldu ama kayit noktasi yok — tam canla devam ediliyor"));
		HealthComp->SetCurrentHealthForLoad(HealthComp->GetMaxHealth());
	}

	// PIE'de CANLI bulunan bulgu: donus BASARILI (kayittan) da olsa, BASARISIZ (fallback) da
	// olsa, VucutSicakligi hala tehlikeli kalabilir (basarili yolda kayittaki deger neyse o
	// geri yuklenir — o kayit bile "cani geri geldi ama hala donuyor" anindaki bir karede
	// alinmis olabilir; fallback yolunda ise hic dokunulmaz). Su an kod tabanindaki TEK hasar
	// kaynagi hipotermi/asiri-sicak oldugundan (TakeDamage'in tek cagrildigi yer
	// TemperatureSimulation), bir sonraki karede ayni sebeple tekrar OnDeath tetiklenip sonsuz
	// olum-dirilis donguesune girilebilir (canli PIE testinde onlarca tekrarla dogrulandi, hem
	// fallback hem basarili-yukleme yollarinda). VucutSicakligini HER donus turunde
	// PlayerCharacter'in kendi spawn varsayilanina (37 derece) sifirlamak bu dongueyu kirar —
	// baska bir olum sebebi eklenirse bu varsayim yeniden degerlendirilmeli.
	SetBodyTemperature(37.0f);

	// Ölüm-döngüsü kök nedeni (playtest bulgusu, 2026-08-14): vücut sıcaklığını sıfırlamak ANLIK
	// bir ölüm sebebini kırar, ama sebep KALICIYSA (kar fırtınası -10°C) sıcaklık ~8sn'de tekrar
	// düşüp yeniden öldürür — üstelik AutoSave fırtına anında alındıysa RevertToLastSave her
	// seferinde fırtınalı kaydı geri yükler → sonsuz ölüm→revert döngüsü (canlı gözlemlendi).
	// Havayı da Clear'a çekmek ortamsal sebebi kaldırır (aynı gerekçe: kod tabanındaki TEK hasar
	// kaynağı hipotermi/aşırı-sıcak, o da havadan türer). SetBodyTemperature ile aynı desende
	// savunmacı sıfırlama; yeni bir hasar kaynağı eklenirse bu varsayım yeniden değerlendirilmeli.
	if (GI)
	{
		if (UWeatherSimulation* WeatherSim = GI->GetSubsystem<UWeatherSimulation>())
		{
			WeatherSim->ForceWeather(EWeatherCondition::Clear);
		}
	}
}

void APlayerCharacter::ApplyMoveInput(const FVector2D& AxisValue)
{
	if (!Controller || AxisValue.IsNearlyZero())
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, AxisValue.Y);
	AddMovementInput(RightDirection, AxisValue.X);
}

void APlayerCharacter::ApplyLookInput(const FVector2D& AxisValue)
{
	AddControllerYawInput(AxisValue.X);
	AddControllerPitchInput(AxisValue.Y);
}
