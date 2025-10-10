// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GL_Character.h"
#include "FPS_Character.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UGL_CameraComponent;
class UFPS_HealthComponent;
struct FDeathEventPayload;

UCLASS()
class FPSGAME_V2_API AFPS_Character : public AGL_Character
{
	GENERATED_BODY()

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UGL_CameraComponent> CameraComponent;

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UFPS_HealthComponent> HealthComponent;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Look;

public:
	AFPS_Character(const FObjectInitializer& ObjectInitializer);

	//~ AActor
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EndViewTarget(APlayerController* PC) override;

	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult);
	//~ End of AActor
	
	//~ APawn
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void Restart() override;
	virtual void PawnClientRestart() override;

	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
	//~ End of APawn

	//~ AGL_Character
	//~ End of AGL_Character

protected:
	UFUNCTION()
	void HandleDeathPayload(const FDeathEventPayload& Data);

	UFUNCTION(NetMulticast, Reliable)
	void NetHandleDeathPayload(const FDeathEventPayload& Data);

protected:
	virtual void Input_Move(const FInputActionValue& Value);
	virtual void Input_Look(const FInputActionValue& Value);

};
