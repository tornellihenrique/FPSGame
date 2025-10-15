#include "Nodes/GA_AnimNode_ProceduralMotion.h"

#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimTrace.h"

#include "Components/GA_ProceduralMotionComponent.h"

FGA_AnimNode_ProceduralMotion::FGA_AnimNode_ProceduralMotion()
	: FAnimNode_SkeletalControlBase()
{
}

void FGA_AnimNode_ProceduralMotion::PreUpdate(const UAnimInstance* InAnimInstance)
{
	ResetCaches();

	if (!InAnimInstance) return;

	const float DeltaSeconds = InAnimInstance->GetDeltaSeconds();

	AActor* SourceActor = (Input.SourceActorOverride != nullptr) ? Input.SourceActorOverride.Get() : InAnimInstance->GetOwningActor();
	if (!SourceActor || !SourceActor->HasActorBegunPlay()) return;

	TInlineComponentArray<UGA_ProceduralMotionComponent*, 16> AllGAComps(SourceActor);
	for (UGA_ProceduralMotionComponent* Comp : AllGAComps)
	{
		if (!IsValid(Comp) || !Comp->IsRegistered()) continue;

		Comp->UpdateOutput(DeltaSeconds, InAnimInstance, Input);

		const TArray<FGA_ProceduralOutput>& Outs = Comp->GetCachedOutputs();
		for (const FGA_ProceduralOutput& Out : Outs)
		{
			if (Out.Weight <= KINDA_SMALL_NUMBER) continue;
			if (Out.Transform.Equals(FTransform::Identity) && Out.Composition == EGA_Composition::Additive) continue;

			const int32 StageIdx = FMath::Clamp((int32)Out.Stage, 0, (int32)EGA_ProcStage::Count - 1);

			FGA_StageOp Op;
			Op.Xform = Out.Transform;
			Op.Weight = FMath::Clamp(Out.Weight, 0.f, 1.f);
			Op.Space = Out.Space;
			Op.Comp = Out.Composition;
			Op.XformB = Out.TransformB;
			Op.SpaceB = Out.SpaceB;
			Op.DualBlend = FMath::Clamp(Out.DualBlend, 0.f, 1.f);
			StageOps[StageIdx].Emplace(MoveTemp(Op));
		}
	}
}

void FGA_AnimNode_ProceduralMotion::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	check(OutBoneTransforms.Num() == 0);

	const FBoneContainer& BC = Output.Pose.GetPose().GetBoneContainer();
	FCompactPoseBoneIndex Idx = BoneToModify.GetCompactPoseIndex(BC);

	FTransform BoneCS = Output.Pose.GetComponentSpaceTransform(Idx);

	for (int32 i = 0; i < (int32)EGA_ProcStage::Count; ++i)
	{
		const EGA_ProcStage Stage = kOrder[i];
		for (const FGA_StageOp& Op : StageOps[(int32)Stage])
		{
			ApplyOp(BoneCS, Op, Output, Idx);
		}
	}

	OutBoneTransforms.Emplace(Idx, BoneCS);
}

static FTransform ToCS_Target(EGA_OutputSpace Space, const FTransform& In, FComponentSpacePoseContext& Output, FCompactPoseBoneIndex& Idx)
{
	const FTransform CompXf = Output.AnimInstanceProxy->GetComponentTransform();
	if (Space == EGA_OutputSpace::Component)
	{
		return In;
	}
	else if (Space == EGA_OutputSpace::Bone)
	{
		FTransform T = In;
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(CompXf, Output.Pose, T, Idx, BCS_BoneSpace);
		return T;
	}
	else // ParentBone
	{
		FTransform T = In;
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(CompXf, Output.Pose, T, Idx, BCS_ParentBoneSpace);
		return T;
	}
}

