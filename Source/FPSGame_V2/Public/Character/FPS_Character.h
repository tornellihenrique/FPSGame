// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/GL_Character.h"
#include "Interfaces/GE_CharacterInterface.h"
#include "Misc/FPS_Types.h"
#include "FPS_Character.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UFPS_CameraComponent;
class UFPS_HealthComponent;
struct FDeathEventPayload;
class UGE_EquipmentManagerComponent;

UCLASS()
class FPSGAME_V2_API AFPS_Character : public AGL_Character
	, public IGE_CharacterInterface
{
	GENERATED_BODY()

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UFPS_CameraComponent> CameraComponent;
	
	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshFP;
	
	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> EquipmentHolderTP;
	
	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> EquipmentHolderFP;

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UFPS_HealthComponent> HealthComponent;

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UGE_EquipmentManagerComponent> EquipmentManager;

public:
	UPROPERTY(EditDefaultsOnly, Category="Settings|Animation")
	TObjectPtr<UAnimSequence> EmptyPoseFP;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Animation")
	TObjectPtr<UAnimSequence> EmptyPoseTP;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Move;
	
	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Sprint;
	
	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Walk;
	
	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Crouch;
	
	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_View;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Cycle;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Slot;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Primary;

	UPROPERTY(EditDefaultsOnly, Category="Settings|Input")
	TObjectPtr<UInputAction> IA_Secondary;

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
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
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
	virtual bool CanRun() const override;
	virtual void OnJumpedNetworked() override;
	virtual void OnRep_ViewMode() override;
	//~ End of AGL_Character

	//~ IGE_CharacterInterface
	virtual AGE_Equipment* GetCurrentEquipment() const override;

	virtual USkeletalMeshComponent* GetMeshFP() const override { return MeshFP; }

	virtual USceneComponent* GetEquipmentHolderFP() const override { return EquipmentHolderFP; }
	virtual USceneComponent* GetEquipmentHolderTP() const override { return EquipmentHolderTP; }

	virtual void SetIsAimingEquipment(bool bNewIsAiming) override { SetAiming(bNewIsAiming); }

	virtual bool ShouldUseRunFireDelay() const override;
	//~ IGE_CharacterInterface

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Character|Animation")
	FAnimState AnimState;
	
	UPROPERTY()
	int32 EquipmentsSpawned;

	UPROPERTY(Replicated)
	int32 EquipmentsCount;

public:
	virtual void PopulateLoadout(const FPlayerLoadout& PlayerLoadout);

protected:
	UFUNCTION()
	void HandleDeathPayload(const FDeathEventPayload& Data);

	UFUNCTION(NetMulticast, Reliable)
	void NetHandleDeathPayload(const FDeathEventPayload& Data);
	
	UFUNCTION()
	void OnEquipmentChanged(AGE_Equipment* NewEquipment, AGE_Equipment* OldEquipment);
	UFUNCTION()
	void OnEquipmentAdded(AGE_Equipment* Equipment, int32 SlotIndex);
	UFUNCTION()
	void OnEquipmentRemoved(AGE_Equipment* Equipment, int32 SlotIndex);

protected:
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_Sprint(const FInputActionValue& ActionValue);
	void Input_Walk();
	void Input_Crouch();
	void Input_Jump(const FInputActionValue& ActionValue);
	void Input_View();

	void Input_Cycle(const FInputActionValue& ActionValue);
	void Input_Slot(const FInputActionValue& ActionValue);

	void Input_Primary(const FInputActionValue& ActionValue);
	void Input_Secondary(const FInputActionValue& ActionValue);

public:
	UFUNCTION(BlueprintPure, Category="Character|Animation")
	const FAnimState& GetAnimState() const { return AnimState; }
	
	UFUNCTION(BlueprintPure, Category="Character")
	bool IsOnFirstPersonView() const;

};
