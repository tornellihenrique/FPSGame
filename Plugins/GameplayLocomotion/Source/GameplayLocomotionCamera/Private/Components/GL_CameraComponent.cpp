#include "Components/GL_CameraComponent.h"

#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"

#include "Utility/GL_Math.h"
#include "Utility/GL_Rotation.h"
#include "Utility/GL_CameraConstants.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_CameraComponent)

UGL_CameraComponent::UGL_CameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bTickInEditor = false;
	bHiddenInGame = true;
}

void UGL_CameraComponent::GetViewInfo(FMinimalViewInfo& ViewInfo) const
{
	ViewInfo.Location = CameraLocation;
	ViewInfo.Rotation = CameraRotation;
	ViewInfo.FOV = CameraFOV;

	ViewInfo.PostProcessBlendWeight = PostProcessWeight;
	if (ViewInfo.PostProcessBlendWeight > UE_SMALL_NUMBER)
	{
		ViewInfo.PostProcessSettings = PostProcess;
	}
}

FVector UGL_CameraComponent::GetFirstPersonCameraLocation() const
{
	return Character->GetMesh()->GetSocketLocation(FirstPersonSocketName);
}

FVector UGL_CameraComponent::GetThirdPersonPivotLocation() const
{
	const USkeletalMeshComponent* Mesh = Character->GetMesh();

	FVector FirstPivot;
	if (!IsValid(Mesh->GetAttachParent()) && FirstPivotSocketName == TEXT("root"))
	{
		// Fallback: capsule bottom if mesh is detached and root bone would be static.
		FirstPivot = Character->GetRootComponent()->GetComponentLocation();
		FirstPivot.Z -= Character->GetRootComponent()->Bounds.BoxExtent.Z;
	}
	else
	{
		FirstPivot = Mesh->GetSocketLocation(FirstPivotSocketName);
	}

	const FVector SecondPivot = Mesh->GetSocketLocation(SecondPivotSocketName);
	return (FirstPivot + SecondPivot) * 0.5f;
}

FVector UGL_CameraComponent::GetThirdPersonTraceStartLocation() const
{
	return Character->GetMesh()->GetSocketLocation(TraceSocketName);
}

void UGL_CameraComponent::PostLoad()
{
	Super::PostLoad();

	// Camera mesh never renders; always tick pose.
	VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void UGL_CameraComponent::OnRegister()
{
	Character = Cast<ACharacter>(GetOwner());

	Super::OnRegister();
}

void UGL_CameraComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);
	// Tick after owner for freshest state.
	AddTickPrerequisiteActor(GetOwner());
}

void UGL_CameraComponent::Activate(bool bReset)
{
	if (bReset || ShouldActivate())
	{
		TickCamera(0.f, /*bAllowLag*/false);
	}

	Super::Activate(bReset);
}

void UGL_CameraComponent::InitAnim(bool bForceReinitialize)
{
	Super::InitAnim(bForceReinitialize);
	AnimationInstance = GetAnimInstance();
}

void UGL_CameraComponent::BeginPlay()
{
	ensure(GetAnimInstance());
	ensure(Character);

	Super::BeginPlay();
}

void UGL_CameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (bIgnoreTimeDilation)
	{
		const float TimeDil = PreviousGlobalTimeDilation * GetOwner()->CustomTimeDilation;
		DeltaTime = TimeDil > UE_SMALL_NUMBER ? (DeltaTime / TimeDil) : GetWorld()->DeltaRealTimeSeconds;
	}

	PreviousGlobalTimeDilation = GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Skip camera tick until parallel anim evaluation completes.
	if (!IsRunningParallelEvaluation())
	{
		TickCamera(DeltaTime);
	}
}

void UGL_CameraComponent::CompleteParallelAnimationEvaluation(bool bDoPostAnimationEvaluation)
{
	Super::CompleteParallelAnimationEvaluation(bDoPostAnimationEvaluation);

	if (!GetAnimInstance())
	{
		return;
	}

	TickCamera(GetAnimInstance()->GetDeltaSeconds());
}

