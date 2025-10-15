#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "Nodes/GA_AnimNode_ProceduralMotion.h"
#include "GA_AnimGraphNode_ProceduralMotion.generated.h"

UCLASS()
class GAMEPLAYANIMATIONEDITOR_API UGA_AnimGraphNode_ProceduralMotion : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	FGA_AnimNode_ProceduralMotion Node;

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetMenuCategory() const override;
	virtual FString GetNodeCategory() const override;

	virtual FText GetControllerDescription() const override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }

};