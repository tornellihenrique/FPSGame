#include "Nodes/GL_AnimNode_GameplayTagsBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimNode_GameplayTagsBlend)

int32 FGL_AnimNode_GameplayTagsBlend::GetActiveChildIndex()
{
	const auto& CurrentActiveTag{ GetActiveTag() };

	return CurrentActiveTag.IsValid()
		? GetTags().Find(CurrentActiveTag) + 1
		: 0;
}

const FGameplayTag& FGL_AnimNode_GameplayTagsBlend::GetActiveTag() const
{
	return GET_ANIM_NODE_DATA(FGameplayTag, ActiveTag);
}

const TArray<FGameplayTag>& FGL_AnimNode_GameplayTagsBlend::GetTags() const
{
	return GET_ANIM_NODE_DATA(TArray<FGameplayTag>, Tags);
}

#if WITH_EDITOR
void FGL_AnimNode_GameplayTagsBlend::RefreshPosePins()
{
	const auto Difference{ BlendPose.Num() - GetTags().Num() - 1 };
	if (Difference == 0)
	{
		return;
	}

	if (Difference > 0)
	{
		for (auto i{ Difference }; i > 0; i--)
		{
			RemovePose(BlendPose.Num() - 1);
		}
	}
	else
	{
		for (auto i{ Difference }; i < 0; i++)
		{
			AddPose();
		}
	}
}
#endif
