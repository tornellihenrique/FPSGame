// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GA_ProceduralMotionComponent.h"
#include "Misc/GA_Types.h"
#include "GA_OffsetComponent.generated.h"

UCLASS(ClassGroup=(GameplayAnimation), meta=(BlueprintSpawnableComponent, DisplayName = "Offset Component"))
class GAMEPLAYANIMATION_API UGA_OffsetComponent : public UGA_ProceduralMotionComponent
{
	GENERATED_BODY()

public:
	UGA_OffsetComponent(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Offset|View", meta=(ClampMin="0.0", ClampMax="1.0"))
	float OffsetAlpha = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Offset|FP")
	FRotator OffsetRotationFP = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Offset|FP")
	FVector OffsetLocationFP = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Offset|TP")
	FVector OffsetLocationTP = FVector::ZeroVector;

protected:
	virtual void ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const override;

private:
	static FORCEINLINE void SwapXY(FVector& V) { const float t = V.X; V.X = V.Y; V.Y = t; }
	static FORCEINLINE void SwapPitchRoll(FRotator& R) { const float t = R.Pitch; R.Pitch = R.Roll; R.Roll = t; }

};
