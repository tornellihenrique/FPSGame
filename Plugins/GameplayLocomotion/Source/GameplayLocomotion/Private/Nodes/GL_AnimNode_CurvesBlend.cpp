#include "Nodes/GL_AnimNode_CurvesBlend.h"

#include "Animation/AnimTrace.h"

#include "Utility/GL_Utility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimNode_CurvesBlend)

void FGL_AnimNode_CurvesBlend::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	Super::Initialize_AnyThread(Context);

	SourcePose.Initialize(Context);
	CurvesPose.Initialize(Context);
}

void FGL_AnimNode_CurvesBlend::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	Super::CacheBones_AnyThread(Context);

	SourcePose.CacheBones(Context);
	CurvesPose.CacheBones(Context);
}

void FGL_AnimNode_CurvesBlend::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	Super::Update_AnyThread(Context);

	GetEvaluateGraphExposedInputs().Execute(Context);

	SourcePose.Update(Context);

	const auto CurrentBlendAmount{ GetBlendAmount() };
	if (FAnimWeight::IsRelevant(CurrentBlendAmount))
	{
		CurvesPose.Update(Context);
	}

	TRACE_ANIM_NODE_VALUE(Context, TEXT("Blend Amount"), CurrentBlendAmount)
	TRACE_ANIM_NODE_VALUE(Context, TEXT("Blend Mode"), *GameplayEnumUtility::GetNameStringByValue(GetBlendMode()))
}

void FGL_AnimNode_CurvesBlend::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()
	ANIM_MT_SCOPE_CYCLE_COUNTER_VERBOSE(CurvesBlend, !IsInGameThread())

	Super::Evaluate_AnyThread(Output);

	SourcePose.Evaluate(Output);

	const auto CurrentBlendAmount{ GetBlendAmount() };
	if (!FAnimWeight::IsRelevant(CurrentBlendAmount))
	{
		return;
	}

	auto CurvesPoseContext{ Output };
	CurvesPose.Evaluate(CurvesPoseContext);

	switch (GetBlendMode())
	{
	case EGL_CurvesBlendMode::BlendByAmount:
		Output.Curve.Accumulate(CurvesPoseContext.Curve, CurrentBlendAmount);
		break;

	case EGL_CurvesBlendMode::Combine:
		Output.Curve.Combine(CurvesPoseContext.Curve);
		break;

	case EGL_CurvesBlendMode::CombinePreserved:
		Output.Curve.CombinePreserved(CurvesPoseContext.Curve);
		break;

	case EGL_CurvesBlendMode::UseMaxValue:
		Output.Curve.UseMaxValue(CurvesPoseContext.Curve);
		break;

	case EGL_CurvesBlendMode::UseMinValue:
		Output.Curve.UseMinValue(CurvesPoseContext.Curve);
		break;

	case EGL_CurvesBlendMode::Override:
		Output.Curve.Override(CurvesPoseContext.Curve);
		break;
	}
}

void FGL_AnimNode_CurvesBlend::GatherDebugData(FNodeDebugData& DebugData)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	TStringBuilder<256> DebugItemBuilder{ InPlace, DebugData.GetNodeName(this), TEXTVIEW(": Blend Amount: ") };

	DebugItemBuilder.Appendf(TEXT("%.2f"), GetBlendAmount());

	DebugData.AddDebugItem(FString{ DebugItemBuilder });
	SourcePose.GatherDebugData(DebugData.BranchFlow(1.0f));
	CurvesPose.GatherDebugData(DebugData.BranchFlow(GetBlendAmount()));
}

float FGL_AnimNode_CurvesBlend::GetBlendAmount() const
{
	return GET_ANIM_NODE_DATA(float, BlendAmount);
}

EGL_CurvesBlendMode FGL_AnimNode_CurvesBlend::GetBlendMode() const
{
	return GET_ANIM_NODE_DATA(EGL_CurvesBlendMode, BlendMode);
}