static FORCEINLINE float GL_GetCurve(const UAnimInstance* Anim, const FName& Name)
{
	return Anim ? Anim->GetCurveValue(Name) : 0.f;
}

void UGL_CameraComponent::TickCamera(float DeltaTime, bool bAllowLag /*= true*/)
{
	if (!GetAnimInstance() || !Character)
	{
		return;
	}

	// Refresh movement base.
	const auto& BasedMovement = Character->GetBasedMovement();
	const bool bMovementBaseHasRelativeRotation = BasedMovement.HasRelativeRotation();

	FVector MovementBaseLocation = FVector::ZeroVector;
	FQuat   MovementBaseRotation = FQuat::Identity;

	if (bMovementBaseHasRelativeRotation)
	{
		MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
			MovementBaseLocation, MovementBaseRotation);
	}

	if (BasedMovement.MovementBase != MovementBasePrimitive || BasedMovement.BoneName != MovementBaseBoneName)
	{
		MovementBasePrimitive = BasedMovement.MovementBase;
		MovementBaseBoneName = BasedMovement.BoneName;

		if (bMovementBaseHasRelativeRotation)
		{
			const FQuat Inv = MovementBaseRotation.Inverse();
			PivotMovementBaseRelativeLagLocation = Inv.RotateVector(PivotLagLocation - MovementBaseLocation);
			CameraMovementBaseRelativeRotation = Inv * CameraRotation.Quaternion();
		}
		else
		{
			PivotMovementBaseRelativeLagLocation = FVector::ZeroVector;
			CameraMovementBaseRelativeRotation = FQuat::Identity;
		}
	}

	const FRotator CameraTargetRotation = Character->GetViewRotation();

	const FVector PreviousPivotTargetLocation = PivotTargetLocation;
	PivotTargetLocation = GetThirdPersonPivotLocation();

	// First-person override from AnimBP (0..1)
	const float FirstPersonOverride = UGL_Math::Clamp01(
		GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::FirstPersonOverrideCurveName()));

	if (FAnimWeight::IsFullWeight(FirstPersonOverride))
	{
		// Skip everything else in pure FP.
		PivotLagLocation = PivotTargetLocation;
		PivotLocation = PivotTargetLocation;

		CameraLocation = GetFirstPersonCameraLocation();
		CameraRotation = CameraTargetRotation;
		CameraFOV = bOverrideFOV ? FOVOverride : FirstPersonFOV;
		return;
	}

	// Force disable lag if teleported.
	bAllowLag &= CameraTeleportDistanceThreshold <= 0.f ||
		FVector::DistSquared(PreviousPivotTargetLocation, PivotTargetLocation) <= FMath::Square(CameraTeleportDistanceThreshold);

	// Camera rotation (exact-halflife damper).
	if (bMovementBaseHasRelativeRotation)
	{
		CameraRotation = (MovementBaseRotation * CameraMovementBaseRelativeRotation).Rotator();
		CameraRotation = CalculateCameraRotation(CameraTargetRotation, DeltaTime, bAllowLag);
		CameraMovementBaseRelativeRotation = MovementBaseRotation.Inverse() * CameraRotation.Quaternion();
	}
	else
	{
		CameraRotation = CalculateCameraRotation(CameraTargetRotation, DeltaTime, bAllowLag);
	}

	// Axis-independent pivot lag in camera yaw space.
	const FQuat CameraYawRotation{ FVector::ZAxisVector, FMath::DegreesToRadians(CameraRotation.Yaw) };

	if (bMovementBaseHasRelativeRotation)
	{
		PivotLagLocation = MovementBaseLocation + MovementBaseRotation.RotateVector(PivotMovementBaseRelativeLagLocation);
		PivotLagLocation = CalculatePivotLagLocation(CameraYawRotation, DeltaTime, bAllowLag);
		PivotMovementBaseRelativeLagLocation = MovementBaseRotation.UnrotateVector(PivotLagLocation - MovementBaseLocation);
	}
	else
	{
		PivotLagLocation = CalculatePivotLagLocation(CameraYawRotation, DeltaTime, bAllowLag);
	}

	// Pivot and camera offsets.
	const FVector PivotOffset = CalculatePivotOffset();
	PivotLocation = PivotLagLocation + PivotOffset;

	const FVector CameraTargetLocation = PivotLocation + CalculateCameraOffset();

	// Collision trace with distance smoothing.
	const FVector CameraFinalLocation =
		CalculateCameraTrace(CameraTargetLocation, PivotOffset, DeltaTime, bAllowLag, TraceDistanceRatio);

	if (!FAnimWeight::IsRelevant(FirstPersonOverride))
	{
		CameraLocation = CameraFinalLocation;
		CameraFOV = ThirdPersonFOV;
	}
	else
	{
		CameraLocation = FMath::Lerp(CameraFinalLocation, GetFirstPersonCameraLocation(), FirstPersonOverride);
		CameraFOV = FMath::Lerp(ThirdPersonFOV, FirstPersonFOV, FirstPersonOverride);
	}

	if (bOverrideFOV)
	{
		CameraFOV = FMath::Clamp(FOVOverride, 5.f, 175.f);
	}

	CameraFOV = FMath::Clamp(CameraFOV + CalculateFovOffset(), 5.f, 175.f);
}

