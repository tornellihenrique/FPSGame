#include "Animation/GL_CameraAnimInstance.h"

#include "Character/GL_Character.h"
#include "Components/GL_CameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_CameraAnimInstance)

void UGL_CameraAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AGL_Character>(GetOwningActor());
	Camera = Cast<UGL_CameraComponent>(GetSkelMeshComponent());

#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			if (!Character) { Character = GetMutableDefault<AGL_Character>(); }
			if (!Camera) { Camera = GetMutableDefault<UGL_CameraComponent>(); }
		}
	}
#endif
}

void UGL_CameraAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character || !Camera) return;

	ViewMode = Character->ViewMode;
	LocomotionMode = Character->LocomotionMode;
	Stance = Character->Stance;
	Gait = Character->Gait;
	bAiming = Character->bAiming;
}
