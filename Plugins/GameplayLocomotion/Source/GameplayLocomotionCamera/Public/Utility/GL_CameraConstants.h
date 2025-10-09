#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GL_CameraConstants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class GAMEPLAYLOCOMOTIONCAMERA_API UGL_CameraConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:
	// Animation Curves

	inline static const FName CameraOffsetXCurve{TEXTVIEW("CameraOffsetX")};
	inline static const FName CameraOffsetYCurve{TEXTVIEW("CameraOffsetY")};
	inline static const FName CameraOffsetZCurve{TEXTVIEW("CameraOffsetZ")};
	inline static const FName FovOffsetCurve{TEXTVIEW("FovOffset")};
	inline static const FName PivotOffsetXCurve{TEXTVIEW("PivotOffsetX")};
	inline static const FName PivotOffsetYCurve{TEXTVIEW("PivotOffsetY")};
	inline static const FName PivotOffsetZCurve{TEXTVIEW("PivotOffsetZ")};
	inline static const FName LocationLagXCurve{TEXTVIEW("LocationLagX")};
	inline static const FName LocationLagYCurve{TEXTVIEW("LocationLagY")};
	inline static const FName LocationLagZCurve{TEXTVIEW("LocationLagZ")};
	inline static const FName RotationLagCurve{TEXTVIEW("RotationLag")};
	inline static const FName FirstPersonOverrideCurve{TEXTVIEW("FirstPersonOverride")};
	inline static const FName TraceOverrideCurve{TEXTVIEW("TraceOverride")};

	// Debug

	inline static const FName CameraCurvesDebugDisplay{TEXTVIEW("GameplayCamera.CameraCurves")};
	inline static const FName CameraShapesDebugDisplay{TEXTVIEW("GameplayCamera.CameraShapes")};
	inline static const FName CameraTracesDebugDisplay{TEXTVIEW("GameplayCamera.CameraTraces")};

public:
	// Animation Curves

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& CameraOffsetXCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& CameraOffsetYCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& CameraOffsetZCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FovOffsetCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PivotOffsetXCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PivotOffsetYCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& PivotOffsetZCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LocationLagXCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LocationLagYCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& LocationLagZCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& RotationLagCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& FirstPersonOverrideCurveName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static const FName& TraceOverrideCurveName();

	// Debug

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CameraCurvesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CameraShapesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category="GameplayCamera|Camera Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static const FName& CameraTracesDebugDisplayName();
};

inline const FName& UGL_CameraConstants::CameraOffsetXCurveName()
{
	return CameraOffsetXCurve;
}

inline const FName& UGL_CameraConstants::CameraOffsetYCurveName()
{
	return CameraOffsetYCurve;
}

inline const FName& UGL_CameraConstants::CameraOffsetZCurveName()
{
	return CameraOffsetZCurve;
}

inline const FName& UGL_CameraConstants::FovOffsetCurveName()
{
	return FovOffsetCurve;
}

inline const FName& UGL_CameraConstants::PivotOffsetXCurveName()
{
	return PivotOffsetXCurve;
}

inline const FName& UGL_CameraConstants::PivotOffsetYCurveName()
{
	return PivotOffsetYCurve;
}

inline const FName& UGL_CameraConstants::PivotOffsetZCurveName()
{
	return PivotOffsetZCurve;
}

inline const FName& UGL_CameraConstants::LocationLagXCurveName()
{
	return LocationLagXCurve;
}

inline const FName& UGL_CameraConstants::LocationLagYCurveName()
{
	return LocationLagYCurve;
}

inline const FName& UGL_CameraConstants::LocationLagZCurveName()
{
	return LocationLagZCurve;
}

inline const FName& UGL_CameraConstants::RotationLagCurveName()
{
	return RotationLagCurve;
}

inline const FName& UGL_CameraConstants::FirstPersonOverrideCurveName()
{
	return FirstPersonOverrideCurve;
}

inline const FName& UGL_CameraConstants::TraceOverrideCurveName()
{
	return TraceOverrideCurve;
}

inline const FName& UGL_CameraConstants::CameraCurvesDebugDisplayName()
{
	return CameraCurvesDebugDisplay;
}

inline const FName& UGL_CameraConstants::CameraShapesDebugDisplayName()
{
	return CameraShapesDebugDisplay;
}

inline const FName& UGL_CameraConstants::CameraTracesDebugDisplayName()
{
	return CameraTracesDebugDisplay;
}
