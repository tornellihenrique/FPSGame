#include "Nodes/GA_AnimGraphNode_ProceduralMotion.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_AnimGraphNode_ProceduralMotion)

#define LOCTEXT_NAMESPACE "GA_AnimGraphNode_ProceduralMotion"

FText UGA_AnimGraphNode_ProceduralMotion::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Procedural Motion");
}

FLinearColor UGA_AnimGraphNode_ProceduralMotion::GetNodeTitleColor() const
{
	return { 0.8f, 0.2f, 0.2f };
}

FText UGA_AnimGraphNode_ProceduralMotion::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Procedural Motion");
}

FText UGA_AnimGraphNode_ProceduralMotion::GetMenuCategory() const
{
	return LOCTEXT("Category", "Gameplay Animation");
}

FString UGA_AnimGraphNode_ProceduralMotion::GetNodeCategory() const
{
	return GetMenuCategory().ToString();
}

FText UGA_AnimGraphNode_ProceduralMotion::GetControllerDescription() const
{
	return FText::FromString(TEXT("Procedural Motion Node"));
}

#undef LOCTEXT_NAMESPACE
