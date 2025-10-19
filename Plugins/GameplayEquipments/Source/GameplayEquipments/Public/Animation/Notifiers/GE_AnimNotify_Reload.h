// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GE_AnimNotify_Reload.generated.h"

UCLASS(meta=(DisplayName="Reload Notify"))
class GAMEPLAYEQUIPMENTS_API UGE_AnimNotify_Reload : public UAnimNotify
{
	GENERATED_BODY()

public:
	UGE_AnimNotify_Reload();

	virtual void Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
