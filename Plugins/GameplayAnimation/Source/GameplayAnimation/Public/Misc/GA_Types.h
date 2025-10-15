#pragma once

#include "CoreMinimal.h"

#include "GA_Types.generated.h"

UENUM(BlueprintType)
enum class EGA_ProcStage : uint8
{
	PreOffset    = 0,
	Offset       = 1,
	Additive     = 2,
	ADS          = 3,
	Sway         = 4,
	Recoil       = 5,
	Post         = 6,

	Count        UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EGA_OutputSpace : uint8
{
	Component     = 0,
	Bone          = 1,
	ParentBone    = 2
};

UENUM(BlueprintType)
enum class EGA_Composition : uint8
{
	Additive      = 0, // apply as delta
	LerpToTarget  = 1, // pose = Lerp/Slerp(pose, target, Weight)
	DualBlend	  = 2  // blend between two applications of the same recoil step
};

USTRUCT(BlueprintType)
struct FGA_ProceduralInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	TObjectPtr<AActor> SourceActorOverride;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	FVector PivotOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	bool bFirstPerson = false;
};

USTRUCT(BlueprintType)
struct FGA_ProceduralOutput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	EGA_ProcStage Stage = EGA_ProcStage::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	EGA_OutputSpace Space = EGA_OutputSpace::Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	EGA_Composition Composition = EGA_Composition::Additive;

	// --- DualBlend payload (used only when Composition == DualBlend) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	FTransform TransformB = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	EGA_OutputSpace SpaceB = EGA_OutputSpace::Bone;

	// 0 => use Transform (in Space), 1 => use TransformB (in SpaceB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural")
	float DualBlend = 0.f;
};
