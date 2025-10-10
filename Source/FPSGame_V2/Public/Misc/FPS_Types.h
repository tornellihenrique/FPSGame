#pragma once

#include "CoreMinimal.h"

#include "FPS_Types.generated.h"

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