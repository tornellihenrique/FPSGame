#include "Character/GL_Character.h"

#include "DisplayDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstanceProxy.h"

#include "Animation/GL_AnimInstance.h"
#include "Utility/GL_Constants.h"
#include "Utility/GL_Math.h"
#include "Utility/GL_Utility.h"

#define LOCTEXT_NAMESPACE "GL_CharacterDebug"

static const TSet<FName>& GLCurveAllowlist()
{
	static const TSet<FName> Allow{
		UGL_Constants::LayerHeadCurveName(),
		UGL_Constants::LayerHeadAdditiveCurveName(),
		UGL_Constants::LayerHeadSlotCurveName(),
		UGL_Constants::LayerArmLeftCurveName(),
		UGL_Constants::LayerArmLeftAdditiveCurveName(),
		UGL_Constants::LayerArmLeftLocalSpaceCurveName(),
		UGL_Constants::LayerArmLeftSlotCurveName(),
		UGL_Constants::LayerArmRightCurveName(),
		UGL_Constants::LayerArmRightAdditiveCurveName(),
		UGL_Constants::LayerArmRightLocalSpaceCurveName(),
		UGL_Constants::LayerArmRightSlotCurveName(),
		UGL_Constants::LayerHandLeftCurveName(),
		UGL_Constants::LayerHandRightCurveName(),
		UGL_Constants::LayerSpineCurveName(),
		UGL_Constants::LayerSpineAdditiveCurveName(),
		UGL_Constants::LayerSpineSlotCurveName(),
		UGL_Constants::LayerPelvisCurveName(),
		UGL_Constants::LayerPelvisSlotCurveName(),
		UGL_Constants::LayerLegsCurveName(),
		UGL_Constants::LayerLegsSlotCurveName(),
		UGL_Constants::HandLeftIkCurveName(),
		UGL_Constants::HandRightIkCurveName(),
		UGL_Constants::ViewBlockCurveName(),
		UGL_Constants::AllowAimingCurveName(),
		UGL_Constants::HipsDirectionLockCurveName(),
		UGL_Constants::PoseGaitCurveName(),
		UGL_Constants::PoseMovingCurveName(),
		UGL_Constants::PoseStandingCurveName(),
		UGL_Constants::PoseCrouchingCurveName(),
		UGL_Constants::PoseGroundedCurveName(),
		UGL_Constants::PoseInAirCurveName(),
		UGL_Constants::FootLeftIkCurveName(),
		UGL_Constants::FootLeftLockCurveName(),
		UGL_Constants::FootRightIkCurveName(),
		UGL_Constants::FootRightLockCurveName(),
		UGL_Constants::FootPlantedCurveName(),
		UGL_Constants::FeetCrossingCurveName(),
		UGL_Constants::RotationYawSpeedCurveName(),
		UGL_Constants::RotationYawOffsetCurveName(),
		UGL_Constants::AllowTransitionsCurveName(),
		UGL_Constants::SprintBlockCurveName(),
		UGL_Constants::GroundPredictionBlockCurveName(),
		UGL_Constants::FootstepSoundBlockCurveName(),
	};
	return Allow;
}

