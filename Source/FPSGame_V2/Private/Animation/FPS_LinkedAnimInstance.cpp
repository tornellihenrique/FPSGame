#include "Animation/FPS_LinkedAnimInstance.h"

#include "Animation/FPS_AnimInstance.h"
#include "Character/FPS_Character.h"

UFPS_LinkedAnimInstance::UFPS_LinkedAnimInstance()
{
    
}

void UFPS_LinkedAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FPSParent = Cast<UFPS_AnimInstance>(GetSkelMeshComponent()->GetAnimInstance());
	FPSCharacter = Cast<AFPS_Character>(GetOwningActor());
}

void UFPS_LinkedAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}
