#include "Utility/GL_Vector.h"

#include "Utility/GL_Math.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_Vector)

FVector UGL_Vector::SlerpSkipNormalization(const FVector& From, const FVector& To, const float Ratio)
{
	// https://allenchou.net/2018/05/game-math-deriving-the-slerp-formula/

	const auto Dot{ From | To };

	if (Dot > 0.9995f || Dot < -0.9995f)
	{
		return FMath::Lerp(From, To, Ratio).GetSafeNormal();
	}

	const auto Theta{ UE_REAL_TO_FLOAT(FMath::Acos(Dot)) * Ratio };

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, Theta);

	const auto FromPerpendicular{ (To - From * Dot).GetSafeNormal() };

	return From * Cos + FromPerpendicular * Sin;
}

FVector UGL_Vector::SpringDamperVector(FGL_SpringVectorState& SpringState, const FVector& Current,
	const FVector& Target, const float DeltaTime, const float Frequency,
	const float DampingRatio, const float TargetVelocityAmount)
{
	return UGL_Math::SpringDamper(SpringState, Current, Target, DeltaTime, Frequency, DampingRatio, TargetVelocityAmount);
}


