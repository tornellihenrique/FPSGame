#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GL_Constants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GAMEPLAYLOCOMOTION_API UGL_Constants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:
	// Bones

	inline static const FName RootBone{TEXTVIEW("root")};
	inline static const FName PelvisBone{TEXTVIEW("pelvis")};
	inline static const FName HeadBone{TEXTVIEW("head")};
	inline static const FName Spine03Bone{TEXTVIEW("spine_03")};
	inline static const FName FootLeftBone{TEXTVIEW("foot_l")};
	inline static const FName FootRightBone{TEXTVIEW("foot_r")};
	inline static const FName HandLeftGunVirtualBone{TEXTVIEW("VB hand_l_to_ik_hand_gun")};
	inline static const FName HandRightGunVirtualBone{TEXTVIEW("VB hand_r_to_ik_hand_gun")};
	inline static const FName FootLeftIkBone{TEXTVIEW("ik_foot_l")};
	inline static const FName FootRightIkBone{TEXTVIEW("ik_foot_r")};
	inline static const FName FootLeftVirtualBone{TEXTVIEW("VB foot_l")};

	// Animation Slots

	inline static const FName FootRightVirtualBone{TEXTVIEW("VB foot_r")};
	inline static const FName TransitionSlot{TEXTVIEW("Transition")};
	inline static const FName TurnInPlaceStandingSlot{TEXTVIEW("TurnInPlaceStanding")};

	// Layering Animation Curves

	inline static const FName TurnInPlaceCrouchingSlot{TEXTVIEW("TurnInPlaceCrouching")};
	inline static const FName LayerHeadCurve{TEXTVIEW("LayerHead")};
	inline static const FName LayerHeadAdditiveCurve{TEXTVIEW("LayerHeadAdditive")};
	inline static const FName LayerHeadSlotCurve{TEXTVIEW("LayerHeadSlot")};
	inline static const FName LayerArmLeftCurve{TEXTVIEW("LayerArmLeft")};
	inline static const FName LayerArmLeftAdditiveCurve{TEXTVIEW("LayerArmLeftAdditive")};
	inline static const FName LayerArmLeftLocalSpaceCurve{TEXTVIEW("LayerArmLeftLocalSpace")};
	inline static const FName LayerArmLeftSlotCurve{TEXTVIEW("LayerArmLeftSlot")};
	inline static const FName LayerArmRightCurve{TEXTVIEW("LayerArmRight")};
	inline static const FName LayerArmRightAdditiveCurve{TEXTVIEW("LayerArmRightAdditive")};
	inline static const FName LayerArmRightLocalSpaceCurve{TEXTVIEW("LayerArmRightLocalSpace")};
	inline static const FName LayerArmRightSlotCurve{TEXTVIEW("LayerArmRightSlot")};
	inline static const FName LayerHandLeftCurve{TEXTVIEW("LayerHandLeft")};
	inline static const FName LayerHandRightCurve{TEXTVIEW("LayerHandRight")};
	inline static const FName LayerSpineCurve{TEXTVIEW("LayerSpine")};
	inline static const FName LayerSpineAdditiveCurve{TEXTVIEW("LayerSpineAdditive")};
	inline static const FName LayerSpineSlotCurve{TEXTVIEW("LayerSpineSlot")};
	inline static const FName LayerPelvisCurve{TEXTVIEW("LayerPelvis")};
	inline static const FName LayerPelvisSlotCurve{TEXTVIEW("LayerPelvisSlot")};
	inline static const FName LayerLegsCurve{TEXTVIEW("LayerLegs")};
	inline static const FName LayerLegsSlotCurve{TEXTVIEW("LayerLegsSlot")};
	inline static const FName HandLeftIkCurve{TEXTVIEW("HandLeftIk")};
	inline static const FName HandRightIkCurve{TEXTVIEW("HandRightIk")};
	inline static const FName ViewBlockCurve{TEXTVIEW("ViewBlock")};
	inline static const FName AllowAimingCurve{TEXTVIEW("AllowAiming")};
	inline static const FName HipsDirectionLockCurve{TEXTVIEW("HipsDirectionLock")};

	// Pose Animation Curves

	inline static const FName PoseGaitCurve{TEXTVIEW("PoseGait")};
	inline static const FName PoseMovingCurve{TEXTVIEW("PoseMoving")};
	inline static const FName PoseStandingCurve{TEXTVIEW("PoseStanding")};
	inline static const FName PoseCrouchingCurve{TEXTVIEW("PoseCrouching")};
	inline static const FName PoseGroundedCurve{TEXTVIEW("PoseGrounded")};
	inline static const FName PoseInAirCurve{TEXTVIEW("PoseInAir")};

	// Feet Animation Curves

	inline static const FName FootLeftIkCurve{TEXTVIEW("FootLeftIk")};
	inline static const FName FootLeftLockCurve{TEXTVIEW("FootLeftLock")};
	inline static const FName FootRightIkCurve{TEXTVIEW("FootRightIk")};
	inline static const FName FootRightLockCurve{TEXTVIEW("FootRightLock")};
	inline static const FName FootPlantedCurve{TEXTVIEW("FootPlanted")};
	inline static const FName FeetCrossingCurve{TEXTVIEW("FeetCrossing")};

	// Other Animation Curves

	inline static const FName RotationYawSpeedCurve{TEXTVIEW("RotationYawSpeed")};
	inline static const FName RotationYawOffsetCurve{TEXTVIEW("RotationYawOffset")};
	inline static const FName AllowTransitionsCurve{TEXTVIEW("AllowTransitions")};
	inline static const FName SprintBlockCurve{TEXTVIEW("SprintBlock")};
	inline static const FName GroundPredictionBlockCurve{TEXTVIEW("GroundPredictionBlock")};
	inline static const FName FootstepSoundBlockCurve{TEXTVIEW("FootstepSoundBlock")};

	// Debug

	inline static const FName CurvesDebugDisplay{TEXTVIEW("GL.Curves")};
	inline static const FName CharacterStateDebugDisplay{TEXTVIEW("GL.CharacterState")};
	inline static const FName ViewStateDebugDisplay{TEXTVIEW("GL.ViewState")};
	inline static const FName LocomotionStateDebugDisplay{TEXTVIEW("GL.LocomotionState")};
	inline static const FName MovementBaseDebugDisplay{TEXTVIEW("GL.MovementBase")};
	inline static const FName LeanStateDebugDisplay{TEXTVIEW("GL.LeanState")};
	inline static const FName GroundedStateDebugDisplay{TEXTVIEW("GL.GroundedState")};
	inline static const FName StandingStateDebugDisplay{TEXTVIEW("GL.StandingState")};
	inline static const FName CrouchingStateDebugDisplay{TEXTVIEW("GL.CrouchingState")};
	inline static const FName InAirStateDebugDisplay{TEXTVIEW("GL.InAirState")};
	inline static const FName PoseStateDebugDisplay{TEXTVIEW("GL.PoseState")};
	inline static const FName TurnInPlaceStateDebugDisplay{TEXTVIEW("GL.TurnInPlaceState")};