static void ApplyAdditive_InSpace(EGA_OutputSpace Space, FTransform& PoseCS, const FTransform& Delta, float Weight, FComponentSpacePoseContext& Output, FCompactPoseBoneIndex& Idx)
{
	if (Space == EGA_OutputSpace::Component)
	{
		PoseCS.AddToTranslation(Delta.GetTranslation() * Weight);
		const FQuat R = FQuat::Slerp(FQuat::Identity, Delta.GetRotation(), Weight);
		PoseCS.SetRotation(R * PoseCS.GetRotation());
		return;
	}

	const FTransform CompXf = Output.AnimInstanceProxy->GetComponentTransform();
	FTransform Local = PoseCS;

	const EBoneControlSpace BCS = (Space == EGA_OutputSpace::Bone) ? BCS_BoneSpace : BCS_ParentBoneSpace;
	FAnimationRuntime::ConvertCSTransformToBoneSpace(CompXf, Output.Pose, Local, Idx, BCS);

	Local.AddToTranslation(Delta.GetTranslation() * Weight);
	const FQuat R = FQuat::Slerp(FQuat::Identity, Delta.GetRotation(), Weight);
	Local.SetRotation(R * Local.GetRotation());

	FAnimationRuntime::ConvertBoneSpaceTransformToCS(CompXf, Output.Pose, Local, Idx, BCS);
	PoseCS = Local;
}

static FTransform PoseAfterAdditive(const FTransform& BasePoseCS, EGA_OutputSpace Space, const FTransform& Delta, FComponentSpacePoseContext& Output, FCompactPoseBoneIndex& Idx)
{
	FTransform Result = BasePoseCS;
	ApplyAdditive_InSpace(Space, Result, Delta, /*Weight=*/1.f, Output, Idx);
	return Result;
}

void FGA_AnimNode_ProceduralMotion::ApplyOp(FTransform& InOutPoseCS, const FGA_StageOp& Op, FComponentSpacePoseContext& Output, FCompactPoseBoneIndex& CompactBoneIndex)
{
	switch (Op.Comp)
	{
		case EGA_Composition::Additive:
		{
			ApplyAdditive_InSpace(Op.Space, InOutPoseCS, Op.Xform, Op.Weight, Output, CompactBoneIndex);
		}
		break;

		case EGA_Composition::LerpToTarget:
		{
			const FTransform TargetCS = ToCS_Target(Op.Space, Op.Xform, Output, CompactBoneIndex);
			InOutPoseCS.SetLocation(FMath::Lerp(InOutPoseCS.GetLocation(), TargetCS.GetLocation(), Op.Weight));
			InOutPoseCS.SetRotation(FQuat::Slerp(InOutPoseCS.GetRotation(), TargetCS.GetRotation(), Op.Weight));
		}
		break;

		case EGA_Composition::DualBlend:
		{
			const FTransform A = PoseAfterAdditive(InOutPoseCS, Op.Space, Op.Xform, Output, CompactBoneIndex);   // "Global" path
			const FTransform B = PoseAfterAdditive(InOutPoseCS, Op.SpaceB, Op.XformB, Output, CompactBoneIndex);   // "Local/Bone" path

			FTransform Mixed = A;
			Mixed.SetLocation(FMath::Lerp(A.GetLocation(), B.GetLocation(), Op.DualBlend));
			Mixed.SetRotation(FQuat::Slerp(A.GetRotation(), B.GetRotation(), Op.DualBlend));

			InOutPoseCS.SetLocation(FMath::Lerp(InOutPoseCS.GetLocation(), Mixed.GetLocation(), Op.Weight));
			InOutPoseCS.SetRotation(FQuat::Slerp(InOutPoseCS.GetRotation(), Mixed.GetRotation(), Op.Weight));
		}
		break;
	}
}

void FGA_AnimNode_ProceduralMotion::ResetCaches()
{
	for (int32 i = 0; i < (int32)EGA_ProcStage::Count; ++i)
	{
		StageOps[i].Reset();
	}
}
