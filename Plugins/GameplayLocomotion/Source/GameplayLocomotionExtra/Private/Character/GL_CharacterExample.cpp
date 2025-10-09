#include "Character/GL_CharacterExample.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Misc/GL_GameplayTags.h"
#include "Components/GL_CameraComponent.h"

AGL_CharacterExample::AGL_CharacterExample(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Camera = CreateDefaultSubobject<UGL_CameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh());
	Camera->SetRelativeRotation_Direct(FRotator(0.0f, 90.0f, 0.0f));
}

void AGL_CharacterExample::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams DefaultParameters;
	DefaultParameters.bIsPushBased = true;
	DefaultParameters.Condition = COND_None;

	// DOREPLIFETIME_WITH_PARAMS_FAST(AGL_CharacterExample, Property, DefaultParameters);
}

void AGL_CharacterExample::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGL_CharacterExample::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGL_CharacterExample::BeginPlay()
{
	Super::BeginPlay();
}

void AGL_CharacterExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AGL_CharacterExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGL_CharacterExample::CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	if (IsLocallyControlled() && IsPlayerControlled())
	{
		if (Camera->IsActive())
		{
			Camera->GetViewInfo(ViewInfo);
			return;
		}
	}

	Super::CalcCamera(DeltaTime, ViewInfo);
}

void AGL_CharacterExample::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
}

void AGL_CharacterExample::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);
}

void AGL_CharacterExample::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AGL_CharacterExample::UnPossessed()
{
	Super::UnPossessed();
}

void AGL_CharacterExample::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void AGL_CharacterExample::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AGL_CharacterExample::Restart()
{
	Super::Restart();
}

void AGL_CharacterExample::PawnClientRestart()
{
	Super::PawnClientRestart();
}

void AGL_CharacterExample::NotifyControllerChanged()
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

void AGL_CharacterExample::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(Input))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Look);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Sprint);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Canceled, this, &AGL_CharacterExample::Input_Sprint);
		EIC->BindAction(IA_Aim, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Aim);
		EIC->BindAction(IA_Walk, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Walk);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Crouch);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &AGL_CharacterExample::Input_Jump);
		EIC->BindAction(IA_View, ETriggerEvent::Triggered, this, &AGL_CharacterExample::Input_View);
	}
}

void AGL_CharacterExample::Input_Move(const FInputActionValue& Value)
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

void AGL_CharacterExample::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AGL_CharacterExample::Input_Sprint(const FInputActionValue& ActionValue)
{
	SetDesiredGait(ActionValue.Get<bool>() ? GameplayGaitTags::Running : GameplayGaitTags::Walking);
}

void AGL_CharacterExample::Input_Aim(const FInputActionValue& ActionValue)
{
	SetAiming(ActionValue.Get<bool>());
}

void AGL_CharacterExample::Input_Walk()
{
	if (DesiredGait == GameplayGaitTags::Walking)
	{
		SetDesiredGait(GameplayGaitTags::Running);
	}
	else if (DesiredGait == GameplayGaitTags::Running)
	{
		SetDesiredGait(GameplayGaitTags::Walking);
	}
}

void AGL_CharacterExample::Input_Crouch()
{
	if (DesiredStance == GameplayStanceTags::Standing)
	{
		SetDesiredStance(GameplayStanceTags::Crouching);
	}
	else if (DesiredStance == GameplayStanceTags::Crouching)
	{
		SetDesiredStance(GameplayStanceTags::Standing);
	}
}

void AGL_CharacterExample::Input_Jump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		if (Stance == GameplayStanceTags::Crouching)
		{
			SetDesiredStance(GameplayStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void AGL_CharacterExample::Input_View()
{
	SetViewModeTag(ViewMode == GameplayViewModeTags::ThirdPerson ? GameplayViewModeTags::FirstPerson : GameplayViewModeTags::ThirdPerson);
}
