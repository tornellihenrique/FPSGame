#include "Components/GA_SwayComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Misc/GA_SwayData.h"

UGA_SwayComponent::UGA_SwayComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Stage = EGA_ProcStage::Sway;
}

void UGA_SwayComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UGA_SwayComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UGA_SwayComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGA_SwayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UGA_SwayComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UGA_SwayComponent::ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const
{
	const FAnimSway& Preset = GetPreset();

	RefreshSwayTargets(DeltaSeconds, AnimInstance);

	RefreshAimSway(DeltaSeconds, Preset);
	RefreshMoveSway(DeltaSeconds, Preset);

	FVector T = AimSwayOutput.GetLocation();
	T.X = T.Y;
	T.Y = 0.f;

	FRotator R = AimSwayOutput.Rotator();
	const float Roll = R.Roll;
	R.Roll = R.Pitch;
	R.Pitch = Roll;

	FTransform Result = FTransform(R, T);

	// Move contribution
	{
		FVector MT = MoveSwayOutput.GetLocation();
		float CachedX = MT.X;
		MT.X = MT.Y;
		MT.Y = CachedX;

		FRotator MR = MoveSwayOutput.Rotator();
		CachedX = MR.Roll;
		MR.Roll = MR.Pitch;
		MR.Pitch = CachedX;

		Result.AddToTranslation(MT);
		Result.SetRotation(Result.GetRotation() * MR.Quaternion());
	}

	// Pivot offset (component-space)
	{
		const FVector P = Preset.PivotOffset;
		const FVector Offset = Result.GetRotation().RotateVector(P) - P;
		Result.AddToTranslation(Offset);
	}

	Out.Transform = Result;
	Out.Weight = 1.f;
	Out.Stage = EGA_ProcStage::Sway;

	return !Result.Equals(FTransform::Identity);
}

FAnimSway UGA_SwayComponent::GetPreset() const
{
	if (IsValid(SwayPreset))
	{
		return SwayPreset->AnimSway;
	}

	static FAnimSway Default;
	return Default;
}

void UGA_SwayComponent::RefreshSwayTargets(float DT, const UAnimInstance* AnimInstance) const
{
	AcquireInputsFromOwner(AnimInstance);

	// --- Aim target ---
	{
		FVectorSpringSettings S = GetPreset().AimSwaySettings.RotationSettings;

		float Roll = S.Scale.X * MouseDeltaInput.X;
		float Yaw = S.Scale.Z * MouseDeltaInput.X;
		float Pitch = S.Scale.Y * MouseDeltaInput.Y;

		Roll = FMath::Clamp(Roll, -S.Threshold.X, S.Threshold.X);
		Yaw = FMath::Clamp(Yaw, -S.Threshold.Z, S.Threshold.Z);
		Pitch = FMath::Clamp(Pitch, -S.Threshold.Y, S.Threshold.Y);

		FRotator Q = AimSwayTarget.Rotator();
		Pitch = FMath::FInterpTo(Q.Pitch, Pitch, DT, S.SmoothDamping.Y);
		Yaw = FMath::FInterpTo(Q.Yaw, Yaw, DT, S.SmoothDamping.Z);
		Roll = FMath::FInterpTo(Q.Roll, Roll, DT, S.SmoothDamping.X);
		AimSwayTarget.SetRotation(FRotator(Pitch, Yaw, Roll).Quaternion());

		S = GetPreset().AimSwaySettings.TranslationSettings;

		float Y = S.Scale.Y * MouseDeltaInput.X;
		float Z = S.Scale.Z * MouseDeltaInput.Y;

		Y = FMath::Clamp(Y, -S.Threshold.Y, S.Threshold.Y);
		Z = FMath::Clamp(Z, -S.Threshold.Z, S.Threshold.Z);

		FVector T = AimSwayTarget.GetLocation();
		Y = FMath::FInterpTo(T.Y, Y, DT, S.SmoothDamping.Y);
		Z = FMath::FInterpTo(T.Z, Z, DT, S.SmoothDamping.Z);
		AimSwayTarget.SetLocation(FVector(0.f, Y, Z));
	}

	// --- Move target ---
	{
		FVectorSpringSettings S = GetPreset().MoveSwaySettings.RotationSettings;

		float Roll = S.Scale.X * MoveInput.X;
		float Yaw = S.Scale.Z * MoveInput.X;
		float Pitch = S.Scale.Y * MoveInput.Y;

		Roll = FMath::Clamp(Roll, -S.Threshold.X, S.Threshold.X);
		Yaw = FMath::Clamp(Yaw, -S.Threshold.Z, S.Threshold.Z);
		Pitch = FMath::Clamp(Pitch, -S.Threshold.Y, S.Threshold.Y);

		FRotator Q = MoveSwayTarget.Rotator();
		Pitch = FMath::FInterpTo(Q.Pitch, Pitch, DT, S.SmoothDamping.Y);
		Yaw = FMath::FInterpTo(Q.Yaw, Yaw, DT, S.SmoothDamping.Z);
		Roll = FMath::FInterpTo(Q.Roll, Roll, DT, S.SmoothDamping.X);
		MoveSwayTarget.SetRotation(FRotator(Pitch, Yaw, Roll).Quaternion());

		S = GetPreset().MoveSwaySettings.TranslationSettings;

		float X = S.Scale.X * MoveInput.Y;
		float Y = S.Scale.Y * MoveInput.X;
		float Z = S.Scale.Z * MoveInput.Y;

		X = FMath::Clamp(X, -S.Threshold.X, S.Threshold.X);
		Y = FMath::Clamp(Y, -S.Threshold.Y, S.Threshold.Y);
		Z = FMath::Clamp(Z, -S.Threshold.Z, S.Threshold.Z);

		FVector T = MoveSwayTarget.GetLocation();
		X = FMath::FInterpTo(T.X, X, DT, S.SmoothDamping.X);
		Y = FMath::FInterpTo(T.Y, Y, DT, S.SmoothDamping.Y);
		Z = FMath::FInterpTo(T.Z, Z, DT, S.SmoothDamping.Z);
		MoveSwayTarget.SetLocation(FVector(X, Y, Z));
	}
}

