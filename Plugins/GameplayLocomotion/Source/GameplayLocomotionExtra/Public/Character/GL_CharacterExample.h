// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GL_Character.h"
#include "GL_CharacterExample.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UGL_CameraComponent;

UCLASS()
class GAMEPLAYLOCOMOTIONEXTRA_API AGL_CharacterExample : public AGL_Character
{
	GENERATED_BODY()

	UPROPERTY(Category = "Character Example", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGL_CameraComponent> Camera;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Look;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Aim;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Sprint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Walk;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Crouch;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Input")
	TObjectPtr<UInputAction> IA_View;

public:
	AGL_CharacterExample(const FObjectInitializer& ObjectInitializer);

	//~ AActor
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo) override;

	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EndViewTarget(APlayerController* PC) override;
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

protected:
	virtual void Input_Move(const FInputActionValue& Value);
	virtual void Input_Look(const FInputActionValue& Value);
	virtual void Input_Sprint(const FInputActionValue& ActionValue);
	virtual void Input_Aim(const FInputActionValue& ActionValue);
	virtual void Input_Walk();
	virtual void Input_Crouch();
	virtual void Input_Jump(const FInputActionValue& ActionValue);
	virtual void Input_View();

};
