#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/GE_Types.h"
#include "GE_Equipment.generated.h"

class UGE_EquipmentAnimData;
class IGE_CharacterInterface;
class UGE_EquipmentManagerComponent;
class ACharacter;

UENUM(BlueprintType)
enum class EGE_EquipmentState : uint8
{
	Idle,
	Equipping,
	Equipped,
	Unequipping
};

UCLASS(Abstract, meta=(DisplayName="Equipment"))
class GAMEPLAYEQUIPMENTS_API AGE_Equipment : public AActor
{
	GENERATED_BODY()

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshFP;

	UPROPERTY(Category=Components, VisibleDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> MeshTP;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Equipment")
	TObjectPtr<UGE_EquipmentAnimData> AnimData;

public:
	AGE_Equipment(const FObjectInitializer& ObjectInitializer);

	//~ AActor
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnRep_Owner() override;
	virtual void OnRep_Instigator() override;

	virtual void SetActorHiddenInGame(bool bNewHidden) override;
	//~ End of AActor

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment|State", ReplicatedUsing=OnRep_EquipState)
	EGE_EquipmentState EquipState = EGE_EquipmentState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment|State", ReplicatedUsing=OnRep_Equipped)
	uint8 bEquipped : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment|State")
	uint8 bOwnerIsFirstPerson : 1;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TScriptInterface<IGE_CharacterInterface> OwningCharacterInterface;

public:
	UFUNCTION(BlueprintPure, Category="Helper")
	virtual bool IsLocallyControlled() const;
	UFUNCTION(BlueprintPure, Category="Helper")
	virtual bool IsLocallyViewed() const;
	UFUNCTION(BlueprintPure, Category="Helper")
	virtual bool IsPlayerControlled() const;

	static float PlayDual(const FDualAnimMontageData& Dual, USkeletalMeshComponent* TargetMesh, bool bFirstPerson, float PlayRate = 1.0f, FName StartSectionName = NAME_None);
	static void StopDual(const FDualAnimMontageData& Dual, USkeletalMeshComponent* TargetMesh, bool bFirstPerson, float BlendOutDuration = 0.1f);

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual float PlayEquipmentMontageSet(const FEquipmentAnimMontageData& Data, bool bFirstPerson, bool bThirdPerson, float PlayRate = 1.0f, FName StartSectionName = NAME_None) const;
	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void StopEquipmentMontageSet(const FEquipmentAnimMontageData& Data, bool bFirstPerson, bool bThirdPerson, float BlendOutDuration = 0.1f) const;
	
	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual float BeginEquip(bool bFirstPerson, bool bThirdPerson);

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void EndEquip();

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual float BeginUnequip(bool bFirstPerson, bool bThirdPerson);

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void EndUnequip();

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void ForceEquippedVisible(bool bFirstPerson);
	
	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void CancelUnequip();

protected:
	virtual void AttachToHolders();
	virtual void DetachFromHolders();

	virtual void UpdateTickAndVisibility();

	UFUNCTION()
	virtual void OnRep_EquipState();

	UFUNCTION()
	virtual void OnRep_Equipped();

public:
	UFUNCTION(BlueprintNativeEvent, Category="Equipment|Input")
	void PrimaryAction(bool bPressed);

	UFUNCTION(BlueprintNativeEvent, Category="Equipment|Input")
	void SecondaryAction(bool bPressed);

	UFUNCTION(BlueprintCallable, Category="Equipment")
	virtual void SetOwningCharacter(ACharacter* NewOwner);

	virtual void OnBeforeRemoved(UGE_EquipmentManagerComponent* Manager, EGE_EquipmentRemovalReason Reason) {}

	virtual void UpdateViewMode(bool bFirstPerson);

public:
	UFUNCTION(BlueprintPure, Category="State")
	FORCEINLINE USkeletalMeshComponent* GetMeshFP() const { return MeshFP; }

	UFUNCTION(BlueprintPure, Category="State")
	FORCEINLINE USkeletalMeshComponent* GetMeshTP() const { return MeshTP; }

	UFUNCTION(BlueprintPure, Category="State")
	FORCEINLINE bool IsEquipped() const { return bEquipped; }

};