FRotator UGL_CameraComponent::CalculateCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const
{
	if (!bAllowLag)
	{
		return CameraTargetRotation;
	}

	const float RotationLag =
		GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::RotationLagCurveName());

	return UGL_Rotation::DamperExactRotation(CameraRotation, CameraTargetRotation, DeltaTime, RotationLag);
}

FVector UGL_CameraComponent::CalculatePivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const
{
	if (!bAllowLag)
	{
		return PivotTargetLocation;
	}

	const FVector RelInit = CameraYawRotation.UnrotateVector(PivotLagLocation);
	const FVector RelTgt = CameraYawRotation.UnrotateVector(PivotTargetLocation);

	const float LagX = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::LocationLagXCurveName());
	const float LagY = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::LocationLagYCurveName());
	const float LagZ = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::LocationLagZCurveName());

	const FVector Rel{
		UGL_Math::DamperExact(RelInit.X, RelTgt.X, DeltaTime, LagX),
		UGL_Math::DamperExact(RelInit.Y, RelTgt.Y, DeltaTime, LagY),
		UGL_Math::DamperExact(RelInit.Z, RelTgt.Z, DeltaTime, LagZ)
	};

	return CameraYawRotation.RotateVector(Rel);
}

FVector UGL_CameraComponent::CalculatePivotOffset() const
{
	const float sx = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::PivotOffsetXCurveName());
	const float sy = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::PivotOffsetYCurveName());
	const float sz = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::PivotOffsetZCurveName());

	const float ScaleZ = Character->GetMesh()->GetComponentScale().Z;
	return Character->GetMesh()->GetComponentQuat().RotateVector(FVector(sx, sy, sz) * ScaleZ);
}

FVector UGL_CameraComponent::CalculateCameraOffset() const
{
	const float sx = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::CameraOffsetXCurveName());
	const float sy = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::CameraOffsetYCurveName());
	const float sz = GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::CameraOffsetZCurveName());

	const float ScaleZ = Character->GetMesh()->GetComponentScale().Z;
	return CameraRotation.RotateVector(FVector(sx, sy, sz) * ScaleZ);
}

float UGL_CameraComponent::CalculateFovOffset() const
{
	return GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::FovOffsetCurveName());
}

