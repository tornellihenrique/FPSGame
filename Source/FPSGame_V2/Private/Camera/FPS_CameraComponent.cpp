// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/FPS_CameraComponent.h"

#include "Character/FPS_Character.h"

void UFPS_CameraComponent::GetViewInfo(FMinimalViewInfo& ViewInfo) const
{
	Super::GetViewInfo(ViewInfo);

	ViewInfo.FirstPersonFOV = FirstPersonRenderingFOV;
	ViewInfo.FirstPersonScale = FirstPersonRenderingScale;
	ViewInfo.bUseFirstPersonParameters = true;
}

FVector UFPS_CameraComponent::GetFirstPersonCameraLocation() const
{
	if (const AFPS_Character* FPSCharacter = Cast<AFPS_Character>(Character))
	{
		return FPSCharacter->GetMeshFP()->GetSocketLocation(FirstPersonSocketName);
	}

	return Super::GetFirstPersonCameraLocation();
}
