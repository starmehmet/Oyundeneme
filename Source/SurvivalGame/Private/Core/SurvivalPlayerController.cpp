#include "Core/SurvivalPlayerController.h"
#include "Player/PlayerCharacter.h"
#include "Player/CameraManager.h"
#include "Interaction/InteractionComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "SurvivalGame.h"

void ASurvivalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			else
			{
				UE_LOG(LogSurvival, Warning,
					TEXT("SurvivalPlayerController: DefaultMappingContext atanmamis — oyuncu girisi calismayacak. Bir Blueprint alt sinifinda ata."));
			}
		}
	}
}

void ASurvivalPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogSurvival, Error, TEXT("SurvivalPlayerController: InputComponent UEnhancedInputComponent degil — DefaultInput.ini kontrol et."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurvivalPlayerController::Handle_Move);
	}
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASurvivalPlayerController::Handle_Look);
	}
	if (JumpAction)
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASurvivalPlayerController::Handle_JumpStarted);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASurvivalPlayerController::Handle_JumpCompleted);
	}
	if (ZoomAction)
	{
		EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ASurvivalPlayerController::Handle_Zoom);
	}
	if (InteractAction)
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASurvivalPlayerController::Handle_Interact);
	}
}

void ASurvivalPlayerController::Handle_Move(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerChar->ApplyMoveInput(Value.Get<FVector2D>());
	}
}

void ASurvivalPlayerController::Handle_Look(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerChar->ApplyLookInput(Value.Get<FVector2D>());
	}
}

void ASurvivalPlayerController::Handle_JumpStarted(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerChar->Jump();
	}
}

void ASurvivalPlayerController::Handle_JumpCompleted(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		PlayerChar->StopJumping();
	}
}

void ASurvivalPlayerController::Handle_Zoom(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		if (UCameraManager* CamManager = PlayerChar->GetCameraManager())
		{
			CamManager->ApplyZoomInput(Value.Get<float>());
		}
	}
}

void ASurvivalPlayerController::Handle_Interact(const FInputActionValue& Value)
{
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn()))
	{
		if (UInteractionComponent* Interaction = PlayerChar->GetInteractionComponent())
		{
			Interaction->TryInteract();
		}
	}
}
