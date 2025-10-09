#pragma once

#include "Animation/AnimInstanceProxy.h"

#include "GL_AnimInstanceProxy.generated.h"

USTRUCT()
struct GAMEPLAYLOCOMOTION_API FGL_AnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FGL_AnimInstanceProxy() = default;
	explicit FGL_AnimInstanceProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}

protected:
	virtual void PostUpdate(UAnimInstance* AnimationInstance) const override;
};