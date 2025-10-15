// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GA_AdditiveComponent.h"

UGA_AdditiveComponent::UGA_AdditiveComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Stage = EGA_ProcStage::Additive;
}

bool UGA_AdditiveComponent::ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const
{
	FTransform AdditiveCS = FTransform::Identity;

	if (CurveAlpha > KINDA_SMALL_NUMBER && AnimInstance)
	{
		const float Roll = ReadCurveGT(AnimInstance, Curve_IK_Roll);
		const float Pitch = ReadCurveGT(AnimInstance, Curve_IK_Pitch);
		const float Yaw = ReadCurveGT(AnimInstance, Curve_IK_Yaw);

		const float CurveMaskAlpha = 1.0f - FMath::Clamp(ReadCurveGT(AnimInstance, Curve_MaskSprint), 0.0f, 1.0f);

		const FQuat CurveRot = FRotator(Pitch, Yaw, Roll).Quaternion();
		const FQuat Blended = FQuat::Slerp(FQuat::Identity, CurveRot, FMath::Clamp(CurveAlpha * CurveMaskAlpha, 0.f, 1.f));
		AdditiveCS.SetRotation(AdditiveCS.GetRotation() * Blended);

		const FVector CurveTrans(
			ReadCurveGT(AnimInstance, Curve_IK_X),
			ReadCurveGT(AnimInstance, Curve_IK_Y),
			ReadCurveGT(AnimInstance, Curve_IK_Z));

		AdditiveCS.AddToTranslation(CurveTrans * CurveAlpha * CurveMaskAlpha);
	}

	if (AdditiveCS.Equals(FTransform::Identity))
	{
		return false;
	}

	Out.Transform = AdditiveCS;       // component-space by convention
	Out.Weight = 1.f;                 // full weight; node stage already handles blending order
	Out.Stage = EGA_ProcStage::Additive;

	return true;
}

float UGA_AdditiveComponent::ReadCurveGT(const UAnimInstance* AnimInstance, const FName& CurveName)
{
	return (AnimInstance && HasValid(CurveName)) ? AnimInstance->GetCurveValue(CurveName) : 0.f;
}
