#include "Utility/GL_Utility.h"

#include "GameplayTagsManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_Utility)

FString UGL_Utility::NameToDisplayString(const FName& Name, const bool bNameIsBool)
{
	return FName::NameToDisplayString(Name.ToString(), bNameIsBool);
}

float UGL_Utility::GetAnimationCurveValueFromCharacter(const ACharacter* Character, const FName& CurveName)
{
	const USkeletalMeshComponent* Mesh{ IsValid(Character) ? Character->GetMesh() : nullptr };
	const UAnimInstance* AnimationInstance{ IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr };

	return ensure(IsValid(AnimationInstance)) ? AnimationInstance->GetCurveValue(CurveName) : 0.0f;
}

FGameplayTagContainer UGL_Utility::GetChildTags(const FGameplayTag& Tag)
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Tag);
}

FName UGL_Utility::GetSimpleTagName(const FGameplayTag& Tag)
{
	const auto TagNode{ UGameplayTagsManager::Get().FindTagNode(Tag) };

	return TagNode.IsValid() ? TagNode->GetSimpleTagName() : NAME_None;
}

float UGL_Utility::GetFirstPlayerPingSeconds(const UObject* WorldContext)
{
	const UWorld* World{ IsValid(WorldContext) ? WorldContext->GetWorld() : nullptr };
	const APlayerController* Player{ IsValid(World) ? World->GetFirstPlayerController() : nullptr };
	const APlayerState* PlayerState{ IsValid(Player) ? Player->PlayerState.Get() : nullptr };

	return IsValid(PlayerState) ? PlayerState->GetPingInMilliseconds() * 0.001f : 0.0f;
}

bool UGL_Utility::TryGetMovementBaseRotationSpeed(const FBasedMovementInfo& BasedMovement, FRotator& RotationSpeed)
{
	if (!MovementBaseUtility::IsDynamicBase(BasedMovement.MovementBase))
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const FBodyInstance* Body{ BasedMovement.MovementBase->GetBodyInstance(BasedMovement.BoneName) };
	if (Body == nullptr)
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	const FVector AngularVelocityVector{ Body->GetUnrealWorldAngularVelocityInRadians() };
	if (AngularVelocityVector.IsNearlyZero())
	{
		RotationSpeed = FRotator::ZeroRotator;
		return false;
	}

	RotationSpeed.Roll = FMath::RadiansToDegrees(AngularVelocityVector.X);
	RotationSpeed.Pitch = FMath::RadiansToDegrees(AngularVelocityVector.Y);
	RotationSpeed.Yaw = FMath::RadiansToDegrees(AngularVelocityVector.Z);

	return true;
}


