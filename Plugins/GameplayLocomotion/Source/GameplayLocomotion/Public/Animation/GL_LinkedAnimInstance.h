#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GL_LinkedAnimInstance.generated.h"

class UGL_AnimInstance;
class AGL_Character;

UCLASS()
class GAMEPLAYLOCOMOTION_API UGL_LinkedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category="State", Transient)
	TWeakObjectPtr<UGL_AnimInstance> Parent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	TObjectPtr<AGL_Character> Character;

public:
	UGL_LinkedAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;

public:
	UFUNCTION(BlueprintPure, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe, ReturnDisplayName="Parent"))
	UGL_AnimInstance* GetParent() const { return Parent.Get(); }

	// Passthroughs used by your anim graph
	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void InitializeGrounded();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshGrounded();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshGroundedMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void InitializeStandingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshStandingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void ResetPivot();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshCrouchingMovement();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshInAir();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void InitializeTurnInPlace();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void RefreshTurnInPlace();

	UFUNCTION(BlueprintCallable, Category="Gameplay|LinkedAnim", Meta=(BlueprintThreadSafe))
	void InitializeLean();

};
