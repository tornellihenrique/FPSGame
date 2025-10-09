#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "Misc/GL_Types.h"
#include "GL_AnimInstance.generated.h"

class UCurveFloat;
class AGL_Character;

UCLASS()
class GAMEPLAYLOCOMOTION_API UGL_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend class AGL_Character;
	friend class UGL_LinkedAnimInstance;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|General")
	FGL_AnimGeneralSettings General;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Grounded")
	FGL_AnimGroundedSettings Grounded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Standing")
	FGL_AnimStandingSettings Standing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Crouching")
	FGL_AnimCrouchingSettings Crouching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|InAir")
	FGL_AnimInAirSettings InAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|TurnInPlace")
	FGL_TurnInPlaceGeneralSettings TurnInPlace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|TurnInPlace")
	FGL_TurnInPlaceAnimSettings StandingTurn90Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|TurnInPlace")
	FGL_TurnInPlaceAnimSettings StandingTurn90Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|TurnInPlace")
	FGL_TurnInPlaceAnimSettings CrouchingTurn90Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|TurnInPlace")
	FGL_TurnInPlaceAnimSettings CrouchingTurn90Right;

protected:
	// Character / state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	TObjectPtr<AGL_Character> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	uint8 bPendingUpdate : 1 { true };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, Meta=(ClampMin=0))
	double TeleportedTime = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag LocomotionMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag OverlayMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	bool bAiming;

	// Sub-states
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_MovementBaseState MovementBase;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_ViewAnimState ViewState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_LocomotionAnimState LocomotionState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_LeanState LeanState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_GroundedState GroundedState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_StandingState StandingState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_PoseState PoseState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_CrouchingState CrouchingState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_InAirState InAirState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_TurnInPlaceState TurnInPlaceState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient) FGL_LayeringState LayeringState;

public:
	// Core overrides
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativePostUpdateAnimation();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

public:
	// Utilities
	void MarkPendingUpdate() { bPendingUpdate |= true; }
	void MarkTeleported() { TeleportedTime = GetWorld()->GetTimeSeconds(); }
	void Jump() { InAirState.bJumpRequested = true; }

	// Graph-callable API
	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void InitializeGrounded();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshGrounded();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshGroundedMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void InitializeStandingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshStandingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void ResetPivot() { StandingState.bPivotActive = false; }

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshCrouchingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshInAir();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void InitializeTurnInPlace();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void RefreshTurnInPlace();

	UFUNCTION(BlueprintCallable, Category="Gameplay|Anim", Meta=(BlueprintThreadSafe))
	void InitializeLean();

protected:
	// Internals
	void RefreshMovementBaseOnGameThread();
	void RefreshLocomotionOnGameThread();
	void RefreshViewOnGameThread();
	void RefreshView(float DeltaSeconds);
	void RefreshVelocityBlend();
	void RefreshGroundedLean();
	void RefreshPoseState();
	void RefreshMovementDirection(float ViewRelativeVelocityYawAngle);
	void RefreshRotationYawOffsets(float ViewRelativeVelocityYawAngle);
	void RefreshInAirOnGameThread();
	void RefreshGroundPrediction();
	void RefreshInAirLean();
	void RefreshLayering();

	// Helpers
	FVector3f GetRelativeVelocity() const;
	FVector2f GetRelativeAccelerationAmount() const;

};
