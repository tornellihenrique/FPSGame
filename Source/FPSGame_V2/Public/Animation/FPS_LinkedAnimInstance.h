#pragma once

#include "CoreMinimal.h"
#include "Animation/GL_LinkedAnimInstance.h"
#include "FPS_LinkedAnimInstance.generated.h"

class UFPS_AnimInstance;
class AFPS_Character;

UCLASS()
class FPSGAME_V2_API UFPS_LinkedAnimInstance : public UGL_LinkedAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category="State", Transient)
	TWeakObjectPtr<UFPS_AnimInstance> FPSParent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", Transient)
	TObjectPtr<AFPS_Character> FPSCharacter;

public:
	UFPS_LinkedAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;

public:
	UFUNCTION(BlueprintPure, Category="FPS|LinkedAnim", Meta=(BlueprintThreadSafe, ReturnDisplayName="Parent"))
	UFPS_AnimInstance* GetFPSParent() const { return FPSParent.Get(); }

};
