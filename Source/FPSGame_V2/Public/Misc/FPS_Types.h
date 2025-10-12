#pragma once

#include "CoreMinimal.h"

#include "FPS_Types.generated.h"

class AGE_Equipment;

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<AController> Instigator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TSubclassOf<UDamageType> DamageTypeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<AActor> DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	float DamageAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	uint8 bHeadshot : 1;

	FDamageInfo()
		: Instigator(nullptr)
		, DamageCauser(nullptr)
		, DamageAmount(0.0f)
		, bHeadshot(false)
	{}

	FDamageInfo(AController* InInstigator, TSubclassOf<UDamageType> InDamageTypeClass, TObjectPtr<AActor> InDamageCauser, float InDamageAmount, bool InHeadshot)
		: Instigator(InInstigator)
		, DamageTypeClass(InDamageTypeClass)
		, DamageCauser(InDamageCauser)
		, DamageAmount(InDamageAmount)
		, bHeadshot(InHeadshot)
	{}

	bool IsValid() const
	{
		return ::IsValid(Instigator);
	}
};

USTRUCT(BlueprintType)
struct FAnimState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimSequence> PoseFP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<UAnimSequence> PoseTP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector PivotOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FRotator OriginRelativeRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightHandOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftHandOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightClavicleOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftClavicleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector RightLowerarmOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector LeftLowerarmOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	float ForceDisableRunningAlpha;

	FAnimState()
		: PivotOffset(FVector::ZeroVector)
		, OriginRelativeRotation(FRotator::ZeroRotator)
		, RightHandOffset(FVector::ZeroVector)
		, LeftHandOffset(FVector::ZeroVector)
		, RightClavicleOffset(FVector::ZeroVector)
		, LeftClavicleOffset(FVector::ZeroVector)
		, RightLowerarmOffset(FVector::ZeroVector)
		, LeftLowerarmOffset(FVector::ZeroVector)
		, ForceDisableRunningAlpha(0.0f)
	{}

	void Reset()
	{
		PivotOffset = FVector::ZeroVector;
		OriginRelativeRotation = FRotator::ZeroRotator;
		RightHandOffset = FVector::ZeroVector;
		LeftHandOffset = FVector::ZeroVector;
		RightClavicleOffset = FVector::ZeroVector;
		LeftClavicleOffset = FVector::ZeroVector;
		RightLowerarmOffset = FVector::ZeroVector;
		LeftLowerarmOffset = FVector::ZeroVector;
		ForceDisableRunningAlpha = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FPlayerLoadout
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TArray<TSoftClassPtr<AGE_Equipment>> Equipments;

	FPlayerLoadout()
		: Equipments({})
	{}

	bool IsValid() const
	{
		return Equipments.Num() > 0;
	}
};