public:
	// Bones

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& RootBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& PelvisBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HeadBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& Spine03BoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HandLeftGunVirtualBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& HandRightGunVirtualBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftIkBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightIkBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootLeftVirtualBoneName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static const FName& FootRightVirtualBoneName();

	// Animation Slots

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static const FName& TransitionSlotName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static const FName& TurnInPlaceStandingSlotName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static const FName& TurnInPlaceCrouchingSlotName();

	// Layering Animation Curves

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHeadSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftLocalSpaceCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmLeftSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightLocalSpaceCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerArmRightSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHandLeftCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerHandRightCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerSpineSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerPelvisCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerPelvisSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerLegsCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LayerLegsSlotCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HandLeftIkCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HandRightIkCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& ViewBlockCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowAimingCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& HipsDirectionLockCurveName();

	// Pose Animation Curves

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseGaitCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseMovingCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseStandingCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseCrouchingCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseGroundedCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PoseInAirCurveName();

	// Feet Animation Curves

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootLeftIkCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootLeftLockCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootRightIkCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootRightLockCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootPlantedCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FeetCrossingCurveName();

	// Other Animation Curves

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawSpeedCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationYawOffsetCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& AllowTransitionsCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& SprintBlockCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& GroundPredictionBlockCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FootstepSoundBlockCurveName();

	// Debug

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CurvesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CharacterStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& ViewStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& LocomotionStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& MovementBaseDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& LeanStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& GroundedStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& StandingStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CrouchingStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& InAirStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& PoseStateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayLocomotion|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& TurnInPlaceDebugDisplayName();
};