void AGL_Character::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation)
{
	const float Scale = FMath::Min(
		float(Canvas->SizeX) / (1280.f * Canvas->GetDPIScale()),
		float(Canvas->SizeY) / (720.f * Canvas->GetDPIScale()));

	const float RowOffset = 12.f * Scale;
	const float ColumnOffset = 220.f * Scale;

	float MaxVerticalLocation = VerticalLocation;
	float HorizontalLocation = 5.f * Scale;

	static const FText DebugModeHeaderText{ LOCTEXT("DebugHeader", "GameplayLocomotion Debug - toggle with `ShowDebug`") };
	DisplayDebugHeader(Canvas, DebugModeHeaderText, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
	VerticalLocation += RowOffset;
	MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation);

	// If none of our categories is enabled, pass through to parent
	if (!DisplayInfo.IsDisplayOn(UGL_Constants::CurvesDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::CharacterStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::ViewStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::LocomotionStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::MovementBaseDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::LeanStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::GroundedStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::StandingStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::CrouchingStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::InAirStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::PoseStateDebugDisplayName()) &&
		!DisplayInfo.IsDisplayOn(UGL_Constants::TurnInPlaceDebugDisplayName()))
	{
		VerticalLocation = MaxVerticalLocation;
		Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
		return;
	}

	const float InitialVertical = VerticalLocation;

	// Helper lambda to print a section header & optionally a column with data
	auto DrawSection = [&](const FText& Header, const FName& ToggleName, auto DrawFn)
	{
		if (DisplayInfo.IsDisplayOn(ToggleName))
		{
			DisplayDebugHeader(Canvas, Header, FLinearColor::Green, Scale, HorizontalLocation, VerticalLocation);
			(DrawFn)(Canvas, Scale, HorizontalLocation, VerticalLocation);

			MaxVerticalLocation = FMath::Max(MaxVerticalLocation, VerticalLocation + RowOffset);
			VerticalLocation = InitialVertical;
			HorizontalLocation += ColumnOffset;
		}
		else
		{
			DisplayDebugHeader(Canvas, Header, FLinearColor(0.f, 0.33f, 0.f), Scale, HorizontalLocation, VerticalLocation);
			VerticalLocation += RowOffset;
		}
	};

	DrawSection(FText::FromString(TEXT("GL.Curves")), UGL_Constants::CurvesDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugCurves(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.CharacterState")), UGL_Constants::CharacterStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugCharacterState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.ViewState")), UGL_Constants::ViewStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugViewState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.LocomotionState")), UGL_Constants::LocomotionStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugLocomotionState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.MovementBase")), UGL_Constants::MovementBaseDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugMovementBase(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.LeanState")), UGL_Constants::LeanStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugLeanState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.GroundedState")), UGL_Constants::GroundedStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugGroundedState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.StandingState")), UGL_Constants::StandingStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugStandingState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.CrouchingState")), UGL_Constants::CrouchingStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugCrouchingState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.InAirState")), UGL_Constants::InAirStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugInAirState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.PoseState")), UGL_Constants::PoseStateDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugPoseState(C, S, X, Y); });
	DrawSection(FText::FromString(TEXT("GL.TurnInPlaceState")), UGL_Constants::TurnInPlaceDebugDisplayName(), [this](const UCanvas* C, float S, float X, float& Y) { DisplayDebugTurnInPlaceState(C, S, X, Y); });

	VerticalLocation = MaxVerticalLocation;

	Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
}

void AGL_Character::DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor,
	const float Scale, const float HorizontalLocation, float& VerticalLocation)
{
	FCanvasTextItem Text({ HorizontalLocation, VerticalLocation }, HeaderText, UEngine::GetMediumFont(), HeaderColor);
	Text.Scale = { Scale, Scale };
	Text.EnableShadow(FLinearColor::Black);
	Text.Draw(Canvas->Canvas);
	VerticalLocation += 15.f * Scale;
}

// -------- Raw Curves --------

void AGL_Character::DisplayDebugCurves(const UCanvas* Canvas, const float Scale,
	const float HorizontalLocation, float& VerticalLocation) const
{
	VerticalLocation += 4.f * Scale;

	FCanvasTextItem Text(FVector2D::ZeroVector, FText::GetEmpty(), UEngine::GetMediumFont(), FLinearColor::White);
	Text.Scale = { Scale * 0.75f, Scale * 0.75f };
	Text.EnableShadow(FLinearColor::Black);

	const float RowOffset = 12.f * Scale;
	const float ColumnOffset = 145.f * Scale;

	TArray<FName> CurveNames;
	if (const USkeletalMeshComponent* MainMesh = GetMesh())
	{
		if (const USkeletalMesh* SkelMesh = MainMesh->GetSkeletalMeshAsset())
		{
			if (const USkeleton* Skel = SkelMesh->GetSkeleton())
			{
				Skel->GetCurveMetaDataNames(CurveNames);
			}
		}
	}

	// Filter to allowlist
	const TSet<FName>& Allow = GLCurveAllowlist();
	CurveNames = CurveNames.FilterByPredicate([&Allow](const FName& N)
		{
			return Allow.Contains(N);
		});

	CurveNames.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	if (CurveNames.Num() == 0)
	{
		Text.SetColor(FLinearColor::Yellow);
		Text.Text = FText::FromString(TEXT("No curves matched filter"));
		Text.Draw(Canvas->Canvas, { HorizontalLocation, VerticalLocation });
		VerticalLocation += RowOffset;
		return;
	}

	TStringBuilder<32> Builder;

	for (const FName& CurveName : CurveNames)
	{
		const float Value = GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);

		Text.SetColor(FMath::Lerp(FLinearColor::Gray, FLinearColor::White, UGL_Math::Clamp01(Value)));
		Text.Text = FText::FromString(FName::NameToDisplayString(CurveName.ToString(), false));
		Text.Draw(Canvas->Canvas, { HorizontalLocation, VerticalLocation });

		Builder.Appendf(TEXT("%.2f"), Value);
		Text.Text = FText::FromString(FString{ Builder });
		Text.Draw(Canvas->Canvas, { HorizontalLocation + ColumnOffset, VerticalLocation });
		Builder.Reset();

		VerticalLocation += RowOffset;
	}
}

