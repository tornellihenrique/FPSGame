#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GL_GameplayTags.h"
#include "GL_MovementSettings.generated.h"

class UCurveVector;
class UCurveFloat;

USTRUCT(BlueprintType)
struct FGL_GaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	uint8 bAllowDirectionDependentSpeed : 1 { false };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, ForceUnits="cm/s"))
	float WalkForwardSpeed = 175.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, EditCondition="bAllowDirectionDependentSpeed", ForceUnits="cm/s"))
	float WalkBackwardSpeed = 175.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, ForceUnits="cm/s"))
	float RunForwardSpeed = 375.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, EditCondition="bAllowDirectionDependentSpeed", ForceUnits="cm/s"))
	float RunBackwardSpeed = 375.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, ForceUnits="cm/s"))
	float SprintSpeed = 650.f;

	// X = Accel, Y = Decel, Z = GroundFriction. Input domain: GaitAmount [0..3].
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TObjectPtr<UCurveVector> AccelDecelFrictionCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TObjectPtr<UCurveFloat> RotationInterpSpeedCurve = nullptr;

	FORCEINLINE float GetMaxWalkSpeed() const
	{
		return bAllowDirectionDependentSpeed ? FMath::Max(WalkForwardSpeed, WalkBackwardSpeed) : WalkForwardSpeed;
	}

	FORCEINLINE float GetMaxRunSpeed() const
	{
		return bAllowDirectionDependentSpeed ? FMath::Max(RunForwardSpeed, RunBackwardSpeed) : RunForwardSpeed;
	}
};

UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYLOCOMOTION_API UGL_MovementSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	// Angle range where we lerp forward/back speed when direction-dependent is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin=0, ClampMax=180, ForceUnits="deg"))
	FFloatInterval VelocityAngleToSpeedInterpolationRange = FFloatInterval(100.f, 125.f);

	// Stances -> per-gait settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ForceInlineRow))
	TMap<FGameplayTag, FGL_GaitSettings> Stances {{GameplayStanceTags::Standing, {}}, {GameplayStanceTags::Crouching, {}}};

};