inline const FName& UGL_Constants::RootBoneName()
{
	return RootBone;
}

inline const FName& UGL_Constants::PelvisBoneName()
{
	return PelvisBone;
}

inline const FName& UGL_Constants::HeadBoneName()
{
	return HeadBone;
}

inline const FName& UGL_Constants::Spine03BoneName()
{
	return Spine03Bone;
}

inline const FName& UGL_Constants::FootLeftBoneName()
{
	return FootLeftBone;
}

inline const FName& UGL_Constants::FootRightBoneName()
{
	return FootRightBone;
}

inline const FName& UGL_Constants::HandLeftGunVirtualBoneName()
{
	return HandLeftGunVirtualBone;
}

inline const FName& UGL_Constants::HandRightGunVirtualBoneName()
{
	return HandRightGunVirtualBone;
}

inline const FName& UGL_Constants::FootLeftIkBoneName()
{
	return FootLeftIkBone;
}

inline const FName& UGL_Constants::FootRightIkBoneName()
{
	return FootRightIkBone;
}

inline const FName& UGL_Constants::FootLeftVirtualBoneName()
{
	return FootLeftVirtualBone;
}

inline const FName& UGL_Constants::FootRightVirtualBoneName()
{
	return FootRightVirtualBone;
}

inline const FName& UGL_Constants::TransitionSlotName()
{
	return TransitionSlot;
}

inline const FName& UGL_Constants::TurnInPlaceStandingSlotName()
{
	return TurnInPlaceStandingSlot;
}

inline const FName& UGL_Constants::TurnInPlaceCrouchingSlotName()
{
	return TurnInPlaceCrouchingSlot;
}

inline const FName& UGL_Constants::LayerHeadCurveName()
{
	return LayerHeadCurve;
}

inline const FName& UGL_Constants::LayerHeadAdditiveCurveName()
{
	return LayerHeadAdditiveCurve;
}

inline const FName& UGL_Constants::LayerHeadSlotCurveName()
{
	return LayerHeadSlotCurve;
}

inline const FName& UGL_Constants::LayerArmLeftCurveName()
{
	return LayerArmLeftCurve;
}

inline const FName& UGL_Constants::LayerArmLeftAdditiveCurveName()
{
	return LayerArmLeftAdditiveCurve;
}

inline const FName& UGL_Constants::LayerArmLeftLocalSpaceCurveName()
{
	return LayerArmLeftLocalSpaceCurve;
}

inline const FName& UGL_Constants::LayerArmLeftSlotCurveName()
{
	return LayerArmLeftSlotCurve;
}

inline const FName& UGL_Constants::LayerArmRightCurveName()
{
	return LayerArmRightCurve;
}

inline const FName& UGL_Constants::LayerArmRightAdditiveCurveName()
{
	return LayerArmRightAdditiveCurve;
}

inline const FName& UGL_Constants::LayerArmRightLocalSpaceCurveName()
{
	return LayerArmRightLocalSpaceCurve;
}

inline const FName& UGL_Constants::LayerArmRightSlotCurveName()
{
	return LayerArmRightSlotCurve;
}

inline const FName& UGL_Constants::LayerHandLeftCurveName()
{
	return LayerHandLeftCurve;
}

inline const FName& UGL_Constants::LayerHandRightCurveName()
{
	return LayerHandRightCurve;
}

inline const FName& UGL_Constants::LayerSpineCurveName()
{
	return LayerSpineCurve;
}

inline const FName& UGL_Constants::LayerSpineAdditiveCurveName()
{
	return LayerSpineAdditiveCurve;
}

inline const FName& UGL_Constants::LayerSpineSlotCurveName()
{
	return LayerSpineSlotCurve;
}

inline const FName& UGL_Constants::LayerPelvisCurveName()
{
	return LayerPelvisCurve;
}

inline const FName& UGL_Constants::LayerPelvisSlotCurveName()
{
	return LayerPelvisSlotCurve;
}

inline const FName& UGL_Constants::LayerLegsCurveName()
{
	return LayerLegsCurve;
}

