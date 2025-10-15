#include "Animation/FPS_AnimInstance.h"

#include "Character/FPS_Character.h"
#include "Equipments/GE_Equipment.h"

UFPS_AnimInstance::UFPS_AnimInstance()
{
    
}

void UFPS_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	FPSCharacter = Cast<AFPS_Character>(GetOwningActor());
}

void UFPS_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!IsValid(FPSCharacter)) return;

	bIsLocalPlayer = FPSCharacter->IsLocallyControlled() && FPSCharacter->IsPlayerControlled();

	if (AGE_Equipment* Equipment = FPSCharacter->GetCurrentEquipment())
	{
		ActiveEquipment = Equipment;
		bHasActiveEquipment = true;
	}
	else
	{
		ActiveEquipment.Reset();
		bHasActiveEquipment = false;
	}
}

void UFPS_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaTime)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaTime);
}

FGA_ProceduralInput UFPS_AnimInstance::GetProceduralMotionInput() const
{
	FGA_ProceduralInput Out;

	if (IsValid(FPSCharacter))
	{
		Out.SourceActorOverride = FPSCharacter->GetCurrentEquipment();
		Out.PivotOffset = GetAnimState().PivotOffset;
		Out.bFirstPerson = bFirstPerson;
	}

	return Out;
}

const FAnimState& UFPS_AnimInstance::GetAnimState() const
{
	if (IsValid(FPSCharacter))
	{
		return FPSCharacter->GetAnimState();
	}

	static FAnimState DefaultAnimState;
	return DefaultAnimState;
}
