#include "Character/GL_Character.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"

#include "Components/GL_CharacterMovementComponent.h"
#include "Misc/GL_GameplayTags.h"
#include "Animation/GL_AnimInstance.h"

AGL_Character::AGL_Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGL_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;
	bClientCheckEncroachmentOnNetUpdate = true; // Required for bSimGravityDisabled to be updated.

	GetCapsuleComponent()->InitCapsuleSize(30.0f, 90.0f);

	if (IsValid(GetMesh()))
	{
		GetMesh()->SetRelativeLocation_Direct({ 0.0f, 0.0f, -92.0f });
		GetMesh()->SetRelativeRotation_Direct({ 0.0f, -90.0f, 0.0f });

		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
		GetMesh()->bEnableUpdateRateOptimizations = false;
	}

	// Defaults (tags assumed to exist natively in your project)
	DesiredStance = GameplayStanceTags::Standing;
	DesiredGait = GameplayGaitTags::Walking;
	ViewMode = GameplayViewModeTags::ThirdPerson;
	OverlayMode = GameplayOverlayModeTags::Default;

	Stance = DesiredStance;
	Gait = GameplayGaitTags::Walking;
	LocomotionMode = GameplayLocomotionModeTags::Grounded;
}


UGL_CharacterMovementComponent* AGL_Character::GLMovement() const
{
	return CastChecked<UGL_CharacterMovementComponent>(GetCharacterMovement());
}

void AGL_Character::SetDesiredStance(const FGameplayTag& NewStance)
{
	if (DesiredStance == NewStance) { return; }

	if (HasAuthority())
	{
		DesiredStance = NewStance;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredStance, this);

		ApplyDesiredStance();
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		DesiredStance = NewStance;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredStance, this);

		ApplyDesiredStance();
		ServerSetDesiredStance(NewStance);
	}
}

void AGL_Character::SetDesiredGait(const FGameplayTag& NewGait)
{
	if (DesiredGait == NewGait) { return; }

	if (HasAuthority())
	{
		DesiredGait = NewGait;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredGait, this);
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		DesiredGait = NewGait;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredGait, this);

		ServerSetDesiredGait(NewGait);
	}
}

void AGL_Character::SetViewModeTag(const FGameplayTag& NewViewMode)
{
	if (ViewMode == NewViewMode) { return; }

	if (HasAuthority())
	{
		ViewMode = NewViewMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, ViewMode, this);

		OnRep_ViewMode();
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ViewMode = NewViewMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, ViewMode, this);

		OnRep_ViewMode();

		ServerSetViewMode(NewViewMode);
	}
}

void AGL_Character::SetOverlayModeTag(const FGameplayTag& NewOverlayMode)
{
	if (OverlayMode == NewOverlayMode) { return; }

	const FGameplayTag PreviousOverlayMode = OverlayMode;

	if (HasAuthority())
	{
		OverlayMode = NewOverlayMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, OverlayMode, this);

		OnRep_OverlayMode(PreviousOverlayMode);
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		OverlayMode = NewOverlayMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, OverlayMode, this);

		OnOverlayModeChanged(PreviousOverlayMode);

		ServerSetOverlayMode(NewOverlayMode);
	}
}

void AGL_Character::SetAiming(bool bNewAiming)
{
	if (bAiming == bNewAiming) { return; }

	if (HasAuthority())
	{
		bAiming = bNewAiming;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, bAiming, this);
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		const bool bPreviousAiming = bAiming;

		bAiming = bNewAiming;
		MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, bAiming, this);

		OnAimingChanged(bPreviousAiming);

		ServerSetAiming(bNewAiming);
	}
}

void AGL_Character::StartRagdolling()
{
	if (GetLocalRole() <= ROLE_SimulatedProxy || !IsRagdollingAllowedToStart())
	{
		return;
	}

	if (GetLocalRole() >= ROLE_Authority)
	{
		MulticastStartRagdolling();
	}
	else
	{
		GetCharacterMovement()->FlushServerMoves();

		ServerStartRagdolling();
	}
}

bool AGL_Character::StopRagdolling()
{
	if (GetLocalRole() <= ROLE_SimulatedProxy || !IsRagdollingAllowedToStop())
	{
		return false;
	}

	if (GetLocalRole() >= ROLE_Authority)
	{
		MulticastStopRagdolling();
	}
	else
	{
		ServerStopRagdolling();
	}

	return true;
}

