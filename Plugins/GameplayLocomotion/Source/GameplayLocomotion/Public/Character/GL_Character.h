// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Misc/GL_Types.h"
#include "GL_Character.generated.h"

class UGL_MovementSettings;
class UGL_CharacterMovementComponent;

UCLASS()
class GAMEPLAYLOCOMOTION_API AGL_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AGL_Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// === Desired state (replicated to non-owners) ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Locomotion", ReplicatedUsing=OnRep_DesiredStance)
	FGameplayTag DesiredStance;           // Stance.Standing / .Crouching

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Locomotion", ReplicatedUsing=OnRep_DesiredGait)
	FGameplayTag DesiredGait;             // Gait.Walking / .Running / .Sprinting

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Locomotion", ReplicatedUsing=OnRep_ViewMode)
	FGameplayTag ViewMode;                // ViewMode.FirstPerson / .ThirdPerson (camera-only)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Locomotion", ReplicatedUsing=OnRep_OverlayMode)
	FGameplayTag OverlayMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Locomotion", ReplicatedUsing=OnRep_Aiming)
	uint8 bAiming : 1;

	// === Current runtime state (not all replicated) ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag LocomotionMode;          // Grounded / InAir

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag Stance;                  // current stance

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag Gait;                    // coarse gait tag from speed, optional

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGL_LocomotionState LocomotionState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag LocomotionAction;

	// Replicate input dir so simulated proxies can compute InputYawAngle if needed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, Replicated)
	FVector_NetQuantizeNormal InputDirection = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings|Locomotion")
	TObjectPtr<UGL_MovementSettings> MovementSettings;

	UGL_CharacterMovementComponent* GLMovement() const;

public: // API
	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void SetDesiredStance(const FGameplayTag& NewStance);

	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void SetDesiredGait(const FGameplayTag& NewGait);

	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void SetViewModeTag(const FGameplayTag& NewViewMode);

	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void SetOverlayModeTag(const FGameplayTag& NewOverlayMode);

	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void SetAiming(bool bNewAiming);
	
	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion")
	virtual void StartRagdolling();

	UFUNCTION(BlueprintCallable, Category = "GameplayLocomotion", meta=(ReturnDisplayName="Success"))
	virtual bool StopRagdolling();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreRegisterAllComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void PostRegisterAllComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void Jump() override;
	virtual void OnJumped_Implementation() override;

	void RefreshInput(float DeltaSeconds);
	void RefreshLocomotion(float DeltaSeconds);
	void RefreshGait();
	void RefreshGaitFromSpeed();

	virtual void ApplyDesiredStance();

	virtual FGameplayTag CalculateMaxAllowedGait() const;
	virtual bool CanRun() const;
	virtual bool CanSprint() const;

	virtual bool IsRagdollingAllowedToStart() const;
	virtual bool IsRagdollingAllowedToStop() const;

	virtual void StartRagdollingImplementation();
	virtual void StopRagdollingImplementation();

	virtual FVector RagdollTraceGround(bool& bGrounded) const;

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetDesiredStance(FGameplayTag NewStance);

	UFUNCTION(Server, Reliable)
	void ServerSetDesiredGait(FGameplayTag NewGait);

	UFUNCTION(Server, Reliable)
	void ServerSetViewMode(FGameplayTag NewViewMode);

	UFUNCTION(Server, Reliable)
	void ServerSetOverlayMode(FGameplayTag NewOverlayMode);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerStartRagdolling();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartRagdolling();
	
	UFUNCTION(Server, Reliable)
	void ServerStopRagdolling();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopRagdolling();

	UFUNCTION()
	virtual void OnRep_DesiredStance();

	UFUNCTION()
	virtual void OnRep_DesiredGait();

	UFUNCTION()
	virtual void OnRep_ViewMode();

	UFUNCTION()
	virtual void OnRep_OverlayMode(const FGameplayTag& PreviousOverlayMode);

	UFUNCTION()
	virtual void OnRep_Aiming(bool bPreviousAiming);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnJumpedNetworked();

	virtual void OnJumpedNetworked();
	
	UFUNCTION(BlueprintNativeEvent, Category = "GL Character")
	void OnOverlayModeChanged(const FGameplayTag& PreviousOverlayMode);
	
	UFUNCTION(BlueprintNativeEvent, Category = "GL Character")
	void OnAimingChanged(bool bPreviousAiming);

public:
	// ShowDebug entry
	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation) override;

private:
	// Section headers
	static void DisplayDebugHeader(const UCanvas* Canvas, const FText& HeaderText, const FLinearColor& HeaderColor, float Scale, float HorizontalLocation, float& VerticalLocation);

	// Categories
	void DisplayDebugCurves(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

	void DisplayDebugCharacterState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugViewState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugLocomotionState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugMovementBase(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugLeanState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugGroundedState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugStandingState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugCrouchingState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugInAirState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugPoseState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;
	void DisplayDebugTurnInPlaceState(const UCanvas* Canvas, float Scale, float HorizontalLocation, float& VerticalLocation) const;

};
