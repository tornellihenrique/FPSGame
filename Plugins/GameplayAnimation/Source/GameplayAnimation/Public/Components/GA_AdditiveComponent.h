// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GA_ProceduralMotionComponent.h"
#include "Misc/GA_Types.h"
#include "GA_AdditiveComponent.generated.h"

UCLASS(ClassGroup=(GameplayAnimation), meta=(BlueprintSpawnableComponent, DisplayName = "Additive Component"))
class GAMEPLAYANIMATION_API UGA_AdditiveComponent : public UGA_ProceduralMotionComponent
{
	GENERATED_BODY()

public:
	UGA_AdditiveComponent(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CurveAlpha = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_X = TEXT("IK_X");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_Y = TEXT("IK_Y");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_Z = TEXT("IK_Z");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_Roll  = TEXT("IK_Roll");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_Pitch = TEXT("IK_Pitch");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_IK_Yaw   = TEXT("IK_Yaw");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Additive|Curves")
	FName Curve_MaskSprint = TEXT("MaskSprint");

protected:
	virtual bool ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const override;

private:
	static FORCEINLINE bool HasValid(const FName& N) { return !N.IsNone(); }

	static float ReadCurveGT(const UAnimInstance* AnimInstance, const FName& CurveName);
	
};
