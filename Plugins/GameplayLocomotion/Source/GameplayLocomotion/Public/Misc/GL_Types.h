#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "GL_Types.generated.h"

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_LocomotionState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	bool bHasInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(ClampMin=-180, ClampMax=180, ForceUnits="deg"))
	float InputYawAngle = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(ClampMin=-180, ClampMax=180, ForceUnits="deg"))
	float VelocityYawAngle = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	FVector Velocity = FVector::ZeroVector;
};

UENUM(BlueprintType)
enum class EGL_MovementDirection : uint8
{
	Forward,
	Backward,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_MovementDirectionCache
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayLocomotion")
	uint8 bForward : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayLocomotion")
	uint8 bBackward : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayLocomotion")
	uint8 bLeft : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameplayLocomotion")
	uint8 bRight : 1 {false};

public:
	constexpr FGL_MovementDirectionCache() = default;

	constexpr FGL_MovementDirectionCache(const EGL_MovementDirection MovementDirection)
	{
		bForward = MovementDirection == EGL_MovementDirection::Forward;
		bBackward = MovementDirection == EGL_MovementDirection::Backward;
		bLeft = MovementDirection == EGL_MovementDirection::Left;
		bRight = MovementDirection == EGL_MovementDirection::Right;
	}

	FGL_MovementDirectionCache& operator=(EGL_MovementDirection MovementDirection)
	{
		bForward = MovementDirection == EGL_MovementDirection::Forward;
		bBackward = MovementDirection == EGL_MovementDirection::Backward;
		bLeft = MovementDirection == EGL_MovementDirection::Left;
		bRight = MovementDirection == EGL_MovementDirection::Right;

		return *this;
	}

	operator EGL_MovementDirection() const
	{
		if (bForward)  return EGL_MovementDirection::Forward;
		if (bBackward) return EGL_MovementDirection::Backward;
		if (bLeft)     return EGL_MovementDirection::Left;
		if (bRight)    return EGL_MovementDirection::Right;

		return EGL_MovementDirection::Forward;
	}
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_AnimGeneralSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="General")
	uint8 bUseHandIkBones : 1 { true };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="General")
	uint8 bUseFootIkBones : 1 { true };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="General", meta=(ClampMin=0))
	float LeanInterpolationHalfLife = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="General", meta=(ClampMin=0))
	float MovingSmoothSpeedThreshold = 150.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_AnimGroundedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grounded", meta=(ClampMin=0))
	float VelocityBlendInterpolationHalfLife = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grounded")
	TObjectPtr<UCurveFloat> RotationYawOffsetForwardCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grounded")
	TObjectPtr<UCurveFloat> RotationYawOffsetBackwardCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grounded")
	TObjectPtr<UCurveFloat> RotationYawOffsetLeftCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grounded")
	TObjectPtr<UCurveFloat> RotationYawOffsetRightCurve;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_AnimStandingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing")
	TObjectPtr<UCurveFloat> StrideBlendAmountWalkCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing")
	TObjectPtr<UCurveFloat> StrideBlendAmountRunCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing", meta=(ClampMin=1))
	float AnimatedWalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing", meta=(ClampMin=1))
	float AnimatedRunSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing", meta=(ClampMin=1))
	float AnimatedSprintSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Standing", meta=(ClampMin=0))
	float PivotActivationSpeedThreshold = 200.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_AnimCrouchingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crouching")
	TObjectPtr<UCurveFloat> StrideBlendAmountCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Crouching", meta=(ClampMin=1))
	float AnimatedCrouchSpeed = 150.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_AnimInAirSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InAir")
	TEnumAsByte<ECollisionChannel> GroundPredictionSweepChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
	TArray<TEnumAsByte<ECollisionChannel>> GroundPredictionResponseChannels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InAir")
	FCollisionResponseContainer GroundPredictionSweepResponses = FCollisionResponseContainer(ECR_Ignore);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InAir")
	TObjectPtr<UCurveFloat> GroundPredictionAmountCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="InAir")
	TObjectPtr<UCurveFloat> LeanAmountCurve;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_TurnInPlaceAnimSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	TObjectPtr<UAnimSequenceBase> Sequence = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0, ForceUnits="x"))
	float PlayRate = 1.2f;

	// If true, scales PlayRate by |DesiredTurnAngle| / AnimatedTurnAngle.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	uint8 bScalePlayRateByAnimatedTurnAngle : 1 { true };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0.0001, ClampMax=180, ForceUnits="deg"))
	float AnimatedTurnAngle = 90.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_TurnInPlaceGeneralSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0, ClampMax=180, ForceUnits="deg"))
	float RootYawOffsetThreshold = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	float RootYawOffsetToActivationDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0, ForceUnits="s"))
	float BlendDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	FName TurnSlotNameStanding = TEXT("TurnInPlaceStanding");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	FName TurnSlotNameCrouching = TEXT("TurnInPlaceCrouching");

	// Curve names authored on the turn sequences.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	FName CurveIsTurningName = TEXT("IsTurn");

	// Curve whose magnitude goes from |Angle| -> 0 over the course of the anim.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace")
	FName CurveTurnYawDistanceName = TEXT("DistanceCurve");

	// Clamp for accumulated RootYawOffset while idle.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0, ClampMax=90, ForceUnits="deg"))
	float MaxIdleRootYawOffsetAbs = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TurnInPlace", meta=(ClampMin=0, ClampMax=15, ForceUnits="deg"))
	float EarlyStopToleranceDeg = 3.f;
};

