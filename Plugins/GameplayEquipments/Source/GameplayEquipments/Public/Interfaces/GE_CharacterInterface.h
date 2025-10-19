// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GE_CharacterInterface.generated.h"

class AGE_Equipment;

UINTERFACE(MinimalAPI)
class UGE_CharacterInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMEPLAYEQUIPMENTS_API IGE_CharacterInterface
{
	GENERATED_BODY()

public:
	virtual AGE_Equipment* GetCurrentEquipment() const { return nullptr; }

	virtual USkeletalMeshComponent* GetMeshFP() const { return nullptr; }

	virtual USceneComponent* GetEquipmentHolderFP() const { return nullptr; }
	virtual USceneComponent* GetEquipmentHolderTP() const { return nullptr; }

	virtual void SetIsAimingEquipment(bool bNewIsAiming) {}

	virtual bool ShouldUseRunFireDelay() const { return false; }

};