FVector UGL_CameraComponent::CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, float DeltaTime, bool bAllowLag, float& OutTraceDistanceRatio) const
{
	const float MeshScale = UE_REAL_TO_FLOAT(Character->GetMesh()->GetComponentScale().Z);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius * MeshScale);

	// Build trace start with optional curve override (no shoulder switch).
	const float OverrideAlpha = UGL_Math::Clamp01(GL_GetCurve(GetAnimInstance(), UGL_CameraConstants::TraceOverrideCurveName()));
	const FVector TraceStartDefault = GetThirdPersonTraceStartLocation();
	const FVector TraceOverride = PivotTargetLocation + PivotOffset + TraceOverrideOffset;
	FVector TraceStart = FMath::Lerp(TraceStartDefault, TraceOverride, OverrideAlpha);

	const FVector TraceEnd = CameraTargetLocation;

	FHitResult Hit;
	FVector TraceResult = TraceEnd;

	static const FName MainTraceTag(TEXT("UGL_CameraComponent::MainTrace"));
	if (GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, TraceChannel, Shape, { MainTraceTag, false, GetOwner() }))
	{
		if (!Hit.bStartPenetrating)
		{
			TraceResult = Hit.Location;
		}
		else if (TryAdjustLocationBlockedByGeometry(TraceStart))
		{
			static const FName AdjustedTraceTag(TEXT("UGL_CameraComponent::AdjustedTrace"));
			GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, TraceChannel, Shape, { AdjustedTraceTag, false, GetOwner() });
			if (Hit.IsValidBlockingHit())
			{
				TraceResult = Hit.Location;
			}
		}
		else
		{
			// Note: TraceStart may be modified inside TryAdjustLocationBlockedByGeometry even if it returned false.
			TraceResult = TraceStart;
		}
	}

	// Apply trace distance smoothing.
	if (!bAllowLag || !bEnableTraceDistanceSmoothing)
	{
		OutTraceDistanceRatio = 1.f;
		return TraceResult;
	}

	const FVector TraceVector = TraceEnd - TraceStart;
	const float   TraceDistance = TraceVector.Size();
	if (TraceDistance <= UE_KINDA_SMALL_NUMBER)
	{
		OutTraceDistanceRatio = 1.f;
		return TraceResult;
	}

	const float TargetRatio = UE_REAL_TO_FLOAT((TraceResult - TraceStart).Size() / TraceDistance);

	OutTraceDistanceRatio = (TargetRatio <= TraceDistanceRatio)
		? TargetRatio
		: UGL_Math::DamperExact(TraceDistanceRatio, TargetRatio, DeltaTime, TraceDistanceHalfLife);

	return TraceStart + TraceVector * TraceDistanceRatio;
}

bool UGL_CameraComponent::TryAdjustLocationBlockedByGeometry(FVector& Location) const
{
	// Based on ComponentEncroachesBlockingGeometry_WithAdjustment().
	const float MeshScale = UE_REAL_TO_FLOAT(Character->GetMesh()->GetComponentScale().Z);
	const FCollisionShape Shape = FCollisionShape::MakeSphere((TraceRadius + 1.f) * MeshScale);

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty());
	ON_SCOPE_EXIT{ Overlaps.Reset(); };

	static const FName OverlapTag(TEXT("UGL_CameraComponent::OverlapMulti"));
	if (!GetWorld()->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, TraceChannel, Shape, { OverlapTag, false, GetOwner() }))
	{
		return false;
	}

	FVector Adjustment = FVector::ZeroVector;
	bool bAnyValidBlock = false;

	FMTDResult MTD;
	for (const auto& O : Overlaps)
	{
		if (!O.Component.IsValid() || O.Component->GetCollisionResponseToChannel(TraceChannel) != ECR_Block)
		{
			continue;
		}

		const FBodyInstance* Body = O.Component->GetBodyInstance(NAME_None, true, O.ItemIndex);
		if (!Body || !Body->OverlapTest(Location, FQuat::Identity, Shape, &MTD))
		{
			return false;
		}

		if (!FMath::IsNearlyZero(MTD.Distance))
		{
			Adjustment += MTD.Direction * MTD.Distance;
			bAnyValidBlock = true;
		}
	}

	if (!bAnyValidBlock)
	{
		return false;
	}

	FVector AdjustmentDir = Adjustment;
	if (!AdjustmentDir.Normalize() || ((GetOwner()->GetActorLocation() - Location).GetSafeNormal() | AdjustmentDir) < -UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	Location += Adjustment;

	static const FName FreeSpaceTag(TEXT("UGL_CameraComponent::FreeSpaceOverlap"));
	return !GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius * MeshScale),
		{ FreeSpaceTag, false, GetOwner() });
}

float UGL_CameraComponent::GetCurve(const FName& Name) const
{
	const UAnimInstance* Anim = AnimationInstance.Get();
	return Anim ? Anim->GetCurveValue(Name) : 0.f;
}