// -------- Helpers to print a name:value row quickly --------

static void GLDebugRow(const UCanvas* Canvas, float Scale, float X, float& Y,
	const TCHAR* Label, const FString& Value, const FLinearColor& Color = FLinearColor::White,
	float ColumnOffset = 160.f)
{
	FCanvasTextItem Text(FVector2D::ZeroVector, FText::GetEmpty(), UEngine::GetMediumFont(), Color);
	Text.Scale = { Scale * 0.75f, Scale * 0.75f };
	Text.EnableShadow(FLinearColor::Black);

	const float RowOffset = 12.f * Scale;

	Text.Text = FText::FromString(Label);
	Text.Draw(Canvas->Canvas, { X, Y });

	Text.Text = FText::FromString(Value);
	Text.Draw(Canvas->Canvas, { X + ColumnOffset * Scale, Y });

	Y += RowOffset;
}

static FString BToStr(bool b) { return b ? TEXT("True") : TEXT("False"); }
static FString F2(float V) { return FString::Printf(TEXT("%.2f"), V); }
static FString I32(int32 V) { return FString::Printf(TEXT("%d"), V); }
static FString NameOrNone(UObject* Obj) { return Obj ? Obj->GetName() : TEXT("None"); }

// -------- Each category reads UGL_AnimInstance and prints its state --------

void AGL_Character::DisplayDebugCharacterState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	GLDebugRow(Canvas, Scale, X, Y, TEXT("DesiredStance"), DesiredStance.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("DesiredGait"), DesiredGait.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("ViewMode"), ViewMode.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("OverlayMode"), OverlayMode.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("Aiming"), BToStr(bAiming));
	GLDebugRow(Canvas, Scale, X, Y, TEXT("LocomotionMode"), LocomotionMode.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("Stance"), Stance.ToString());
	GLDebugRow(Canvas, Scale, X, Y, TEXT("Gait"), Gait.ToString());

	GLDebugRow(Canvas, Scale, X, Y, TEXT("HasInput"), BToStr(LocomotionState.bHasInput));
	GLDebugRow(Canvas, Scale, X, Y, TEXT("InputYawAngle"), F2(LocomotionState.InputYawAngle));
	GLDebugRow(Canvas, Scale, X, Y, TEXT("VelocityYawAngle"), F2(LocomotionState.VelocityYawAngle));
	GLDebugRow(Canvas, Scale, X, Y, TEXT("Velocity (X/Y/Z)"),
		FString::Printf(TEXT("%.1f / %.1f / %.1f"), LocomotionState.Velocity.X, LocomotionState.Velocity.Y, LocomotionState.Velocity.Z));
}

void AGL_Character::DisplayDebugViewState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->ViewState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Rotation (R/P/Y)"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f"), S.Rotation.Roll, S.Rotation.Pitch, S.Rotation.Yaw));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("YawSpeed"), F2(S.YawSpeed));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("YawAngle"), F2(S.YawAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PitchAngle"), F2(S.PitchAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PitchAmount"), F2(S.PitchAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("LookAmount"), F2(S.LookAmount));
	}
}

void AGL_Character::DisplayDebugLocomotionState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->LocomotionState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Location (X/Y/Z)"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f"), S.Location.X, S.Location.Y, S.Location.Z));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Rotation Yaw"), F2(S.Rotation.Yaw));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("YawSpeed"), F2(S.YawSpeed));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Speed"), F2(S.Speed));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Scale"), F2(S.Scale));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Capsule R/H"), FString::Printf(TEXT("%.1f / %.1f"), S.CapsuleRadius, S.CapsuleHalfHeight));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("HasInput"), BToStr(S.bHasInput));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Moving"), BToStr(S.bMoving));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("MovingSmooth"), BToStr(S.bMovingSmooth));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("InputYaw"), F2(S.InputYawAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("VelYaw"), F2(S.VelocityYawAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("TargetYaw"), F2(S.TargetYawAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Velocity (X/Y/Z)"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f"), S.Velocity.X, S.Velocity.Y, S.Velocity.Z));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Acceleration (X/Y/Z)"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f"), S.Acceleration.X, S.Acceleration.Y, S.Acceleration.Z));
	}
}

