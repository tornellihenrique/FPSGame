#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Containers/UnrealString.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "GL_Utility.generated.h"

struct FBasedMovementInfo;

UCLASS()
class GAMEPLAYLOCOMOTION_API UGL_Utility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr FStringView BoolToString(bool bValue);

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Utility", Meta = (AutoCreateRefTerm = "Name", ReturnDisplayName = "Display String"))
	static FString NameToDisplayString(const FName& Name, bool bNameIsBool);

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Utility",
		Meta = (DefaultToSelf = "Character", AutoCreateRefTerm = "CurveName", ReturnDisplayName = "Curve Value"))
	static float GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName);

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Utility", Meta = (AutoCreateRefTerm = "Tag", ReturnDisplayName = "Child Tags"))
	static FGameplayTagContainer GetChildTags(const FGameplayTag& Tag);

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Utility", Meta = (AutoCreateRefTerm = "Tag", ReturnDisplayName = "Tag Name"))
	static FName GetSimpleTagName(const FGameplayTag& Tag);

	UFUNCTION(BlueprintPure, Category = "GameplayLocomotion|Utility", Meta = (WorldContext = "WorldContext", ReturnDisplayName = "Ping"))
	static float GetFirstPlayerPingSeconds(const UObject* WorldContext);

	static bool TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed);
	
};

constexpr FStringView UGL_Utility::BoolToString(const bool bValue)
{
	return bValue ? TEXTVIEW("True") : TEXTVIEW("False");
}

namespace GameplayEnumUtility
{
	template <class EnumType>
	concept CIsUEnumClass = static_cast<bool>(TIsUEnumClass<EnumType>::Value);

	template <CIsUEnumClass EnumType>
	int32 GetIndexByValue(const EnumType Value)
	{
		return StaticEnum<EnumType>()->GetIndexByValue(static_cast<int64>(Value));
	}

	template <CIsUEnumClass EnumType>
	FString GetNameStringByValue(const EnumType Value)
	{
		return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(Value));
	}
}