void UGA_SwayComponent::RefreshAimSway(float DT, const FAnimSway& Preset) const
{
	const float Weight = SwayScale * (bScaleWithADS && bIsAiming ? Preset.AdsScale : 1.f);

	// Rotation spring
	{
		const FVectorSpringSettings S = Preset.AimSwaySettings.RotationSettings;
		FRotator Out = AimSwayOutput.Rotator();

		Out.Pitch = UKismetMathLibrary::FloatSpringInterp(Out.Pitch, AimSwayTarget.Rotator().Pitch, AimSpring.Pitch, S.SpringStiffness.Y, S.SpringDamping.Y, DT * S.SmoothSpeed.Y);
		Out.Yaw = UKismetMathLibrary::FloatSpringInterp(Out.Yaw, AimSwayTarget.Rotator().Yaw, AimSpring.Yaw, S.SpringStiffness.Z, S.SpringDamping.Z, DT * S.SmoothSpeed.Z);
		Out.Roll = UKismetMathLibrary::FloatSpringInterp(Out.Roll, AimSwayTarget.Rotator().Roll, AimSpring.Roll, S.SpringStiffness.X, S.SpringDamping.X, DT * S.SmoothSpeed.X);

		AimSwayOutput.SetRotation(FQuat::Slerp(FQuat::Identity, Out.Quaternion(), Weight));
	}

	// Translation spring
	{
		const FVectorSpringSettings S = Preset.AimSwaySettings.TranslationSettings;
		FVector Out = AimSwayOutput.GetLocation();

		Out.X = UKismetMathLibrary::FloatSpringInterp(Out.X, AimSwayTarget.GetLocation().X, AimSpring.X, S.SpringStiffness.X, S.SpringDamping.X, DT * S.SmoothSpeed.X);
		Out.Y = UKismetMathLibrary::FloatSpringInterp(Out.Y, AimSwayTarget.GetLocation().Y, AimSpring.Y, S.SpringStiffness.Y, S.SpringDamping.Y, DT * S.SmoothSpeed.Y);
		Out.Z = UKismetMathLibrary::FloatSpringInterp(Out.Z, AimSwayTarget.GetLocation().Z, AimSpring.Z, S.SpringStiffness.Z, S.SpringDamping.Z, DT * S.SmoothSpeed.Z);

		AimSwayOutput.SetLocation(FMath::Lerp(FVector::ZeroVector, Out, Weight));
	}
}