void AGL_Character::DisplayDebugMovementBase(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->MovementBase;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Primitive"), NameOrNone(S.Primitive));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("BoneName"), S.BoneName.ToString());
		GLDebugRow(Canvas, Scale, X, Y, TEXT("BaseChanged"), BToStr(S.bBaseChanged));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("RelLocation"), BToStr(S.bHasRelativeLocation));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("RelRotation"), BToStr(S.bHasRelativeRotation));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("DeltaRot (R/P/Y)"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f"), S.DeltaRotation.Roll, S.DeltaRotation.Pitch, S.DeltaRotation.Yaw));
	}
}

void AGL_Character::DisplayDebugLeanState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->LeanState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Lean Right"), F2(S.RightAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Lean Forward"), F2(S.ForwardAmount));
	}
}

void AGL_Character::DisplayDebugGroundedState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->GroundedState;
		const EGL_MovementDirection MovementDirection = S.MovementDirection;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("VelBlend F/B/L/R"),
			FString::Printf(TEXT("%.2f / %.2f / %.2f / %.2f"),
				S.VelocityBlend.ForwardAmount, S.VelocityBlend.BackwardAmount,
				S.VelocityBlend.LeftAmount, S.VelocityBlend.RightAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("HipsDirLock"), F2(S.HipsDirectionLockAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("MoveDir"), GameplayEnumUtility::GetNameStringByValue(MovementDirection));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("YawOffsets F/B/L/R"),
			FString::Printf(TEXT("%.1f / %.1f / %.1f / %.1f"),
				S.RotationYawOffsets.ForwardAngle, S.RotationYawOffsets.BackwardAngle,
				S.RotationYawOffsets.LeftAngle, S.RotationYawOffsets.RightAngle));
	}
}

void AGL_Character::DisplayDebugStandingState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->StandingState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("StrideBlend"), F2(S.StrideBlendAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("WalkRunBlend"), F2(S.WalkRunBlendAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PlayRate"), F2(S.PlayRate));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("SprintBlock"), F2(S.SprintBlockAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("SprintTime"), F2(S.SprintTime));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("SprintAccel"), F2(S.SprintAccelerationAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PivotActive"), BToStr(S.bPivotActive));
	}
}

void AGL_Character::DisplayDebugCrouchingState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->CrouchingState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("StrideBlend"), F2(S.StrideBlendAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PlayRate"), F2(S.PlayRate));
	}
}

void AGL_Character::DisplayDebugInAirState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->InAirState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Jumped"), BToStr(S.bJumped));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("JumpRequested"), BToStr(S.bJumpRequested));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("JumpPlayRate"), F2(S.JumpPlayRate));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("VerticalVel"), F2(S.VerticalVelocity));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("GroundPredict"), F2(S.GroundPredictionAmount));
	}
}

void AGL_Character::DisplayDebugPoseState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& P = AI->PoseState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Grounded"), F2(P.GroundedAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("InAir"), F2(P.InAirAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Standing"), F2(P.StandingAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Crouching"), F2(P.CrouchingAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Moving"), F2(P.MovingAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Gait"), F2(P.GaitAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Gait W/R/S"),
			FString::Printf(TEXT("%.2f / %.2f / %.2f"),
				P.GaitWalkingAmount, P.GaitRunningAmount, P.GaitSprintingAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Unweighted Gait"), F2(P.UnweightedGaitAmount));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Unweighted W/R/S"),
			FString::Printf(TEXT("%.2f / %.2f / %.2f"),
				P.UnweightedGaitWalkingAmount, P.UnweightedGaitRunningAmount, P.UnweightedGaitSprintingAmount));
	}
}

void AGL_Character::DisplayDebugTurnInPlaceState(const UCanvas* Canvas, float Scale, float X, float& Y) const
{
	if (const UGL_AnimInstance* AI = Cast<UGL_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		const auto& S = AI->TurnInPlaceState;
		GLDebugRow(Canvas, Scale, X, Y, TEXT("UpdatedThisFrame"), BToStr(S.bUpdatedThisFrame));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("RootYawOffset"), F2(S.RootYawOffset));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("ActivationDelay"), F2(S.ActivationDelay));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("QueuedAngle"), F2(S.QueuedTurnYawAngle));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("PlayRate"), F2(S.PlayRate));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("QueuedSequence"), S.QueuedSequence ? S.QueuedSequence->GetName() : TEXT("None"));
		GLDebugRow(Canvas, Scale, X, Y, TEXT("SlotName"), S.QueuedSlotName.ToString());
		GLDebugRow(Canvas, Scale, X, Y, TEXT("Turning"), BToStr(S.bTurning));
	}
}

#undef LOCTEXT_NAMESPACE