void AGL_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_SkipOwner;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, DesiredStance, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, DesiredGait, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, ViewMode, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, OverlayMode, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, bAiming, Params);

	DOREPLIFETIME_WITH_PARAMS_FAST(AGL_Character, InputDirection, Params);
}

void AGL_Character::PreRegisterAllComponents()
{
	Super::PreRegisterAllComponents();

	// Make sure movement component has settings early for tick 0 access
	if (UGL_CharacterMovementComponent* Mv = GLMovement())
	{
		Mv->SetStance(Stance);
		if (MovementSettings) { Mv->SetMovementSettings(MovementSettings); }
	}
}

void AGL_Character::PostInitializeComponents()
{
	GetMesh()->AddTickPrerequisiteActor(this);

	Super::PostInitializeComponents();

	if (MovementSettings)
	{
		if (UGL_CharacterMovementComponent* Mv = GLMovement())
		{
			Mv->SetMovementSettings(MovementSettings);
		}
	}

	ApplyDesiredStance();
}

void AGL_Character::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	const float YawAngle = UE_REAL_TO_FLOAT(GetActorRotation().Yaw);

	LocomotionState.InputYawAngle = YawAngle;
	LocomotionState.VelocityYawAngle = YawAngle;
}

void AGL_Character::BeginPlay()
{
	Super::BeginPlay();

	OnOverlayModeChanged(OverlayMode);
}

void AGL_Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RefreshInput(DeltaSeconds);
	RefreshLocomotion(DeltaSeconds);
	RefreshGait();
}

void AGL_Character::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	switch (GetCharacterMovement()->MovementMode)
	{
		case MOVE_Walking:
		case MOVE_NavWalking:
			LocomotionMode = GameplayLocomotionModeTags::Grounded;
			break;
		case MOVE_Falling:
			LocomotionMode = GameplayLocomotionModeTags::InAir;
			break;
		default:
			LocomotionMode = FGameplayTag::EmptyTag;
			break;
	}

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
}

void AGL_Character::Jump()
{
	if (Stance == GameplayStanceTags::Standing /*&& !LocomotionAction.IsValid() */&& LocomotionMode == GameplayLocomotionModeTags::Grounded)
	{
		Super::Jump();
	}
}

void AGL_Character::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		OnJumpedNetworked();
	}
	else if (GetLocalRole() >= ROLE_Authority)
	{
		MulticastOnJumpedNetworked();
	}
}

void AGL_Character::RefreshInput(float DeltaSeconds)
{
	if (GetLocalRole() >= ROLE_AutonomousProxy)
	{
		const UCharacterMovementComponent* CM = GetCharacterMovement();
		const FVector Accel = CM->GetCurrentAcceleration();
		const float   MaxA = CM->GetMaxAcceleration();

		const FVector Dir = MaxA > KINDA_SMALL_NUMBER ? (Accel / MaxA) : FVector::ZeroVector;
		const FVector N = Dir.GetSafeNormal();

		if (!N.Equals(InputDirection))
		{
			InputDirection = N;
			MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, InputDirection, this);
		}
	}

	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;
	if (LocomotionState.bHasInput)
	{
		const float YawRad = FMath::Atan2(InputDirection.Y, InputDirection.X);
		LocomotionState.InputYawAngle = FMath::RadiansToDegrees(YawRad);
	}
}

