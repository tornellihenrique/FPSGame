// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GA_ProceduralMotionComponent.h"
#include "GA_ADSComponent.generated.h"

UCLASS(ClassGroup=(GameplayAnimation), meta=(BlueprintSpawnableComponent, DisplayName = "ADS Component"))
class GAMEPLAYANIMATION_API UGA_ADSComponent : public UGA_ProceduralMotionComponent
{
	GENERATED_BODY()

public:
	UGA_ADSComponent(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|ADS", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ADSAlpha = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|ADS")
	FTransform ADSOffset = FTransform::Identity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|ADS")
	FRotator AbsoluteResetRotation = FRotator(0.f, -90.f, 0.f);

protected:
	virtual void ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const override;

private:
	FTransform MakeAbsolutePB() const;
	FTransform MakeAimPointAdditiveCS(const FVector& InPivotOffset) const;

};
