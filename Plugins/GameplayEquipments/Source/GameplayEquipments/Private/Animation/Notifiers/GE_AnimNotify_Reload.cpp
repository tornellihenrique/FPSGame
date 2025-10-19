// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifiers/GE_AnimNotify_Reload.h"

#include "Interfaces/GE_CharacterInterface.h"
#include "Equipments/GE_FireWeapon.h"

UGE_AnimNotify_Reload::UGE_AnimNotify_Reload()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif

	bIsNativeBranchingPoint = true;
}

void UGE_AnimNotify_Reload::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(Mesh, Animation, EventReference);

	if (IGE_CharacterInterface* CharacterInterface = Cast<IGE_CharacterInterface>(Mesh->GetOwner()))
	{
		if (AGE_Equipment* Equipment = CharacterInterface->GetCurrentEquipment())
		{
			if (AGE_FireWeapon* Weapon = Cast<AGE_FireWeapon>(Equipment))
			{
				Weapon->Notify_CommitReload();
			}
		}
	}
}