void AGL_Character::RefreshLocomotion(float DeltaSeconds)
{
	const UCharacterMovementComponent* CM = GetCharacterMovement();

	// Velocity & speed (authoritative from movement)
	LocomotionState.Velocity = GetVelocity();
	const FVector Vel2D(LocomotionState.Velocity.X, LocomotionState.Velocity.Y, 0.0f);
	const float   Speed = Vel2D.Size();
	// If you keep a Speed field in LocomotionState, set it here:
	// LocomotionState.Speed = Speed;

	// Acceleration 2D for braking detection
	const FVector Acc2D(CM->GetCurrentAcceleration().X, CM->GetCurrentAcceleration().Y, 0.0f);
	const bool    bHasAccel = Acc2D.SizeSquared() > KINDA_SMALL_NUMBER;

	// Braking when acceleration opposes velocity
	const bool bHasVel2D = Speed > KINDA_SMALL_NUMBER;
	const bool bBraking = bHasAccel && bHasVel2D &&
		(FVector::DotProduct(Acc2D.GetSafeNormal(), Vel2D.GetSafeNormal()) < -0.2f);

	// Hysteresis thresholds (ALS-like). Scale from walk speed for consistency across projects.
	// Use MaxWalkSpeed if available; fall back to GetMaxSpeed() when modes change that value dynamically.
	const float RefWalk = (CM->IsWalking() || CM->MovementMode == MOVE_Walking)
		? CM->MaxWalkSpeed
		: CM->GetMaxSpeed();

	// Minimum floors keep behavior sane in editor PIE and edge configs.
	const float Enter = FMath::Max(40.0f, RefWalk * 0.35f);  // must reach this to re-align toward velocity
	const float Exit = FMath::Max(25.0f, Enter * 0.5f);     // below this, while braking, freeze yaw

	// Allow updates when:
	//  - accelerating in roughly the velocity direction, OR
	//  - clearly above the "exit" threshold (coasting), OR
	//  - above the "enter" threshold (fresh movement burst).
	bool bAccelTowardVel = false;
	if (bHasAccel && bHasVel2D)
	{
		const float dot = FVector::DotProduct(Acc2D.GetSafeNormal(), Vel2D.GetSafeNormal());
		bAccelTowardVel = (dot > 0.2f); // mildly aligned
	}

	const bool bAllowYawUpdate =
		(bAccelTowardVel && Speed >= Exit) ||                // actively steering up to or above exit
		(!bBraking && Speed >= Exit) ||                      // coasting but not braking, above exit
		(Speed >= Enter);                                    // any strong movement burst

	// Seed on first valid movement or update when allowed; otherwise keep last stable value.
	if (bAllowYawUpdate && bHasVel2D)
	{
		LocomotionState.VelocityYawAngle = FMath::RadiansToDegrees(FMath::Atan2(Vel2D.Y, Vel2D.X));
	}
	// else: preserve previous LocomotionState.VelocityYawAngle to avoid snaps when stopping
}

void AGL_Character::RefreshGait()
{
	if (const UGL_CharacterMovementComponent* MvC = Cast<UGL_CharacterMovementComponent>(GetCharacterMovement()))
	{
		const FGameplayTag MaxAllowedGait = CalculateMaxAllowedGait();

		if (MvC->GetMaxAllowedGait() != MaxAllowedGait)
		{
			if (UGL_CharacterMovementComponent* Mv = const_cast<UGL_CharacterMovementComponent*>(MvC))
			{
				Mv->SetMaxAllowedGait(MaxAllowedGait);
			}
		}
	}

	RefreshGaitFromSpeed();
}

void AGL_Character::RefreshGaitFromSpeed()
{
	const float Speed2D = GetVelocity().Size2D();
	const UGL_CharacterMovementComponent* Mv = GLMovement();
	if (!Mv) { return; }

	const float WalkMax = Mv->GetGaitSettings().GetMaxWalkSpeed();
	const float RunMax = Mv->GetGaitSettings().GetMaxRunSpeed();
	const float SprintMax = Mv->GetGaitSettings().SprintSpeed;

	const FGameplayTag WalkTag = GameplayGaitTags::Walking;
	const FGameplayTag RunTag = GameplayGaitTags::Running;
	const FGameplayTag SprintTag = GameplayGaitTags::Sprinting;

	if (Speed2D < RunMax - 10.0f) { Gait = WalkTag; }
	else if (Speed2D <= SprintMax - 10.0f) { Gait = RunTag; }
	else { Gait = SprintTag; }
}

void AGL_Character::ApplyDesiredStance()
{
	Stance = DesiredStance;

	if (Stance.MatchesTagExact(GameplayStanceTags::Crouching))
	{
		Crouch(false);
	}
	else
	{
		UnCrouch(false);
	}

	if (UGL_CharacterMovementComponent* Mv = GLMovement())
	{
		Mv->SetStance(Stance);
	}
}

FGameplayTag AGL_Character::CalculateMaxAllowedGait() const
{
	if (DesiredGait == GameplayGaitTags::Walking)
	{
		return GameplayGaitTags::Walking;
	}

	if (DesiredGait == GameplayGaitTags::Running)
	{
		if (CanRun())
		{
			return DesiredGait;
		}

		return GameplayGaitTags::Walking;
	}

	if (DesiredGait == GameplayGaitTags::Sprinting)
	{
		if (CanSprint())
		{
			return DesiredGait;
		}

		if (CanRun())
		{
			return GameplayGaitTags::Running;
		}

		return GameplayGaitTags::Walking;
	}

	return GameplayGaitTags::Walking;
}

