#include "Components/GL_CharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveFloat.h"

UGL_CharacterMovementComponent::UGL_CharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRunPhysicsWithNoController = true;
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;

	// Defaults (similar spirit to ALS)
	MinAnalogWalkSpeed = 25.f;
	MaxWalkSpeed = 375.f;
	MaxWalkSpeedCrouched = 150.f;
	MaxAccelerationWalking = 2000.f;
	BrakingDecelerationWalking = 1500.f;
	GroundFriction = 4.f;
	AirControl = 0.15f;

	bUseSeparateBrakingFriction = false;
	BrakingFrictionFactor = 0.f;
	NavAgentProps.bCanCrouch = true;
}

void UGL_CharacterMovementComponent::SetMovementSettings(UGL_MovementSettings* NewSettings)
{
	check(NewSettings);
	MovementSettings = NewSettings;
	RefreshGaitSettings();
}

void UGL_CharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	RefreshGaitSettings();

	if (IsWalking())
	{
		RefreshGroundedMovementSettings();
	}
}

void UGL_CharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (IsWalking())
	{
		RefreshGroundedMovementSettings();
	}
}

void UGL_CharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (IsWalking())
	{
		RefreshGroundedMovementSettings();
	}
}

FNetworkPredictionData_Client* UGL_CharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UGL_CharacterMovementComponent* MutableThis = const_cast<UGL_CharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FGL_NetworkPredictionData(*this);
	}
	return ClientPredictionData;
}

void UGL_CharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);
	if (HasValidData() && (bRunPhysicsWithNoController || CharacterOwner->GetController()))
	{
		OnPhysicsRotation.Broadcast(DeltaTime);
	}
}

void UGL_CharacterMovementComponent::MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult /*= nullptr*/)
{
	if (IsMovingOnGround())
	{
		RefreshGroundedMovementSettings();
	}

	Super::MoveSmooth(InVelocity, DeltaTime, StepDownResult);
}

float UGL_CharacterMovementComponent::GetMaxAcceleration() const
{
	if (IsMovingOnGround())
	{
		return MaxAccelerationWalking;
	}

	return Super::GetMaxAcceleration();
}

bool UGL_CharacterMovementComponent::ApplyRequestedMove(float DeltaTime, float CurrentMaxAcceleration, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& RequestedAcceleration, float& RequestedSpeed)
{
	return !bInputBlocked && Super::ApplyRequestedMove(DeltaTime, CurrentMaxAcceleration, MaxSpeed, Friction, BrakingDeceleration, RequestedAcceleration, RequestedSpeed);
}

void UGL_CharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode /*= 0*/)
{
	if (!bMovementModeLocked)
	{
		Super::SetMovementMode(NewMovementMode, NewCustomMode);
	}
}

void UGL_CharacterMovementComponent::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	bCrouchMaintainsBaseLocation = true;

	if (IsWalking())
	{
		RefreshGaitSettings();
		RefreshGroundedMovementSettings();
	}
}

FVector UGL_CharacterMovementComponent::ConsumeInputVector()
{
	FVector Input = Super::ConsumeInputVector();
	if (bInputBlocked)
	{
		return FVector::ZeroVector;
	}
	return Input;
}

void UGL_CharacterMovementComponent::RefreshGaitSettings()
{
	if (!MovementSettings) { return; }

	const FGL_GaitSettings* NewSettings = MovementSettings->Stances.Find(Stance);
	GaitSettings = NewSettings ? *NewSettings : FGL_GaitSettings{};
}

void UGL_CharacterMovementComponent::RefreshGroundedMovementSettings()
{
	// Compute direction-dependent Walk/Run if enabled
	float WalkSpeed = GaitSettings.WalkForwardSpeed;
	float RunSpeed = GaitSettings.RunForwardSpeed;

	if (GaitSettings.bAllowDirectionDependentSpeed && !Velocity.IsNearlyZero() && MovementSettings)
	{
		const FRotator ViewRot = (GetController() ? GetController()->GetControlRotation() : GetCharacterOwner()->GetViewRotation());
		const FQuat Twist = FQuat(FVector::UpVector, FMath::DegreesToRadians(ViewRot.Yaw));
		const FVector2D RelVel = FVector2D(Twist.Inverse().RotateVector(Velocity));
		const float VelAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(RelVel.Y, RelVel.X));
		const float t = 1.f - FMath::Clamp(MovementSettings->VelocityAngleToSpeedInterpolationRange.GetRangePct(FMath::Abs(VelAngleDeg)), 0.f, 1.f);

		WalkSpeed = FMath::Lerp(GaitSettings.WalkBackwardSpeed, GaitSettings.WalkForwardSpeed, t);
		RunSpeed = FMath::Lerp(GaitSettings.RunBackwardSpeed, GaitSettings.RunForwardSpeed, t);
	}

	// Map speed -> gait amount [0..3]
	const float Speed2D = Velocity.Size2D();
	if (Speed2D > RunSpeed)
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f(RunSpeed, GaitSettings.SprintSpeed), FVector2f(2.f, 3.f), Speed2D);
	}
	else if (Speed2D > WalkSpeed)
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f(WalkSpeed, RunSpeed), FVector2f(1.f, 2.f), Speed2D);
	}
	else
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f(0.f, WalkSpeed), FVector2f(0.f, 1.f), Speed2D);
	}

	if (MaxAllowedGait.MatchesTagExact(GameplayGaitTags::Walking))
	{
		MaxWalkSpeed = WalkSpeed;
	}
	else if (MaxAllowedGait.MatchesTagExact(GameplayGaitTags::Running))
	{
		MaxWalkSpeed = RunSpeed;
	}
	else if (MaxAllowedGait.MatchesTagExact(GameplayGaitTags::Sprinting))
	{
		MaxWalkSpeed = GaitSettings.SprintSpeed;
	}
	else
	{
		MaxWalkSpeed = RunSpeed;
	}

	MaxWalkSpeedCrouched = MaxWalkSpeed;

	// Accel/Decel/Friction from curve by GaitAmount
	if (GaitSettings.AccelDecelFrictionCurve)
	{
		const auto& Curves = GaitSettings.AccelDecelFrictionCurve->FloatCurves;
		MaxAccelerationWalking = Curves[0].Eval(GaitAmount);
		BrakingDecelerationWalking = Curves[1].Eval(GaitAmount);
		GroundFriction = Curves[2].Eval(GaitAmount);
	}
}

// -------- SavedMove --------

void FGL_SavedMove::Clear()
{
	Super::Clear();

	Stance = GameplayStanceTags::Standing;
	MaxAllowedGait = GameplayGaitTags::Walking;
}

void FGL_SavedMove::SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration,
	FNetworkPredictionData_Client_Character& PredictionData)
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	if (const UGL_CharacterMovementComponent* Mv = Cast<UGL_CharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		Stance = Mv->GetStance();
		MaxAllowedGait = Mv->GetMaxAllowedGait();
	}
}

bool FGL_SavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDeltaTime) const
{
	const FGL_SavedMove* NewMove = static_cast<const FGL_SavedMove*>(NewMovePtr.Get());
	return Stance == NewMove->Stance &&
		MaxAllowedGait == NewMove->MaxAllowedGait &&
		Super::CanCombineWith(NewMovePtr, Character, MaxDeltaTime);
}

void FGL_SavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	if (UGL_CharacterMovementComponent* Mv = Cast<UGL_CharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		Mv->SetStance(Stance);
		Mv->SetMaxAllowedGait(MaxAllowedGait);
		Mv->RefreshGaitSettings();
	}
}