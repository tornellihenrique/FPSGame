#include "Animation/GL_AnimInstance.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

#include "Utility/GL_Math.h"
#include "Utility/GL_Vector.h"
#include "Utility/GL_Rotation.h"
#include "Utility/GL_Constants.h"
#include "Utility/GL_PrivateMemberAcessor.h"
#include "Misc/GL_GameplayTags.h"
#include "Character/GL_Character.h"
#include "Animation/GL_AnimInstanceProxy.h"
#include "Components/GL_CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimInstance)

GL_DEFINE_PRIVATE_MEMBER_ACCESSOR(GameplayGetAnimationCurvesAccessor, &FAnimInstanceProxy::GetAnimationCurves,
	const TMap<FName, float>& (FAnimInstanceProxy::*)(EAnimCurveType) const)

void UGL_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AGL_Character>(GetOwningActor());

#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld() && !IsValid(Character))
		{
			Character = GetMutableDefault<AGL_Character>();
		}
	}
#endif
}

void UGL_AnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	check(IsValid(Character));

	TurnInPlaceState.PreviousActorYaw = Character->GetActorRotation().Yaw;
}

void UGL_AnimInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(Character))
	{
		return;
	}

	ViewMode = Character->ViewMode;
	LocomotionMode = Character->LocomotionMode;
	Stance = Character->Stance;
	Gait = Character->Gait;
	OverlayMode = Character->OverlayMode;
	bAiming = Character->bAiming;

	const FVector PrevLocation = LocomotionState.Location;

	RefreshMovementBaseOnGameThread();
	RefreshViewOnGameThread();
	RefreshLocomotionOnGameThread();
	RefreshInAirOnGameThread();

	// Teleport detection (simple)
	if (!bPendingUpdate && FVector::DistSquared(PrevLocation, LocomotionState.Location) > FMath::Square(200.f))
	{
		MarkTeleported();
	}
}

void UGL_AnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!IsValid(Character))
	{
		return;
	}

	TurnInPlaceState.bUpdatedThisFrame = false;

	RefreshLayering();
	RefreshPoseState();
	RefreshView(DeltaSeconds);
	RefreshGrounded();
	RefreshInAir();
}

void UGL_AnimInstance::NativePostUpdateAnimation()
{
	bPendingUpdate = false;
}

#if WITH_EDITOR
void UGL_AnimInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STRING_VIEW_CHECKED(UGL_AnimInstance, InAir))
	{
		InAir.GroundPredictionSweepResponses.SetAllChannels(ECR_Ignore);

		for (const ECollisionChannel CollisionChannel : InAir.GroundPredictionResponseChannels)
		{
			InAir.GroundPredictionSweepResponses.SetResponse(CollisionChannel, ECR_Block);
		}
	}
}
#endif

FAnimInstanceProxy* UGL_AnimInstance::CreateAnimInstanceProxy()
{
	return new FGL_AnimInstanceProxy(this);
}

// ----------------- Graph-facing -----------------

void UGL_AnimInstance::InitializeGrounded()
{
	GroundedState.VelocityBlend.bInitializationRequired = true;
}

void UGL_AnimInstance::RefreshGrounded()
{
	const auto& Curves =
		GameplayGetAnimationCurvesAccessor::Invoke(GetProxyOnAnyThread<FAnimInstanceProxy>(), EAnimCurveType::AttributeCurve);

	const auto GetCurveValue = [](const TMap<FName, float>& InCurves, const FName& CurveName) -> float
	{
		if (const float* Value = InCurves.Find(CurveName))
		{
			return *Value;
		}
		return 0.0f;
	};

	GroundedState.HipsDirectionLockAmount = GetCurveValue(Curves, UGL_Constants::HipsDirectionLockCurveName());
	StandingState.SprintBlockAmount = GetCurveValue(Curves, UGL_Constants::SprintBlockCurveName());

	RefreshVelocityBlend();
	RefreshGroundedLean();
}

void UGL_AnimInstance::RefreshGroundedMovement()
{
	// View relative velocity yaw
	const float ViewRelativeVelocityYaw = FMath::UnwindDegrees(UE_REAL_TO_FLOAT(LocomotionState.VelocityYawAngle - ViewState.Rotation.Yaw));

	RefreshMovementDirection(ViewRelativeVelocityYaw);
	RefreshRotationYawOffsets(ViewRelativeVelocityYaw);
}

