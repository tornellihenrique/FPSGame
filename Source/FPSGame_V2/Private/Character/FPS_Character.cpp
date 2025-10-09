#include "Character/FPS_Character.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AFPS_Character::AFPS_Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFPS_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams DefaultParameters;
	DefaultParameters.bIsPushBased = true;
	DefaultParameters.Condition = COND_None;

	// DOREPLIFETIME_WITH_PARAMS_FAST(AFPS_Character, Property, DefaultParameters);
}

void AFPS_Character::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AFPS_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPS_Character::BeginPlay()
{
	Super::BeginPlay();
}

void AFPS_Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AFPS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPS_Character::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
}

void AFPS_Character::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);
}

void AFPS_Character::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{

}

void AFPS_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AFPS_Character::UnPossessed()
{
	Super::UnPossessed();
}

void AFPS_Character::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void AFPS_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AFPS_Character::Restart()
{
	Super::Restart();
}

void AFPS_Character::PawnClientRestart()
{
	Super::PawnClientRestart();
}

void AFPS_Character::NotifyControllerChanged()
{
	const APlayerController* PreviousPlayer = Cast<APlayerController>(PreviousController);
	if (PreviousPlayer && ::IsValid(PreviousPlayer))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer());
		if (InputSubsystem && ::IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}

	APlayerController* NewPlayer = Cast<APlayerController>(GetController());
	if (NewPlayer && ::IsValid(NewPlayer))
	{
		NewPlayer->InputYawScale_DEPRECATED = 1.0f;
		NewPlayer->InputPitchScale_DEPRECATED = 1.0f;
		NewPlayer->InputRollScale_DEPRECATED = 1.0f;

		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayer->GetLocalPlayer());
		if (InputSubsystem && ::IsValid(InputSubsystem))
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	Super::NotifyControllerChanged();
}

void AFPS_Character::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(Input))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Look);
	}
}

void AFPS_Character::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller && Axis.SizeSquared() > 0.f)
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector  Fwd = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector  Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		AddMovementInput(Fwd, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void AFPS_Character::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}