#include "Nodes/GL_AnimGraphNode_GameplayTagsBlend.h"

#include "Utility/GL_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimGraphNode_GameplayTagsBlend)

#define LOCTEXT_NAMESPACE "GL_AnimGraphNode_GameplayTagsBlend"

UGL_AnimGraphNode_GameplayTagsBlend::UGL_AnimGraphNode_GameplayTagsBlend()
{
	Node.AddPose();
}

void UGL_AnimGraphNode_GameplayTagsBlend::PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent)
{
	if (ChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STRING_VIEW_CHECKED(FGL_AnimNode_GameplayTagsBlend, Tags))
	{
		ReconstructNode();
	}

	Super::PostEditChangeProperty(ChangedEvent);
}

FText UGL_AnimGraphNode_GameplayTagsBlend::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Title", "Blend Poses by Gameplay Tag");
}

FText UGL_AnimGraphNode_GameplayTagsBlend::GetTooltipText() const
{
	return LOCTEXT("Tooltip", "Blend Poses by Gameplay Tag");
}

void UGL_AnimGraphNode_GameplayTagsBlend::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& PreviousPins)
{
	Node.RefreshPosePins();

	Super::ReallocatePinsDuringReconstruction(PreviousPins);
}

FString UGL_AnimGraphNode_GameplayTagsBlend::GetNodeCategory() const
{
	return FString{ TEXTVIEW("GameplayLocomotion") };
}

void UGL_AnimGraphNode_GameplayTagsBlend::CustomizePinData(UEdGraphPin* Pin, const FName SourcePropertyName, const int32 PinIndex) const
{
	Super::CustomizePinData(Pin, SourcePropertyName, PinIndex);

	bool bBlendPosePin;
	bool bBlendTimePin;
	GetBlendPinProperties(Pin, bBlendPosePin, bBlendTimePin);

	if (!bBlendPosePin && !bBlendTimePin)
	{
		return;
	}

	Pin->PinFriendlyName = PinIndex <= 0
		? LOCTEXT("Default", "Default")
		: PinIndex > Node.Tags.Num()
		? LOCTEXT("Invalid", "Invalid")
		: FText::AsCultureInvariant(UGL_Utility::GetSimpleTagName(Node.Tags[PinIndex - 1]).ToString());

	if (bBlendPosePin)
	{
		static const FTextFormat BlendPosePinFormat{ LOCTEXT("Pose", "{PinName} Pose") };

		Pin->PinFriendlyName = FText::Format(BlendPosePinFormat, { {FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName} });
	}
	else if (bBlendTimePin)
	{
		static const FTextFormat BlendTimePinFormat{ LOCTEXT("BlendTime", "{PinName} Blend Time") };

		Pin->PinFriendlyName = FText::Format(BlendTimePinFormat, { {FString{TEXTVIEW("PinName")}, Pin->PinFriendlyName} });
	}
}

void UGL_AnimGraphNode_GameplayTagsBlend::GetBlendPinProperties(const UEdGraphPin* Pin, bool& bBlendPosePin, bool& bBlendTimePin)
{
	const auto PinFullName{ Pin->PinName.ToString() };
	const auto SeparatorIndex{ PinFullName.Find(TEXTVIEW("_"), ESearchCase::CaseSensitive) };

	if (SeparatorIndex <= 0)
	{
		bBlendPosePin = false;
		bBlendTimePin = false;
		return;
	}

	const auto PinName{ PinFullName.Left(SeparatorIndex) };
	bBlendPosePin = PinName == TEXTVIEW("BlendPose");
	bBlendTimePin = PinName == TEXTVIEW("BlendTime");
}

#undef LOCTEXT_NAMESPACE