inline const FName& UGL_Constants::LayerLegsSlotCurveName()
{
	return LayerLegsSlotCurve;
}

inline const FName& UGL_Constants::HandLeftIkCurveName()
{
	return HandLeftIkCurve;
}

inline const FName& UGL_Constants::HandRightIkCurveName()
{
	return HandRightIkCurve;
}

inline const FName& UGL_Constants::ViewBlockCurveName()
{
	return ViewBlockCurve;
}

inline const FName& UGL_Constants::AllowAimingCurveName()
{
	return AllowAimingCurve;
}

inline const FName& UGL_Constants::HipsDirectionLockCurveName()
{
	return HipsDirectionLockCurve;
}

inline const FName& UGL_Constants::PoseGaitCurveName()
{
	return PoseGaitCurve;
}

inline const FName& UGL_Constants::PoseMovingCurveName()
{
	return PoseMovingCurve;
}

inline const FName& UGL_Constants::PoseStandingCurveName()
{
	return PoseStandingCurve;
}

inline const FName& UGL_Constants::PoseCrouchingCurveName()
{
	return PoseCrouchingCurve;
}

inline const FName& UGL_Constants::PoseGroundedCurveName()
{
	return PoseGroundedCurve;
}

inline const FName& UGL_Constants::PoseInAirCurveName()
{
	return PoseInAirCurve;
}

inline const FName& UGL_Constants::FootLeftIkCurveName()
{
	return FootLeftIkCurve;
}

inline const FName& UGL_Constants::FootLeftLockCurveName()
{
	return FootLeftLockCurve;
}

inline const FName& UGL_Constants::FootRightIkCurveName()
{
	return FootRightIkCurve;
}

inline const FName& UGL_Constants::FootRightLockCurveName()
{
	return FootRightLockCurve;
}

inline const FName& UGL_Constants::FootPlantedCurveName()
{
	return FootPlantedCurve;
}

inline const FName& UGL_Constants::FeetCrossingCurveName()
{
	return FeetCrossingCurve;
}

inline const FName& UGL_Constants::RotationYawSpeedCurveName()
{
	return RotationYawSpeedCurve;
}

inline const FName& UGL_Constants::RotationYawOffsetCurveName()
{
	return RotationYawOffsetCurve;
}

inline const FName& UGL_Constants::AllowTransitionsCurveName()
{
	return AllowTransitionsCurve;
}

inline const FName& UGL_Constants::SprintBlockCurveName()
{
	return SprintBlockCurve;
}

inline const FName& UGL_Constants::GroundPredictionBlockCurveName()
{
	return GroundPredictionBlockCurve;
}

inline const FName& UGL_Constants::FootstepSoundBlockCurveName()
{
	return FootstepSoundBlockCurve;
}

inline const FName& UGL_Constants::CurvesDebugDisplayName()
{
	return CurvesDebugDisplay;
}

inline const FName& UGL_Constants::CharacterStateDebugDisplayName()
{
	return CharacterStateDebugDisplay;
}

inline const FName& UGL_Constants::ViewStateDebugDisplayName()
{
	return ViewStateDebugDisplay;
}

inline const FName& UGL_Constants::LocomotionStateDebugDisplayName()
{
	return LocomotionStateDebugDisplay;
}

inline const FName& UGL_Constants::MovementBaseDebugDisplayName()
{
	return MovementBaseDebugDisplay;
}

inline const FName& UGL_Constants::LeanStateDebugDisplayName()
{
	return LeanStateDebugDisplay;
}

inline const FName& UGL_Constants::GroundedStateDebugDisplayName()
{
	return GroundedStateDebugDisplay;
}

inline const FName& UGL_Constants::StandingStateDebugDisplayName()
{
	return StandingStateDebugDisplay;
}

inline const FName& UGL_Constants::CrouchingStateDebugDisplayName()
{
	return CrouchingStateDebugDisplay;
}

inline const FName& UGL_Constants::InAirStateDebugDisplayName()
{
	return InAirStateDebugDisplay;
}

inline const FName& UGL_Constants::PoseStateDebugDisplayName()
{
	return PoseStateDebugDisplay;
}

inline const FName& UGL_Constants::TurnInPlaceDebugDisplayName()
{
	return TurnInPlaceStateDebugDisplay;
}
