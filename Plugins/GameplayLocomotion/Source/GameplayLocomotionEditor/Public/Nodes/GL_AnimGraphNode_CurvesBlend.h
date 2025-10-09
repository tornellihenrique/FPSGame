#pragma once

#include "AnimGraphNode_Base.h"
#include "Nodes/GL_AnimNode_CurvesBlend.h"

#include "GL_AnimGraphNode_CurvesBlend.generated.h"

UCLASS()
class GAMEPLAYLOCOMOTIONEDITOR_API UGL_AnimGraphNode_CurvesBlend : public UAnimGraphNode_Base
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FGL_AnimNode_CurvesBlend Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FString GetNodeCategory() const override;
};