bool AGL_Character::CanRun() const
{
	return true;
}

bool AGL_Character::CanSprint() const
{
	if (!LocomotionState.bHasInput || Stance != GameplayStanceTags::Standing)
	{
		return false;
	}

	static constexpr auto ViewRelativeAngleThreshold{ 50.0f };

	if (FMath::Abs(FMath::UnwindDegrees(UE_REAL_TO_FLOAT(LocomotionState.InputYawAngle - GetActorRotation().Yaw))) < ViewRelativeAngleThreshold)
	{
		return true;
	}

	return false;
}

bool AGL_Character::IsRagdollingAllowedToStart() const
{
	return LocomotionAction != GameplayLocomotionActionTags::Ragdolling;
}

bool AGL_Character::IsRagdollingAllowedToStop() const
{
	return LocomotionAction == GameplayLocomotionActionTags::Ragdolling;
}

void AGL_Character::StartRagdollingImplementation()
{
	if (!IsRagdollingAllowedToStart())
	{
		return;
	}

	GetMesh()->bUpdateJointsFromAnimation = true; // Required for the flail animation to work properly.

	if (!GetMesh()->IsRunningParallelEvaluation() && GetMesh()->GetBoneSpaceTransforms().Num() > 0)
	{
		GetMesh()->UpdateRBJointMotors();
	}

	// Stop any active montages.

	static constexpr float BlendOutDuration = 0.2f;

	GetMesh()->GetAnimInstance()->Montage_Stop(BlendOutDuration);

	// Disable movement corrections and reset network smoothing.

	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;

	// Detach the mesh so that character transformation changes will not affect it in any way.

	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Disable capsule collision and enable mesh physics simulation.

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	// Clear the character movement mode and set the locomotion action to ragdolling.

	GetCharacterMovement()->SetMovementMode(MOVE_None);

	LocomotionAction = GameplayLocomotionActionTags::Ragdolling;
}

void AGL_Character::StopRagdollingImplementation()
{
	if (!IsRagdollingAllowedToStop())
	{
		return;
	}

	const FTransform PelvisTransform = GetMesh()->GetSocketTransform(TEXT("pelvis"));
	const FRotator PelvisRotation = PelvisTransform.Rotator();

	// Disable mesh physics simulation and enable capsule collision.

	GetMesh()->bUpdateJointsFromAnimation = false;

	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = false;

	bool bGrounded;
	const FVector NewActorLocation = RagdollTraceGround(bGrounded);

	// Determine whether the ragdoll is facing upward or downward and set the actor rotation accordingly.

	const bool bRagdollFacingUpward = FMath::UnwindDegrees(PelvisRotation.Roll) <= 0.0f;

	FRotator NewActorRotation = GetActorRotation();
	NewActorRotation.Yaw = bRagdollFacingUpward ? PelvisRotation.Yaw - 180.0f : PelvisRotation.Yaw;

	SetActorLocationAndRotation(NewActorLocation, NewActorRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Attach the mesh back and restore its default relative location.

	const FTransform& ActorTransform = GetActorTransform();

	GetMesh()->SetWorldLocationAndRotationNoPhysics(
		ActorTransform.TransformPositionNoScale(GetBaseTranslationOffset()),
		ActorTransform.TransformRotation(GetBaseRotationOffset()).Rotator());

	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);

	if (GetMesh()->ShouldUseUpdateRateOptimizations())
	{
		// Disable URO for one frame to force the animation blueprint to update and get rid of the incorrect mesh pose.

		GetMesh()->bEnableUpdateRateOptimizations = false;

		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
			{
				GetMesh()->bEnableUpdateRateOptimizations = true;
			}));
	}

	// Restore the pelvis transform to the state it was in before we changed
	// the character and mesh transforms to keep its world transform unchanged.

	const FReferenceSkeleton& ReferenceSkeleton = GetMesh()->GetSkinnedAsset()->GetRefSkeleton();

	const int32 PelvisBoneIndex = ReferenceSkeleton.FindBoneIndex(TEXT("pelvis"));
	if (ensure(PelvisBoneIndex >= 0))
	{
		// We expect the pelvis bone to be the root bone or attached to it, so we can safely use the mesh transform here.
		// FinalRagdollPose.LocalTransforms[PelvisBoneIndex] = PelvisTransform.GetRelativeTransform(GetMesh()->GetComponentTransform());
	}

	if (bGrounded)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}

	LocomotionAction = FGameplayTag::EmptyTag;
}

