#include "Nodes/GL_AnimGraphNode_CurvesBlend.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimGraphNode_CurvesBlend)

#define LOCTEXT_NAMESPACE "GL_AnimGraphNode_CurvesBlend"

FText UGL_AnimGraphNode_CurvesBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Curves");
}

FLinearColor UGL_AnimGraphNode_CurvesBlend::GetNodeTitleColor() const
{
	return { 0.2f, 0.8f, 0.2f };
}

FText UGL_AnimGraphNode_CurvesBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Curves");
}

FText UGL_AnimGraphNode_CurvesBlend::GetMenuCategory() const
{
	return LOCTEXT("Category", "Gameplay Locomotion");
}

FString UGL_AnimGraphNode_CurvesBlend::GetNodeCategory() const
{
	return GetMenuCategory().ToString();
}

#undef LOCTEXT_NAMESPACE

