#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Misc/GA_Types.h"
#include "GA_AnimNode_ProceduralMotion.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct GAMEPLAYANIMATION_API FGA_AnimNode_ProceduralMotion : public FAnimNode_SkeletalControlBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=SkeletalControl)
	FBoneReference BoneToModify;

	UPROPERTY(EditAnywhere, Category="Procedural", meta=(PinShownByDefault))
	FGA_ProceduralInput Input;

public:
	FGA_AnimNode_ProceduralMotion();

	// FAnimNode_SkeletalControlBase
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override
	{
		Super::Initialize_AnyThread(Context);

		ResetCaches();
	}

	virtual bool HasPreUpdate() const override { return true; }
	virtual void PreUpdate(const UAnimInstance* InAnimInstance) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override
	{
		return BoneToModify.IsValidToEvaluate(RequiredBones);
	}
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override
	{
		BoneToModify.Initialize(RequiredBones);
	}

private:
	struct FGA_StageOp
	{
		FTransform Xform;
		float Weight = 1.f;
		EGA_OutputSpace Space = EGA_OutputSpace::Component;
		EGA_Composition Comp = EGA_Composition::Additive;

		// DualBlend fields
		FTransform XformB;
		EGA_OutputSpace SpaceB = EGA_OutputSpace::Bone;
		float DualBlend = 0.f;
	};

	TArray<FGA_StageOp> StageOps[(int32)EGA_ProcStage::Count];

	static void ApplyOp(FTransform& InOutPoseCS, const FGA_StageOp& Op, FComponentSpacePoseContext& Output, FCompactPoseBoneIndex& CompactBoneIndex);

	void ResetCaches();

	static constexpr EGA_ProcStage kOrder[(int32)EGA_ProcStage::Count] =
	{
		EGA_ProcStage::PreOffset,
		EGA_ProcStage::Offset,
		EGA_ProcStage::ADS,
		EGA_ProcStage::Additive,
		EGA_ProcStage::Sway,
		EGA_ProcStage::Recoil,
		EGA_ProcStage::Post
	};
};