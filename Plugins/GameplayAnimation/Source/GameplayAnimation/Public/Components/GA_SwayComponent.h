#pragma once

#include "CoreMinimal.h"
#include "GA_ProceduralMotionComponent.h"
#include "Misc/GA_SwayData.h"
#include "GA_SwayComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEPLAYANIMATION_API UGA_SwayComponent : public UGA_ProceduralMotionComponent
{
	GENERATED_BODY()

public:
	UGA_SwayComponent(const FObjectInitializer& ObjectInitializer);

	//~ UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End of UActorComponent

protected:
	virtual bool ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Procedural Motion|Sway")
	TObjectPtr<UGA_SwayData> SwayPreset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway")
	float SwayScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway")
	bool bScaleWithADS = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway|Input")
	bool bIsAiming = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway|Input")
	bool bAutoReadFromOwner = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway|Input", meta=(EditCondition="bAutoReadFromOwner"))
	float MouseToInputScaleYaw = 1.0f;   // deg -> input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway|Input", meta=(EditCondition="bAutoReadFromOwner"))
	float MouseToInputScalePitch = 1.0f; // deg -> input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion|Sway|Input", meta=(EditCondition="bAutoReadFromOwner"))
	float MoveVectorScale = 1.0f;         // normalize/scale GetLastMovementInputVector

protected:
	// Mouse delta in "game units" (X = yaw, Y = pitch). Usually small values like ~[-1..1].
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FVector2D MouseDeltaInput = FVector2D::ZeroVector;

	// Move input (X = strafe/right, Y = forward). Usually in [-1..1].
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FVector2D MoveInput = FVector2D::ZeroVector;

private:
	mutable FTransform AimSwayTarget = FTransform::Identity;
	mutable FTransform MoveSwayTarget = FTransform::Identity;
	mutable FTransform AimSwayOutput = FTransform::Identity;
	mutable FTransform MoveSwayOutput = FTransform::Identity;

	mutable FSwaySpringState AimSpring;
	mutable FSwaySpringState MoveSpring;

	mutable FRotator LastControlRot = FRotator::ZeroRotator;
	mutable bool bHaveLastControlRot = false;

	FAnimSway GetPreset() const;

	void RefreshSwayTargets(float DT, const UAnimInstance* AnimInstance) const;
	void RefreshAimSway(float DT, const FAnimSway& Preset) const;
	void RefreshMoveSway(float DT, const FAnimSway& Preset) const;

	void AcquireInputsFromOwner(const UAnimInstance* AnimInstance) const;

};
