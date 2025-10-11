#pragma once

#include "CoreMinimal.h"
#include "Animation/GL_AnimInstance.h"
#include "FPS_AnimInstance.generated.h"

class AFPS_Character;
class AGE_Equipment;

UCLASS()
class FPSGAME_V2_API UFPS_AnimInstance : public UGL_AnimInstance
{
	GENERATED_BODY()

public:
	UFPS_AnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaTime) override;
	
public:
	UPROPERTY(BlueprintReadOnly, Category="State")
	TObjectPtr<AFPS_Character> FPSCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category="State")
	uint8 bIsLocalPlayer : 1;
	
	UPROPERTY(BlueprintReadOnly, Category="State")
	uint8 bHasActiveEquipment : 1;
	
	UPROPERTY(BlueprintReadOnly, Category="State")
	TWeakObjectPtr<AGE_Equipment> ActiveEquipment;

public:
	UFUNCTION(BlueprintPure, Category="State", meta = (BlueprintThreadSafe))
	const FAnimState& GetAnimState() const;

};