void UGA_SwayComponent::RefreshMoveSway(float DT, const FAnimSway& Preset) const
{
	const float Weight = SwayScale * (bScaleWithADS && bIsAiming ? Preset.AdsScale : 1.f);

	// Rotation spring
	{
		const FVectorSpringSettings S = Preset.MoveSwaySettings.RotationSettings;
		FRotator Out = MoveSwayOutput.Rotator();

		Out.Pitch = UKismetMathLibrary::FloatSpringInterp(Out.Pitch, MoveSwayTarget.Rotator().Pitch, MoveSpring.Pitch, S.SpringStiffness.Y, S.SpringDamping.Y, DT * S.SmoothSpeed.Y);
		Out.Yaw = UKismetMathLibrary::FloatSpringInterp(Out.Yaw, MoveSwayTarget.Rotator().Yaw, MoveSpring.Yaw, S.SpringStiffness.Z, S.SpringDamping.Z, DT * S.SmoothSpeed.Z);
		Out.Roll = UKismetMathLibrary::FloatSpringInterp(Out.Roll, MoveSwayTarget.Rotator().Roll, MoveSpring.Roll, S.SpringStiffness.X, S.SpringDamping.X, DT * S.SmoothSpeed.X);

		MoveSwayOutput.SetRotation(FQuat::Slerp(FQuat::Identity, Out.Quaternion(), Weight));
	}

	// Translation spring
	{
		const FVectorSpringSettings S = Preset.MoveSwaySettings.TranslationSettings;
		FVector Out = MoveSwayOutput.GetLocation();

		Out.X = UKismetMathLibrary::FloatSpringInterp(Out.X, MoveSwayTarget.GetLocation().X, MoveSpring.X, S.SpringStiffness.X, S.SpringDamping.X, DT * S.SmoothSpeed.X);
		Out.Y = UKismetMathLibrary::FloatSpringInterp(Out.Y, MoveSwayTarget.GetLocation().Y, MoveSpring.Y, S.SpringStiffness.Y, S.SpringDamping.Y, DT * S.SmoothSpeed.Y);
		Out.Z = UKismetMathLibrary::FloatSpringInterp(Out.Z, MoveSwayTarget.GetLocation().Z, MoveSpring.Z, S.SpringStiffness.Z, S.SpringDamping.Z, DT * S.SmoothSpeed.Z);

		MoveSwayOutput.SetLocation(FMath::Lerp(FVector::ZeroVector, Out, Weight));
	}
}

void UGA_SwayComponent::AcquireInputsFromOwner(const UAnimInstance* AnimInstance) const
{
	if (!bAutoReadFromOwner || !AnimInstance)
	{
		return;
	}

	AActor* Owner = AnimInstance->GetOwningActor();
	const APawn* Pawn = Cast<APawn>(Owner);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;

	// Mouse delta from control-rotation change (deg) -> scaled "input".
	if (PC)
	{
		const FRotator CR = PC->GetControlRotation();
		if (bHaveLastControlRot)
		{
			FRotator Delta = (CR - LastControlRot).GetNormalized();
			const_cast<UGA_SwayComponent*>(this)->MouseDeltaInput.X = Delta.Yaw * MouseToInputScaleYaw;
			const_cast<UGA_SwayComponent*>(this)->MouseDeltaInput.Y = -Delta.Pitch * MouseToInputScalePitch; // UE pitch up is negative
		}
		const_cast<UGA_SwayComponent*>(this)->LastControlRot = CR;
		const_cast<UGA_SwayComponent*>(this)->bHaveLastControlRot = true;
	}

	// Move input from last movement input vector.
	if (const UCharacterMovementComponent* Move = Pawn ? Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()) : nullptr)
	{
		const FVector LI = Pawn->GetLastMovementInputVector(); // world dir (not normalized)

		// Project onto pawn right/forward to get strafe/forward magnitudes.
		const FRotator YawOnly(0.f, Pawn->GetActorRotation().Yaw, 0.f);
		const FVector Fwd = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

		const float ForwardMag = FVector::DotProduct(LI, Fwd);
		const float RightMag = FVector::DotProduct(LI, Right);

		const_cast<UGA_SwayComponent*>(this)->MoveInput = FVector2D(
			FMath::Clamp(RightMag * MoveVectorScale, -1.f, 1.f),   // X = strafe/right
			FMath::Clamp(ForwardMag * MoveVectorScale, -1.f, 1.f)  // Y = forward
		);
	}
}
