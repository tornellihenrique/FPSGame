#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GE_Types.h"
#include "GE_EquipmentAnimData.generated.h"

UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYEQUIPMENTS_API UGE_EquipmentAnimData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimSequence> PoseFP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimSequence> PoseTP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FGameplayTag OverlayMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FEquipmentAnimMontageData EquipAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FEquipmentAnimMontageData UnEquipAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FTransform AttachOffsetTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FRotator OriginRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightHandOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftHandOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightClavicleOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftClavicleOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightLowerarmOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftLowerarmOffset = FVector::ZeroVector;

};