void UGL_AnimInstance::InitializeStandingMovement()
{
	StandingState.SprintTime = 0.f;
	StandingState.bPivotActive = false;
}

void UGL_AnimInstance::RefreshStandingMovement()
{
	const float Speed = LocomotionState.Scale > UE_SMALL_NUMBER
		? (LocomotionState.Speed / LocomotionState.Scale)
		: LocomotionState.Speed;

	// Stride blending
	const float WalkStride = Standing.StrideBlendAmountWalkCurve
		? Standing.StrideBlendAmountWalkCurve->GetFloatValue(Speed)
		: 1.f;

	const float RunStride = Standing.StrideBlendAmountRunCurve
		? Standing.StrideBlendAmountRunCurve->GetFloatValue(Speed)
		: 1.f;

	const float GaitRunAlpha = (Gait == GameplayGaitTags::Running || Gait == GameplayGaitTags::Sprinting) ? 1.f : 0.f;

	StandingState.StrideBlendAmount = FMath::Lerp(WalkStride, RunStride, GaitRunAlpha);

	// Walk / Run blend amount
	StandingState.WalkRunBlendAmount = (Gait == GameplayGaitTags::Walking) ? 0.f : 1.f;

	// PlayRate (match speed to anim speed)
	const float WalkRunSpeedAmount = FMath::Lerp(
		Speed / FMath::Max(Standing.AnimatedWalkSpeed, 1.f),
		Speed / FMath::Max(Standing.AnimatedRunSpeed, 1.f),
		GaitRunAlpha);

	const float PlayRateRaw = FMath::Lerp(
		WalkRunSpeedAmount,
		Speed / FMath::Max(Standing.AnimatedSprintSpeed, 1.f),
		(Gait == GameplayGaitTags::Sprinting) ? 1.f : 0.f);

	StandingState.PlayRate = FMath::Clamp(PlayRateRaw / FMath::Max(StandingState.StrideBlendAmount, UE_KINDA_SMALL_NUMBER),
		UE_KINDA_SMALL_NUMBER, 3.f);

	// Optional sprint accel sample window (kept for parity)
	if (Gait != GameplayGaitTags::Sprinting)
	{
		StandingState.SprintTime = 0.f;
		StandingState.SprintAccelerationAmount = 0.f;
		return;
	}

	static constexpr float SprintWindowSeconds = 0.5f;
	StandingState.SprintTime = bPendingUpdate ? SprintWindowSeconds : (StandingState.SprintTime + GetDeltaSeconds());
	StandingState.SprintAccelerationAmount = StandingState.SprintTime >= SprintWindowSeconds ? 0.f : GetRelativeAccelerationAmount().X;
}

void UGL_AnimInstance::RefreshCrouchingMovement()
{
	const float Speed = LocomotionState.Scale > UE_SMALL_NUMBER
		? (LocomotionState.Speed / LocomotionState.Scale)
		: LocomotionState.Speed;

	CrouchingState.StrideBlendAmount = Crouching.StrideBlendAmountCurve
		? Crouching.StrideBlendAmountCurve->GetFloatValue(Speed)
		: 1.f;

	const float PlayRateRaw = Speed / FMath::Max(Crouching.AnimatedCrouchSpeed * CrouchingState.StrideBlendAmount, 1.f);
	CrouchingState.PlayRate = FMath::Clamp(PlayRateRaw, UE_KINDA_SMALL_NUMBER, 2.f);
}

void UGL_AnimInstance::RefreshInAir()
{
	// Jump playrate update would be triggered on Jump() call; omitted here.

	// Vertical velocity
	InAirState.VerticalVelocity = UE_REAL_TO_FLOAT(LocomotionState.Velocity.Z);

	RefreshGroundPrediction();
	RefreshInAirLean();
}

void UGL_AnimInstance::InitializeTurnInPlace()
{
	TurnInPlaceState = FGL_TurnInPlaceState{};
	TurnInPlaceState.PreviousActorYaw = LocomotionState.Rotation.Yaw;
}