// -------------- State structs --------------
USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_MovementBaseState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> Primitive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName BoneName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bBaseChanged : 1 { false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bHasRelativeLocation : 1 { false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bHasRelativeRotation : 1 { false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FQuat Rotation = FQuat::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator DeltaRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_VelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bInitializationRequired : 1 { true };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ForwardAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BackwardAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LeftAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RightAmount = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_RotationYawOffsets
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ForwardAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float BackwardAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float LeftAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float RightAngle = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_GroundedState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGL_VelocityBlend VelocityBlend;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGL_RotationYawOffsets RotationYawOffsets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGL_MovementDirectionCache MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float HipsDirectionLockAmount = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_StandingState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float StrideBlendAmount = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float WalkRunBlendAmount = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PlayRate = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SprintBlockAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SprintTime = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SprintAccelerationAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bPivotActive : 1 { false };
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_PoseState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GroundedAmount = 1.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float InAirAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float StandingAmount = 1.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float CrouchingAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float MovingAmount = 0.0f;

	// 0..3 where 0=stopped, 1=walk, 2=run, 3=sprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GaitAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GaitWalkingAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GaitRunningAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GaitSprintingAmount = 0.0f;

	// Unweighted (one-hot by chosen gait tag), plus 0..3 unweighted scalar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float UnweightedGaitAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float UnweightedGaitWalkingAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float UnweightedGaitRunningAmount = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float UnweightedGaitSprintingAmount = 0.0f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_CrouchingState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float StrideBlendAmount = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PlayRate = 1.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_ViewAnimState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float YawSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float YawAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PitchAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PitchAmount = 0.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float LookAmount = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_LocomotionAnimState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Location = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FQuat RotationQuaternion = FQuat::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float YawSpeed = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Speed = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Scale = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float CapsuleRadius = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float CapsuleHalfHeight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bHasInput : 1 { false };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bMoving : 1 { false };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bMovingSmooth : 1 { false };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float InputYawAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float VelocityYawAngle = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Velocity = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float MaxAcceleration = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float MaxBrakingDeceleration = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float WalkableFloorAngleCos = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float TargetYawAngle = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_LeanState
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float RightAmount = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ForwardAmount = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_InAirState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bJumped : 1 { false };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bJumpRequested : 1 { false };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float JumpPlayRate = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float VerticalVelocity = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float GroundPredictionAmount = 0.f;
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_TurnInPlaceState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bUpdatedThisFrame : 1 { false };

	// Queuing & timing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ForceUnits="s"))
	float ActivationDelay = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimSequenceBase> QueuedSequence = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName QueuedSlotName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ClampMin=0, ClampMax=180, ForceUnits="deg"))
	float QueuedTurnYawAngle = 0.f; // signed (+ right, - left)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(ClampMin=0, ForceUnits="x"))
	float PlayRate = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float InitialRootYawAbs = 0.f;          // abs offset at the moment the turn starts

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ConsumedYawAbs = 0.f;             // how much of that offset we've consumed via the curve

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PreviousDistanceCurveSigned = 0.f;// last frame's signed curve sample (-90..0 or +90..0)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TurnDirection = 0;                // -1 left, +1 right (chosen when starting the turn)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> ActiveMontage; // montage we started for TIP (for early stop)

	// Runtime tracking
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float RootYawOffset = 0.f; // signed accumulator
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PreviousActorYaw = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PreviousDistanceCurve = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) uint8 bTurning : 1 { false };
};

USTRUCT(BlueprintType)
struct GAMEPLAYLOCOMOTION_API FGL_LayeringState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HeadBlendAmount = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HeadAdditiveBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HeadSlotBlendAmount{1.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmLeftBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmLeftAdditiveBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmLeftSlotBlendAmount{1.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmLeftLocalSpaceBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmLeftMeshSpaceBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmRightBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmRightAdditiveBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmRightSlotBlendAmount{1.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmRightLocalSpaceBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float ArmRightMeshSpaceBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HandLeftBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HandRightBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SpineBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SpineAdditiveBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float SpineSlotBlendAmount{1.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PelvisBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PelvisSlotBlendAmount{1.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float LegsBlendAmount{0.0f};
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float LegsSlotBlendAmount{1.0f};
};
