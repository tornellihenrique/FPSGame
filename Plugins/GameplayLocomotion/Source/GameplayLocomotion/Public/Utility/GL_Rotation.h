#pragma once

#include "GL_Math.h"
#include "GL_Rotation.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GAMEPLAYLOCOMOTION_API UGL_Rotation : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr auto CounterClockwiseRotationAngleThreshold{5.0f};

public:
	template <typename ValueType> requires std::is_floating_point_v<ValueType>
	static constexpr ValueType RemapAngleForCounterClockwiseRotation(ValueType Angle);

	// Remaps the angle from the [175, 180] range to [-185, -180]. Used to
	// make the character rotate counterclockwise during a 180 degree turn.
	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (ReturnDisplayName = "Angle"))
	static float RemapAngleForCounterClockwiseRotation(float Angle);

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (ReturnDisplayName = "Angle"))
	static float LerpAngle(float From, float To, float Ratio);

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (AutoCreateRefTerm = "From, To", ReturnDisplayName = "Rotation"))
	static FRotator LerpRotation(const FRotator& From, const FRotator& To, float Ratio);

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (ReturnDisplayName = "Angle"))
	static float InterpolateAngleConstant(float Current, float Target, float DeltaTime, float Speed);

	// HalfLife is the time it takes for the distance to the target to be reduced by half.
	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (ReturnDisplayName = "Angle"))
	static float DamperExactAngle(float Current, float Target, float DeltaTime, float HalfLife);

	// HalfLife is the time it takes for the distance to the target to be reduced by half.
	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility",
		Meta = (AutoCreateRefTerm = "Current, Target", ReturnDisplayName = "Rotation"))
	static FRotator DamperExactRotation(const FRotator& Current, const FRotator& Target, float DeltaTime, float HalfLife);

	// Same as FMath::QInterpTo(), but uses FQuat::FastLerp() instead of FQuat::Slerp().
	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (ReturnDisplayName = "Quaternion"))
	static FQuat InterpolateQuaternionFast(const FQuat& Current, const FQuat& Target, float DeltaTime, float Speed);

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Rotation Utility", Meta = (AutoCreateRefTerm = "TwistAxis", ReturnDisplayName = "Twist"))
	static FQuat GetTwist(const FQuat& Quaternion, const FVector& TwistAxis = FVector::UpVector);
	
};

template <typename ValueType> requires std::is_floating_point_v<ValueType>
constexpr ValueType UGL_Rotation::RemapAngleForCounterClockwiseRotation(const ValueType Angle)
{
	if (Angle > 180.0f - CounterClockwiseRotationAngleThreshold)
	{
		return Angle - 360.0f;
	}

	return Angle;
}

inline float UGL_Rotation::RemapAngleForCounterClockwiseRotation(const float Angle)
{
	return RemapAngleForCounterClockwiseRotation<float>(Angle);
}

inline float UGL_Rotation::LerpAngle(const float From, const float To, const float Ratio)
{
	auto Delta{ FMath::UnwindDegrees(To - From) };
	Delta = RemapAngleForCounterClockwiseRotation(Delta);

	return FMath::UnwindDegrees(From + Delta * Ratio);
}

inline FRotator UGL_Rotation::LerpRotation(const FRotator& From, const FRotator& To, const float Ratio)
{
	auto Result{ To - From };
	Result.Normalize();

	Result.Pitch = RemapAngleForCounterClockwiseRotation(Result.Pitch);
	Result.Yaw = RemapAngleForCounterClockwiseRotation(Result.Yaw);
	Result.Roll = RemapAngleForCounterClockwiseRotation(Result.Roll);

	Result *= Ratio;
	Result += From;
	Result.Normalize();

	return Result;
}

inline float UGL_Rotation::InterpolateAngleConstant(const float Current, const float Target, const float DeltaTime, const float Speed)
{
	if (Speed <= 0.0f || FMath::IsNearlyEqual(Current, Target))
	{
		return Target;
	}

	auto Delta{ FMath::UnwindDegrees(Target - Current) };
	Delta = RemapAngleForCounterClockwiseRotation(Delta);

	const auto MaxDelta{ Speed * DeltaTime };

	return FMath::UnwindDegrees(Current + FMath::Clamp(Delta, -MaxDelta, MaxDelta));
}

inline float UGL_Rotation::DamperExactAngle(const float Current, const float Target, const float DeltaTime, const float HalfLife)
{
	return LerpAngle(Current, Target, UGL_Math::DamperExactAlpha(DeltaTime, HalfLife));
}

inline FRotator UGL_Rotation::DamperExactRotation(const FRotator& Current, const FRotator& Target,
	const float DeltaTime, const float HalfLife)
{
	return LerpRotation(Current, Target, UGL_Math::DamperExactAlpha(DeltaTime, HalfLife));
}

inline FQuat UGL_Rotation::InterpolateQuaternionFast(const FQuat& Current, const FQuat& Target, const float DeltaTime, const float Speed)
{
	if (Speed <= 0.0f || Current.Equals(Target))
	{
		return Target;
	}

	return FQuat::FastLerp(Current, Target, UGL_Math::Clamp01(Speed * DeltaTime)).GetNormalized();
}

inline FQuat UGL_Rotation::GetTwist(const FQuat& Quaternion, const FVector& TwistAxis)
{
	// Based on TQuat<T>::ToSwingTwist().

	const auto Projection{ (TwistAxis | FVector{Quaternion.X, Quaternion.Y, Quaternion.Z}) * TwistAxis };

	return FQuat{ Projection.X, Projection.Y, Projection.Z, Quaternion.W }.GetNormalized();
}

