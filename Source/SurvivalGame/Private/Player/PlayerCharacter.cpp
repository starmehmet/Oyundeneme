#include "Player/PlayerCharacter.h"
#include "Player/CameraManager.h"
#include "Interaction/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Crafting/CraftingComponent.h"
#include "Construction/ConstructionComponent.h"
#include "Player/HealthComponent.h"
#include "UI/HUDController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

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