void UGL_AnimInstance::RefreshTurnInPlace()
{
	TurnInPlaceState.bUpdatedThisFrame = true;

	// Sample curves ONCE this frame (after Slot, before RotateRootBone).
	const float IsTurning = GetCurveValue(TurnInPlace.CurveIsTurningName);
	const float DistanceCurveSigned = GetCurveValue(TurnInPlace.CurveTurnYawDistanceName); // -90..0 (L), +90..0 (R)

	// Accumulate actor yaw only when NOT turning (single writer -> no FP jitter).
	const float CurrentActorYaw = LocomotionState.Rotation.Yaw;
	const float ActorYawDelta = FMath::UnwindDegrees(CurrentActorYaw - TurnInPlaceState.PreviousActorYaw);
	TurnInPlaceState.PreviousActorYaw = CurrentActorYaw;

	if (IsTurning <= 0.f)
	{
		TurnInPlaceState.RootYawOffset -= ActorYawDelta;
		TurnInPlaceState.RootYawOffset = FMath::Clamp(
			TurnInPlaceState.RootYawOffset,
			-TurnInPlace.MaxIdleRootYawOffsetAbs, TurnInPlace.MaxIdleRootYawOffsetAbs);
	}

	const bool bGrounded = LocomotionMode.MatchesTagExact(GameplayLocomotionModeTags::Grounded);
	const bool bIdle = !LocomotionState.bMoving && LocomotionState.Speed <= General.MovingSmoothSpeedThreshold;

	// Consume signed curve while turning to drive RYO toward 0; early-stop to avoid overshoot.
	if (IsTurning > 0.f)
	{
		// Freeze direction & initialize on first turning frame.
		if (TurnInPlaceState.TurnDirection == 0)
		{
			TurnInPlaceState.TurnDirection = (TurnInPlaceState.RootYawOffset < 0.f) ? -1 : +1;
			TurnInPlaceState.InitialRootYawAbs = FMath::Abs(TurnInPlaceState.RootYawOffset);
			TurnInPlaceState.ConsumedYawAbs = 0.f;
			TurnInPlaceState.PreviousDistanceCurveSigned = DistanceCurveSigned;
		}

		const float Prev = TurnInPlaceState.PreviousDistanceCurveSigned;
		const float Cur = DistanceCurveSigned;

		// Magnitude delta (always >= 0 when progressing), direction is frozen.
		const float PrevAbs = FMath::Abs(Prev);
		const float CurAbs = FMath::Abs(Cur);
		const float ConsumedThisFrame = FMath::Max(0.f, PrevAbs - CurAbs);

		// Apply consumption: subtract signed amount from RootYawOffset.
		TurnInPlaceState.RootYawOffset -= (float)TurnInPlaceState.TurnDirection * ConsumedThisFrame;

		TurnInPlaceState.ConsumedYawAbs += ConsumedThisFrame;
		TurnInPlaceState.PreviousDistanceCurveSigned = Cur;
		TurnInPlaceState.bTurning = true;

		// Early stop once we've consumed the needed yaw (plus small tolerance).
		const float Need = TurnInPlaceState.InitialRootYawAbs;
		if (TurnInPlaceState.ConsumedYawAbs + TurnInPlace.EarlyStopToleranceDeg >= Need)
		{
			TurnInPlaceState.RootYawOffset = 0.f;
			if (TurnInPlaceState.ActiveMontage && Montage_IsPlaying(TurnInPlaceState.ActiveMontage))
			{
				Montage_Stop(TurnInPlace.BlendDuration/* * 0.5f*/, TurnInPlaceState.ActiveMontage);
			}

			TurnInPlaceState.bTurning = false;
			TurnInPlaceState.TurnDirection = 0;
			TurnInPlaceState.PreviousDistanceCurveSigned = 0.f;
			TurnInPlaceState.ActiveMontage = nullptr;
		}

		return; // while turning, don't try to start another
	}
	else
	{
		TurnInPlaceState.bTurning = false;
		TurnInPlaceState.TurnDirection = 0;
		TurnInPlaceState.PreviousDistanceCurveSigned = 0.f;
	}

	// Countdown delay if set.
	if (TurnInPlaceState.ActivationDelay > 0.f)
	{
		TurnInPlaceState.ActivationDelay -= GetDeltaSeconds();
	}

	// Trigger gate (unchanged threshold).
	const float RootYawAbs = FMath::Abs(TurnInPlaceState.RootYawOffset);
	if (!(bGrounded && bIdle && RootYawAbs > TurnInPlace.RootYawOffsetThreshold))
	{
		TurnInPlaceState.ActivationDelay = 0.f;
		return;
	}
	if (TurnInPlaceState.ActivationDelay > 0.f)
	{
		return;
	}
	if (TurnInPlaceState.ActivationDelay <= 0.f)
	{
		TurnInPlaceState.ActivationDelay = TurnInPlace.RootYawOffsetToActivationDelay;
	}

	// Select clip and start 90° montage.
	const bool bLeft = (TurnInPlaceState.RootYawOffset < 0.f);
	const bool bCrouch = Stance.MatchesTagExact(GameplayStanceTags::Crouching);

	const FGL_TurnInPlaceAnimSettings& Clip =
		bCrouch
		? (bLeft ? CrouchingTurn90Left : CrouchingTurn90Right)
		: (bLeft ? StandingTurn90Left : StandingTurn90Right);

	if (!Clip.Sequence) return;

	TurnInPlaceState.QueuedTurnYawAngle = bLeft ? -90.f : +90.f;

	float PlayRate = Clip.PlayRate;
	if (Clip.bScalePlayRateByAnimatedTurnAngle && Clip.AnimatedTurnAngle > KINDA_SMALL_NUMBER)
	{
		PlayRate *= (FMath::Abs(TurnInPlaceState.QueuedTurnYawAngle) / Clip.AnimatedTurnAngle);
	}
	TurnInPlaceState.PlayRate = PlayRate;

	const FName& TurnSlotName = bCrouch ? TurnInPlace.TurnSlotNameCrouching : TurnInPlace.TurnSlotNameStanding;

	UAnimMontage* Montage = PlaySlotAnimationAsDynamicMontage(
		Clip.Sequence, TurnSlotName,
		/*BlendInTime*/  TurnInPlace.BlendDuration,
		/*BlendOutTime*/ TurnInPlace.BlendDuration,
		/*PlayRate*/     PlayRate,
		/*LoopCount*/    1, /*BlendOutTriggerTime*/ 0.f, /*StartTime*/ 0.f);

	if (Montage)
	{
		TurnInPlaceState.QueuedSequence = Clip.Sequence;
		TurnInPlaceState.QueuedSlotName = TurnSlotName;
		TurnInPlaceState.ActiveMontage = Montage;

		// Seed signed curve and initial yaw for next-frame consumption.
		TurnInPlaceState.PreviousDistanceCurveSigned = DistanceCurveSigned;
		TurnInPlaceState.InitialRootYawAbs = RootYawAbs;
		TurnInPlaceState.ConsumedYawAbs = 0.f;
		TurnInPlaceState.TurnDirection = bLeft ? -1 : +1;
	}
}

