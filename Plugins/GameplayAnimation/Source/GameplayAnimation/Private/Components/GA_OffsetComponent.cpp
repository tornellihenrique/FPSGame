// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GA_OffsetComponent.h"

UGA_OffsetComponent::UGA_OffsetComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Stage = EGA_ProcStage::Offset;
}

void UGA_OffsetComponent::ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const
{
	const float FPw = (Input.bFirstPerson ? 1.f : 0.f) * OffsetAlpha;
	const float TPw = (Input.bFirstPerson ? 0.f : 1.f) * OffsetAlpha;

	if (FPw > KINDA_SMALL_NUMBER && !OffsetRotationFP.IsNearlyZero())
	{
		const FQuat DeltaR = FQuat::Slerp(FQuat::Identity, OffsetRotationFP.Quaternion(), FMath::Clamp(FPw, 0.f, 1.f));
		
		FGA_ProceduralOutput O;
		O.Stage = EGA_ProcStage::Offset;
		O.Composition = EGA_Composition::Additive;
		O.Space = EGA_OutputSpace::Bone;
		O.Transform = FTransform(DeltaR, FVector::ZeroVector);
		O.Weight = 1.f;
		Outs.Add(O);
	}

	const FVector T = (OffsetLocationFP * FPw) + (OffsetLocationTP * TPw);
	if (!T.IsNearlyZero())
	{
		FGA_ProceduralOutput O;
		O.Stage = EGA_ProcStage::Offset;
		O.Composition = EGA_Composition::Additive;
		O.Space = EGA_OutputSpace::Component;
		O.Transform = FTransform(FQuat::Identity, T);
		O.Weight = 1.f;
		Outs.Add(O);
	}
}
