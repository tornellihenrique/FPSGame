#include "Animation/GL_LinkedAnimInstance.h"

#include "Components/SkeletalMeshComponent.h"

#include "Animation/GL_AnimInstance.h"
#include "Character/GL_Character.h"
#include "Animation/GL_AnimInstanceProxy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_LinkedAnimInstance)

UGL_LinkedAnimInstance::UGL_LinkedAnimInstance()
{
	bUseMainInstanceMontageEvaluationData = true;
}

void UGL_LinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Parent = Cast<UGL_AnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
	Character = Cast<AGL_Character>(GetOwningActor());

#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			if (!Parent.IsValid()) { Parent = GetMutableDefault<UGL_AnimInstance>(); }
			if (!IsValid(Character)) { Character = GetMutableDefault<AGL_Character>(); }
		}
	}
#endif
}

void UGL_LinkedAnimInstance::NativeBeginPlay()
{
	checkf(Parent.IsValid(), TEXT("%s must be used as a linked instance within %s!"),
		*GetClass()->GetName(), TEXT("UGL_AnimInstance"));
	Super::NativeBeginPlay();
}

FAnimInstanceProxy* UGL_LinkedAnimInstance::CreateAnimInstanceProxy()
{
	return new FGL_AnimInstanceProxy(this);
}

void UGL_LinkedAnimInstance::InitializeGrounded() { if (Parent.IsValid()) Parent->InitializeGrounded(); }
void UGL_LinkedAnimInstance::RefreshGrounded() { if (Parent.IsValid()) Parent->RefreshGrounded(); }
void UGL_LinkedAnimInstance::RefreshGroundedMovement() { if (Parent.IsValid()) Parent->RefreshGroundedMovement(); }
void UGL_LinkedAnimInstance::InitializeStandingMovement() { if (Parent.IsValid()) Parent->InitializeStandingMovement(); }
void UGL_LinkedAnimInstance::RefreshStandingMovement() { if (Parent.IsValid()) Parent->RefreshStandingMovement(); }
void UGL_LinkedAnimInstance::ResetPivot() { if (Parent.IsValid()) Parent->ResetPivot(); }
void UGL_LinkedAnimInstance::RefreshCrouchingMovement() { if (Parent.IsValid()) Parent->RefreshCrouchingMovement(); }
void UGL_LinkedAnimInstance::RefreshInAir() { if (Parent.IsValid()) Parent->RefreshInAir(); }
void UGL_LinkedAnimInstance::InitializeTurnInPlace() { if (Parent.IsValid()) Parent->InitializeTurnInPlace(); }
void UGL_LinkedAnimInstance::RefreshTurnInPlace() { if (Parent.IsValid()) Parent->RefreshTurnInPlace(); }
void UGL_LinkedAnimInstance::InitializeLean() { if (Parent.IsValid()) Parent->InitializeLean(); }
