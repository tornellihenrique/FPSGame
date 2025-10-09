#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/GL_MovementSettings.h"
#include "Misc/GL_GameplayTags.h"
#include "GameplayTagContainer.h"
#include "GL_CharacterMovementComponent.generated.h"

using FGL_PhysicsRotationDelegate = TMulticastDelegate<void(float /*DeltaTime*/)>;

// SavedMove / MoveData to carry stance and max gait through prediction (client->server)
class GAMEPLAYLOCOMOTION_API FGL_SavedMove : public FSavedMove_Character
{
private:
	using Super = FSavedMove_Character;

public:
	FGameplayTag Stance{GameplayStanceTags::Standing};
	FGameplayTag MaxAllowedGait{GameplayGaitTags::Running};

	virtual void Clear() override;

	virtual void SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration,
		FNetworkPredictionData_Client_Character& PredictionData) override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDeltaTime) const override;

	virtual void PrepMoveFor(ACharacter* Character) override;
};

class FGL_NetworkPredictionData : public FNetworkPredictionData_Client_Character
{
private:
	using Super = FNetworkPredictionData_Client_Character;

public:
	explicit FGL_NetworkPredictionData(const UCharacterMovementComponent& Movement)
		: Super(Movement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return MakeShareable(new FGL_SavedMove());
	}
};

UCLASS(ClassGroup = (GameplayLocomotion), meta = (BlueprintSpawnableComponent))
class GAMEPLAYLOCOMOTION_API UGL_CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend FGL_SavedMove;

public:
	UGL_CharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// Expose movement settings
	UFUNCTION(BlueprintCallable, Category="GameplayLocomotion|Movement")
	void SetMovementSettings(UGL_MovementSettings* NewSettings);

	FORCEINLINE const FGL_GaitSettings& GetGaitSettings() const { return GaitSettings; }

	// State
	FORCEINLINE const FGameplayTag& GetStance() const { return Stance; }
	FORCEINLINE void SetStance(const FGameplayTag& NewStance) { if (Stance != NewStance) { Stance = NewStance; RefreshGaitSettings(); } }

	FORCEINLINE const FGameplayTag& GetMaxAllowedGait() const { return MaxAllowedGait; }
	FORCEINLINE void SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait) { MaxAllowedGait = NewMaxAllowedGait; }

	// 0..3 (0 stop, 1 walk, 2 run, 3 sprint)
	FORCEINLINE float GetGaitAmount() const { return GaitAmount; }

	// Optional control hooks
	void SetMovementModeLocked(bool bLocked) { bMovementModeLocked = bLocked; }
	void SetInputBlocked(bool bBlocked) { bInputBlocked = bBlocked; }

	FGL_PhysicsRotationDelegate OnPhysicsRotation;

public: // UCharacterMovementComponent
	virtual void InitializeComponent() override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void PhysicsRotation(float DeltaTime) override;
	virtual void MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult = nullptr) override;
	virtual float GetMaxAcceleration() const override;
	virtual bool ApplyRequestedMove(float DeltaTime, float CurrentMaxAcceleration, float MaxSpeed, float Friction, float BrakingDeceleration, FVector& RequestedAcceleration, float& RequestedSpeed) override;
	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual FVector ConsumeInputVector() override;

protected:
	// Tuning refresh
	void RefreshGaitSettings();
	void RefreshGroundedMovementSettings();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	TObjectPtr<UGL_MovementSettings> MovementSettings = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGL_GaitSettings GaitSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	FGameplayTag MaxAllowedGait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, meta=(ClampMin=0, ClampMax=3))
	float GaitAmount = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient, meta=(ClampMin=0, ForceUnits="cm/s^2"))
	float MaxAccelerationWalking = 0.f;

	// Quality-of-life switches
	UPROPERTY(EditAnywhere, Category="Settings")
	uint8 bMovementModeLocked : 1;

	UPROPERTY(EditAnywhere, Category="Settings")
	uint8 bInputBlocked : 1;

	FRotator PreviousControlRotation = FRotator::ZeroRotator;

};