void UGL_AnimInstance::InitializeLean()
{
	LeanState.RightAmount = 0.0f;
	LeanState.ForwardAmount = 0.0f;
}

// ----------------- Internals -----------------

void UGL_AnimInstance::RefreshMovementBaseOnGameThread()
{
	const auto& Based = Character->GetBasedMovement();

	if (Based.MovementBase != MovementBase.Primitive || Based.BoneName != MovementBase.BoneName)
	{
		MovementBase.Primitive = Based.MovementBase;
		MovementBase.BoneName = Based.BoneName;
		MovementBase.bBaseChanged = true;
	}
	else
	{
		MovementBase.bBaseChanged = false;
	}

	MovementBase.bHasRelativeLocation = Based.HasRelativeLocation();
	MovementBase.bHasRelativeRotation = MovementBase.bHasRelativeLocation && Based.bRelativeRotation;

	const FQuat PrevRot = MovementBase.Rotation;

	MovementBaseUtility::GetMovementBaseTransform(Based.MovementBase, Based.BoneName,
		MovementBase.Location, MovementBase.Rotation);

	MovementBase.DeltaRotation = (MovementBase.bHasRelativeLocation && !MovementBase.bBaseChanged)
		? (MovementBase.Rotation * PrevRot.Inverse()).Rotator()
		: FRotator::ZeroRotator;
}

