// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GL_CameraComponent.h"
#include "FPS_CameraComponent.generated.h"

UCLASS()
class FPSGAME_V2_API UFPS_CameraComponent : public UGL_CameraComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Camera|FirstPerson")
	float FirstPersonRenderingFOV = 90.0f;
	UPROPERTY(EditAnywhere, Category="Camera|FirstPerson")
	float FirstPersonRenderingScale = 0.2f;

public:
	virtual void GetViewInfo(FMinimalViewInfo& ViewInfo) const override;

	virtual FVector GetFirstPersonCameraLocation() const override;
	
};