FVector AGL_Character::RagdollTraceGround(bool& bGrounded) const
{
	const FVector& RagdollLocation = GetActorLocation();

	// We use a sphere sweep instead of a simple line trace to keep capsule
	// movement consistent between ragdolling and regular character movement.

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const auto CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector TraceStart(RagdollLocation.X, RagdollLocation.Y, RagdollLocation.Z + 2.0f * CapsuleRadius);
	const FVector TraceEnd(RagdollLocation.X, RagdollLocation.Y, RagdollLocation.Z - CapsuleHalfHeight + CapsuleRadius);

	const ECollisionChannel CollisionChannel = GetCharacterMovement()->UpdatedComponent->GetCollisionObjectType();

	FCollisionQueryParams QueryParameters(__FUNCTION__, false, this);
	FCollisionResponseParams CollisionResponses;
	GetCharacterMovement()->InitCollisionParams(QueryParameters, CollisionResponses);

	FHitResult Hit;
	bGrounded = GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, CollisionChannel, FCollisionShape::MakeSphere(CapsuleRadius), QueryParameters, CollisionResponses);

	return FVector(
		RagdollLocation.X, RagdollLocation.Y,
		bGrounded
		? Hit.Location.Z + CapsuleHalfHeight - CapsuleRadius + UCharacterMovementComponent::MIN_FLOOR_DIST
		: RagdollLocation.Z
	);
}

void AGL_Character::ServerSetDesiredStance_Implementation(FGameplayTag NewStance)
{
	DesiredStance = NewStance;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredStance, this);

	ApplyDesiredStance();
}

void AGL_Character::ServerSetDesiredGait_Implementation(FGameplayTag NewGait)
{
	DesiredGait = NewGait;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, DesiredGait, this);

	OnRep_DesiredGait();
}

void AGL_Character::ServerSetViewMode_Implementation(FGameplayTag NewViewMode)
{
	const FGameplayTag PreviousViewMode = ViewMode;

	ViewMode = NewViewMode;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, ViewMode, this);

	OnRep_ViewMode();
}

void AGL_Character::ServerSetOverlayMode_Implementation(FGameplayTag NewOverlayMode)
{
	const FGameplayTag PreviousOverlayMode = OverlayMode;

	OverlayMode = NewOverlayMode;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, OverlayMode, this);

	OnRep_OverlayMode(PreviousOverlayMode);
}

void AGL_Character::ServerSetAiming_Implementation(bool bNewAiming)
{
	const bool bPreviousAiming = bAiming;

	bAiming = bNewAiming;
	MARK_PROPERTY_DIRTY_FROM_NAME(AGL_Character, bAiming, this);

	OnRep_Aiming(bPreviousAiming);
}

void AGL_Character::ServerStartRagdolling_Implementation()
{
	if (IsRagdollingAllowedToStart())
	{
		MulticastStartRagdolling();
		ForceNetUpdate();
	}
}

void AGL_Character::MulticastStartRagdolling_Implementation()
{
	StartRagdollingImplementation();
}

void AGL_Character::ServerStopRagdolling_Implementation()
{
	if (IsRagdollingAllowedToStop())
	{
		MulticastStopRagdolling();
		ForceNetUpdate();
	}
}

void AGL_Character::MulticastStopRagdolling_Implementation()
{
	StopRagdollingImplementation();
}

void AGL_Character::OnRep_DesiredStance()
{
	ApplyDesiredStance();
}

void AGL_Character::OnRep_DesiredGait()
{
	
}

void AGL_Character::OnRep_ViewMode()
{

}

void AGL_Character::OnRep_OverlayMode(const FGameplayTag& PreviousOverlayMode)
{
	OnOverlayModeChanged(PreviousOverlayMode);
}

void AGL_Character::OnRep_Aiming(bool bPreviousAiming)
{
	OnAimingChanged(bPreviousAiming);
}

void AGL_Character::MulticastOnJumpedNetworked_Implementation()
{
	if (GetLocalRole() != ROLE_AutonomousProxy)
	{
		OnJumpedNetworked();
	}
}

void AGL_Character::OnJumpedNetworked()
{
	if (UGL_AnimInstance* AnimInstance = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->Jump();
	}
}

void AGL_Character::OnOverlayModeChanged_Implementation(const FGameplayTag& PreviousOverlayMode) {}

void AGL_Character::OnAimingChanged_Implementation(bool bPreviousAiming) {}
