#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "GA_SwayData.generated.h"

USTRUCT(BlueprintType)
struct FVectorSpringSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector SpringStiffness{FVector::OneVector};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector SpringDamping{FVector::OneVector};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector SmoothDamping{FVector::OneVector};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector SmoothSpeed{FVector::OneVector};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector Scale{FVector::OneVector};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector Threshold{FVector::ZeroVector};
};

USTRUCT(BlueprintType)
struct FSpringSwaySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVectorSpringSettings TranslationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVectorSpringSettings RotationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FTransform SpaceOffset{FTransform::Identity};
};

USTRUCT(BlueprintType)
struct FAnimSway
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aiming")
	FSpringSwaySettings AimSwaySettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	FSpringSwaySettings MoveSwaySettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	float AdsScale{1.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FVector PivotOffset{FVector::ZeroVector};
};

struct FSwaySpringState
{
	FFloatSpringState X;
	FFloatSpringState Y;
	FFloatSpringState Z;

	FFloatSpringState Pitch;
	FFloatSpringState Yaw;
	FFloatSpringState Roll;

	void Reset()
	{
		X.Reset();
		Y.Reset();
		Z.Reset();

		Pitch.Reset();
		Yaw.Reset();
		Roll.Reset();
	}
};

UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYANIMATION_API UGA_SwayData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Defaults)
	FAnimSway AnimSway;

};
