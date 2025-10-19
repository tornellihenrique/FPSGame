// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GA_ADSComponent.h"

UGA_ADSComponent::UGA_ADSComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Stage = EGA_ProcStage::ADS;
}

void UGA_ADSComponent::ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const
{
	if (ADSAlpha <= KINDA_SMALL_NUMBER) return;

	const FTransform AbsPB = MakeAbsolutePB();

	{
		FGA_ProceduralOutput O;
		O.Transform = AbsPB;
		O.Weight = ADSAlpha;
		O.Stage = EGA_ProcStage::ADS;
		O.Space = EGA_OutputSpace::ParentBone;
		O.Composition = EGA_Composition::LerpToTarget;
		Outs.Add(O);
	}

	const FTransform AimAddCS = MakeAimPointAdditiveCS(Input.PivotOffset);
	if (!AimAddCS.Equals(FTransform::Identity))
	{
		FGA_ProceduralOutput O;
		O.Transform = AimAddCS;
		O.Weight = ADSAlpha;
		O.Stage = EGA_ProcStage::ADS;
		O.Space = EGA_OutputSpace::Component;
		O.Composition = EGA_Composition::Additive;
		Outs.Add(O);
	}
}

FTransform UGA_ADSComponent::MakeAbsolutePB() const
{
	return FTransform(AbsoluteResetRotation.Quaternion(), FVector::ZeroVector);
}

FTransform UGA_ADSComponent::MakeAimPointAdditiveCS(const FVector& InPivotOffset) const
{
	// Rotation swizzle: roll <- -pitch ; pitch <- roll ; yaw <- 0
	const FRotator OffR = ADSOffset.Rotator();
	const FRotator AimR(/*Pitch*/ OffR.Roll, /*Yaw*/ 0.f, /*Roll*/ OffR.Pitch);
	const FQuat AimQ = AimR.Quaternion();

	FVector T = ADSOffset.GetLocation();
	T.Z *= -1.f;
	T += InPivotOffset;
	// T = AimQ.RotateVector(T);
	T.X *= -1.f;
	Swap(T.X, T.Y);

	return FTransform(AimQ, T);
}
