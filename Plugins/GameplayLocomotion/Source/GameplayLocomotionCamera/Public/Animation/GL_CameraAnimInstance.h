#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "GL_CameraAnimInstance.generated.h"

class AGL_Character;
class UGL_CameraComponent;

UCLASS()
class GAMEPLAYLOCOMOTIONCAMERA_API UGL_CameraAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	TObjectPtr<AGL_Character> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	TObjectPtr<UGL_CameraComponent> Camera;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Expose whatever you want to drive curves via your anim graph (ViewMode, Stance, etc.)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	FGameplayTag ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	FGameplayTag LocomotionMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	FGameplayTag Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="State")
	FGameplayTag Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "State")
	bool bAiming;

};
