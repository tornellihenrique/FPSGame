#include "Animation/GL_AnimInstanceProxy.h"

#include "Animation/GL_AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimInstanceProxy)

void FGL_AnimInstanceProxy::PostUpdate(UAnimInstance* AnimationInstance) const
{
	FAnimInstanceProxy::PostUpdate(AnimationInstance);

	if (UGL_AnimInstance* GI = Cast<UGL_AnimInstance>(AnimationInstance))
	{
		GI->NativePostUpdateAnimation();
	}
}