void UGL_AnimInstance::RefreshLocomotionOnGameThread()
{
	const UWorld* World = GetWorld();
	const float ActorDt = (IsValid(World) ? World->GetDeltaSeconds() * Character->CustomTimeDilation : 0.f);
	const bool bCanRateOfChange = !bPendingUpdate && ActorDt > UE_SMALL_NUMBER;

	// Pull from character’s runtime locomotion state (populated by movement)
	const auto& Loco = Character->LocomotionState;

	LocomotionState.bHasInput = Loco.bHasInput;
	LocomotionState.InputYawAngle = Loco.InputYawAngle;

	const FVector PrevVelocity = LocomotionState.Velocity;

	LocomotionState.Velocity = Loco.Velocity;
	LocomotionState.Speed = Loco.Velocity.Size2D();
	LocomotionState.VelocityYawAngle = Loco.VelocityYawAngle;

	LocomotionState.Acceleration = bCanRateOfChange ? (LocomotionState.Velocity - PrevVelocity) / FMath::Max(ActorDt, UE_SMALL_NUMBER)
		: FVector::ZeroVector;

	const UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	LocomotionState.MaxAcceleration = Move->GetMaxAcceleration();
	LocomotionState.MaxBrakingDeceleration = Move->GetMaxBrakingDeceleration();
	LocomotionState.WalkableFloorAngleCos = Move->GetWalkableFloorZ();

	LocomotionState.bMoving = (LocomotionState.Speed > UE_SMALL_NUMBER);
	LocomotionState.bMovingSmooth = (Loco.bHasInput && LocomotionState.bMoving) ||
		LocomotionState.Speed > General.MovingSmoothSpeedThreshold;

	// Rotation / smoothing (no custom network smoothing here)
	const FTransform ActorXf = Character->GetActorTransform();
	LocomotionState.Location = ActorXf.GetLocation();
	LocomotionState.Rotation = ActorXf.Rotator();
	LocomotionState.RotationQuaternion = ActorXf.GetRotation();

	const float PrevYaw = LocomotionState.YawSpeed;
	LocomotionState.YawSpeed = bCanRateOfChange
		? FMath::UnwindDegrees(UE_REAL_TO_FLOAT(LocomotionState.Rotation.Yaw - PrevYaw)) / FMath::Max(ActorDt, UE_SMALL_NUMBER)
		: 0.f;

	LocomotionState.Scale = UE_REAL_TO_FLOAT(GetSkelMeshComponent()->GetComponentScale().Z);

	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	LocomotionState.CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	LocomotionState.CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	LocomotionState.TargetYawAngle = Loco.VelocityYawAngle; // simple target (can be refined)
}

void UGL_AnimInstance::RefreshViewOnGameThread()
{
	// Use controller / camera rotation.
	const FRotator ViewRot = Character->IsLocallyControlled() && Character->IsPlayerControlled() ? Character->GetViewRotation() : Character->GetBaseAimRotation();
	ViewState.Rotation = ViewRot;
}

void UGL_AnimInstance::RefreshView(const float DeltaSeconds)
{
	// Derive yaw/pitch deltas vs actor rotation
	ViewState.YawAngle = FMath::UnwindDegrees(UE_REAL_TO_FLOAT(ViewState.Rotation.Yaw - LocomotionState.Rotation.Yaw));
	ViewState.PitchAngle = FMath::UnwindDegrees(UE_REAL_TO_FLOAT(ViewState.Rotation.Pitch - LocomotionState.Rotation.Pitch));
	ViewState.PitchAmount = 0.5f - ViewState.PitchAngle / 180.f;

	// YawSpeed from control rotation delta
	// NOTE: if you want camera-component-based yaw speed, expose it and read here.
	static float LastViewYaw = 0.f;
	const float CurrentViewYaw = ViewState.Rotation.Yaw;
	const float DeltaYaw = FMath::UnwindDegrees(CurrentViewYaw - LastViewYaw);
	ViewState.YawSpeed = (DeltaSeconds > UE_SMALL_NUMBER) ? (DeltaYaw / DeltaSeconds) : 0.f;
	LastViewYaw = CurrentViewYaw;

	// LookAmount could be blended with an anim curve; for now: 0..1 gate (no aim system here)
	ViewState.LookAmount = 1.f;
}

FVector3f UGL_AnimInstance::GetRelativeVelocity() const
{
	return FVector3f{ LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Velocity) };
}

