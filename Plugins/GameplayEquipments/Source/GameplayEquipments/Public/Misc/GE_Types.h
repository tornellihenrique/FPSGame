#pragma once

#include "CoreMinimal.h"

#include "GE_Types.generated.h"

UENUM(BlueprintType)
enum class EGE_EquipmentRemovalReason : uint8
{
	GenericRemove,
	OwnerDeath
};

USTRUCT(BlueprintType)
struct FDualAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimMontage> MontageFP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimMontage> MontageTP;

	bool IsValid() const
	{
		return ::IsValid(MontageFP) && ::IsValid(MontageTP);
	}
};

USTRUCT(BlueprintType)
struct FEquipmentAnimMontageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FDualAnimMontageData CharacterAnimData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FDualAnimMontageData EquipmentAnimData;

	bool IsValid() const
	{
		return CharacterAnimData.IsValid();
	}
};