FVector2f UGL_AnimInstance::GetRelativeAccelerationAmount() const
{
	const float MaxA = ((LocomotionState.Acceleration | LocomotionState.Velocity) >= 0.f)
		? LocomotionState.MaxAcceleration
		: LocomotionState.MaxBrakingDeceleration;

	if (MaxA <= UE_KINDA_SMALL_NUMBER) { return FVector2f::ZeroVector; }

	const FVector3f RelA = FVector3f{ LocomotionState.RotationQuaternion.UnrotateVector(LocomotionState.Acceleration) };
	return FVector2f{ UGL_Vector::ClampMagnitude01(RelA / MaxA) };
}

void UGL_AnimInstance::RefreshVelocityBlend()
{
	auto& VB = GroundedState.VelocityBlend;

	FVector3f RelVel = GetRelativeVelocity();
	FVector3f Target = FVector3f::ZeroVector;

	if (RelVel.Normalize())
	{
		Target = RelVel / (FMath::Abs(RelVel.X) + FMath::Abs(RelVel.Y) + FMath::Abs(RelVel.Z));
	}

	if (VB.bInitializationRequired || Grounded.VelocityBlendInterpolationHalfLife <= 0.f)
	{
		VB.bInitializationRequired = false;
		VB.ForwardAmount = UGL_Math::Clamp01(Target.X);
		VB.BackwardAmount = FMath::Abs(FMath::Clamp(Target.X, -1.f, 0.f));
		VB.LeftAmount = FMath::Abs(FMath::Clamp(Target.Y, -1.f, 0.f));
		VB.RightAmount = UGL_Math::Clamp01(Target.Y);
	}
	else
	{
		const float Alpha = UGL_Math::DamperExactAlpha(GetDeltaSeconds(), Grounded.VelocityBlendInterpolationHalfLife);
		VB.ForwardAmount = FMath::Lerp(VB.ForwardAmount, UGL_Math::Clamp01(Target.X), Alpha);
		VB.BackwardAmount = FMath::Lerp(VB.BackwardAmount, FMath::Abs(FMath::Clamp(Target.X, -1.f, 0.f)), Alpha);
		VB.LeftAmount = FMath::Lerp(VB.LeftAmount, FMath::Abs(FMath::Clamp(Target.Y, -1.f, 0.f)), Alpha);
		VB.RightAmount = FMath::Lerp(VB.RightAmount, UGL_Math::Clamp01(Target.Y), Alpha);
	}
}

void UGL_AnimInstance::RefreshGroundedLean()
{
	const FVector2f Target = GetRelativeAccelerationAmount();

	if (bPendingUpdate || General.LeanInterpolationHalfLife <= 0.f)
	{
		LeanState.RightAmount = Target.Y;
		LeanState.ForwardAmount = Target.X;
	}
	else
	{
		const float Alpha = UGL_Math::DamperExactAlpha(GetDeltaSeconds(), General.LeanInterpolationHalfLife);
		LeanState.RightAmount = FMath::Lerp(LeanState.RightAmount, Target.Y, Alpha);
		LeanState.ForwardAmount = FMath::Lerp(LeanState.ForwardAmount, Target.X, Alpha);
	}
}

void UGL_AnimInstance::RefreshPoseState()
{
	const auto& Curves =
		GameplayGetAnimationCurvesAccessor::Invoke(GetProxyOnAnyThread<FAnimInstanceProxy>(), EAnimCurveType::AttributeCurve);

	const auto GetCurveValue = [](const TMap<FName, float>& InCurves, const FName& CurveName) -> float
	{
		if (const float* Value = InCurves.Find(CurveName))
		{
			return *Value;
		}
		return 0.0f;
	};

	// Pose weights (directly from curves, ALS-style)
	PoseState.GroundedAmount = GetCurveValue(Curves, UGL_Constants::PoseGroundedCurveName());
	PoseState.InAirAmount = GetCurveValue(Curves, UGL_Constants::PoseInAirCurveName());

	PoseState.StandingAmount = GetCurveValue(Curves, UGL_Constants::PoseStandingCurveName());
	PoseState.CrouchingAmount = GetCurveValue(Curves, UGL_Constants::PoseCrouchingCurveName());

	PoseState.MovingAmount = GetCurveValue(Curves, UGL_Constants::PoseMovingCurveName());

	// Gait (0..3), with weighted and unweighted channels
	PoseState.GaitAmount = FMath::Clamp(GetCurveValue(Curves, UGL_Constants::PoseGaitCurveName()), 0.0f, 3.0f);
	PoseState.GaitWalkingAmount = UGL_Math::Clamp01(PoseState.GaitAmount);
	PoseState.GaitRunningAmount = UGL_Math::Clamp01(PoseState.GaitAmount - 1.0f);
	PoseState.GaitSprintingAmount = UGL_Math::Clamp01(PoseState.GaitAmount - 2.0f);

	// Unweight by grounded amount (instant full gait on grounded transitions)
	PoseState.UnweightedGaitAmount = PoseState.GroundedAmount > UE_SMALL_NUMBER
		? (PoseState.GaitAmount / PoseState.GroundedAmount)
		: PoseState.GaitAmount;

	PoseState.UnweightedGaitWalkingAmount = UGL_Math::Clamp01(PoseState.UnweightedGaitAmount);
	PoseState.UnweightedGaitRunningAmount = UGL_Math::Clamp01(PoseState.UnweightedGaitAmount - 1.0f);
	PoseState.UnweightedGaitSprintingAmount = UGL_Math::Clamp01(PoseState.UnweightedGaitAmount - 2.0f);
}

void UGL_AnimInstance::RefreshMovementDirection(const float ViewRelativeVelocityYawAngle)
{
	// Sprint or velocity-direction rules: always forward
	if (Gait == GameplayGaitTags::Sprinting)
	{
		GroundedState.MovementDirection = EGL_MovementDirection::Forward;
		return;
	}

	static constexpr float ForwardHalfAngle = 70.f;
	static constexpr float AngleThreshold = 5.f;

	GroundedState.MovementDirection = UGL_Math::CalculateMovementDirection(ViewRelativeVelocityYawAngle, ForwardHalfAngle, AngleThreshold);
}

void UGL_AnimInstance::RefreshRotationYawOffsets(const float ViewRelativeVelocityYawAngle)
{
	auto& O = GroundedState.RotationYawOffsets;

	O.ForwardAngle = Grounded.RotationYawOffsetForwardCurve ? Grounded.RotationYawOffsetForwardCurve->GetFloatValue(ViewRelativeVelocityYawAngle) : 0.f;
	O.BackwardAngle = Grounded.RotationYawOffsetBackwardCurve ? Grounded.RotationYawOffsetBackwardCurve->GetFloatValue(ViewRelativeVelocityYawAngle) : 0.f;
	O.LeftAngle = Grounded.RotationYawOffsetLeftCurve ? Grounded.RotationYawOffsetLeftCurve->GetFloatValue(ViewRelativeVelocityYawAngle) : 0.f;
	O.RightAngle = Grounded.RotationYawOffsetRightCurve ? Grounded.RotationYawOffsetRightCurve->GetFloatValue(ViewRelativeVelocityYawAngle) : 0.f;
}

void UGL_AnimInstance::RefreshInAirOnGameThread()
{
	InAirState.bJumped = !bPendingUpdate && (InAirState.bJumped || InAirState.bJumpRequested);
	InAirState.bJumpRequested = false;
}

void UGL_AnimInstance::RefreshGroundPrediction()
{
	// Optional: mimic ALS ground prediction (disabled if curves / settings are not provided)
	if (!InAir.GroundPredictionAmountCurve) { InAirState.GroundPredictionAmount = 0.f; return; }

	static constexpr float VerticalVelocityThreshold = -200.f;
	if (InAirState.VerticalVelocity > VerticalVelocityThreshold)
	{
		InAirState.GroundPredictionAmount = 0.f;
		return;
	}

	// Allow full amount by default (you can drive a block curve later if desired)
	const float Allowance = 1.f;
	if (Allowance <= UE_SMALL_NUMBER) { InAirState.GroundPredictionAmount = 0.f; return; }

	const FVector SweepStart = LocomotionState.Location;

	const float ClampedZ = FMath::Clamp(InAirState.VerticalVelocity, -4000.f, -200.f);
	FVector VelDir = LocomotionState.Velocity; VelDir.Z = ClampedZ; VelDir.Normalize();

	const float SweepDist = FMath::GetMappedRangeValueClamped(FVector2f{ -200.f, -4000.f }, FVector2f{ 150.f, 2000.f }, InAirState.VerticalVelocity);

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit, SweepStart, SweepStart + VelDir * SweepDist * LocomotionState.Scale, FQuat::Identity,
		InAir.GroundPredictionSweepChannel,
		FCollisionShape::MakeCapsule(LocomotionState.CapsuleRadius, LocomotionState.CapsuleHalfHeight),
		{ TEXT("UGL_AnimInstance::GroundPrediction"), false, Character },
		InAir.GroundPredictionSweepResponses);

	const bool bGroundValid = Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z >= LocomotionState.WalkableFloorAngleCos;

	InAirState.GroundPredictionAmount = bGroundValid
		? InAir.GroundPredictionAmountCurve->GetFloatValue(Hit.Time) * Allowance
		: 0.f;
}

void UGL_AnimInstance::RefreshInAirLean()
{
	if (!InAir.LeanAmountCurve) { LeanState.RightAmount = 0.f; LeanState.ForwardAmount = 0.f; return; }

	static constexpr float ReferenceSpeed = 350.f;
	const FVector3f RelVel = GetRelativeVelocity();
	const float     Mult = InAir.LeanAmountCurve->GetFloatValue(InAirState.VerticalVelocity);

	const FVector2f Target{ (RelVel.X / ReferenceSpeed) * Mult, (RelVel.Y / ReferenceSpeed) * Mult };

	if (bPendingUpdate || General.LeanInterpolationHalfLife <= 0.f)
	{
		LeanState.RightAmount = Target.Y;
		LeanState.ForwardAmount = Target.X;
	}
	else
	{
		const float Alpha = UGL_Math::DamperExactAlpha(GetDeltaSeconds(), General.LeanInterpolationHalfLife);
		LeanState.RightAmount = FMath::Lerp(LeanState.RightAmount, Target.Y, Alpha);
		LeanState.ForwardAmount = FMath::Lerp(LeanState.ForwardAmount, Target.X, Alpha);
	}
}

void UGL_AnimInstance::RefreshLayering()
{
	const auto& Curves =
		GameplayGetAnimationCurvesAccessor::Invoke(GetProxyOnAnyThread<FAnimInstanceProxy>(), EAnimCurveType::AttributeCurve);

	const auto GetCurveValue = [](const TMap<FName, float>& InCurves, const FName& CurveName) -> float
	{
		if (const float* Value = InCurves.Find(CurveName))
		{
			return *Value;
		}
		return 0.0f;
	};

	LayeringState.HeadBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerHeadCurveName());
	LayeringState.HeadAdditiveBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerHeadAdditiveCurveName());
	LayeringState.HeadSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerHeadSlotCurveName());

	// The mesh space blend will always be 1 unless the local space blend is 1.

	LayeringState.ArmLeftBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmLeftCurveName());
	LayeringState.ArmLeftAdditiveBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmLeftAdditiveCurveName());
	LayeringState.ArmLeftSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmLeftSlotCurveName());
	LayeringState.ArmLeftLocalSpaceBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmLeftLocalSpaceCurveName());
	LayeringState.ArmLeftMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(LayeringState.ArmLeftLocalSpaceBlendAmount);

	// The mesh space blend will always be 1 unless the local space blend is 1.

	LayeringState.ArmRightBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmRightCurveName());
	LayeringState.ArmRightAdditiveBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmRightAdditiveCurveName());
	LayeringState.ArmRightSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmRightSlotCurveName());
	LayeringState.ArmRightLocalSpaceBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerArmRightLocalSpaceCurveName());
	LayeringState.ArmRightMeshSpaceBlendAmount = !FAnimWeight::IsFullWeight(LayeringState.ArmRightLocalSpaceBlendAmount);

	LayeringState.HandLeftBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerHandLeftCurveName());
	LayeringState.HandRightBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerHandRightCurveName());

	LayeringState.SpineBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerSpineCurveName());
	LayeringState.SpineAdditiveBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerSpineAdditiveCurveName());
	LayeringState.SpineSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerSpineSlotCurveName());

	LayeringState.PelvisBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerPelvisCurveName());
	LayeringState.PelvisSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerPelvisSlotCurveName());

	LayeringState.LegsBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerLegsCurveName());
	LayeringState.LegsSlotBlendAmount = GetCurveValue(Curves, UGL_Constants::LayerLegsSlotCurveName());
}
