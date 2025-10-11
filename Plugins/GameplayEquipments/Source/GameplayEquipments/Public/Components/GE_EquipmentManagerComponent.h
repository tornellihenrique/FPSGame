#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/GE_Types.h"
#include "GE_EquipmentManagerComponent.generated.h"

class AGE_Equipment;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentChanged, AGE_Equipment*, NewEquipment, AGE_Equipment*, OldEquipment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentAdded, AGE_Equipment*, Equipment, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentRemoved, AGE_Equipment*, Equipment, int32, SlotIndex);

UENUM(BlueprintType)
enum class EGE_EquipFlowPhase : uint8
{
	Idle,
	Unequipping,
	Equipping
};

UCLASS(ClassGroup=(GameplayEquipments), meta=(BlueprintSpawnableComponent, DisplayName="Equipment Manager"))
class GAMEPLAYEQUIPMENTS_API UGE_EquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Equipment Manager")
	int32 MaxSlots = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Equipment Manager")
	uint8 bDestroyOnRemove : 1;

public:
	UGE_EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer);

	//~ UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End of UActorComponent

public:
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnEquipmentAdded OnEquipmentAdded;

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnEquipmentRemoved OnEquipmentRemoved;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<TObjectPtr<AGE_Equipment>> Slots;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentIndex)
	int32 CurrentIndex = INDEX_NONE;

	EGE_EquipFlowPhase FlowPhase = EGE_EquipFlowPhase::Idle;

	int32 PendingIndex = INDEX_NONE;

	FTimerHandle FlowTimer;

public:
	/** Add an already spawned equipment to the bar. Authority only. Returns slot index or INDEX_NONE. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment")
	int32 AddEquipment(AGE_Equipment* Equipment, int32 PreferredSlot = -1);

	/** Remove equipment by slot. Authority only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment")
	bool RemoveEquipmentAt(int32 SlotIndex, EGE_EquipmentRemovalReason Reason = EGE_EquipmentRemovalReason::GenericRemove);

	/** Remove equipment by instance. Authority only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment")
	bool RemoveEquipment(AGE_Equipment* Equipment, EGE_EquipmentRemovalReason Reason = EGE_EquipmentRemovalReason::GenericRemove);

	/** Remove all equipments. Authority only. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment")
	bool RemoveEquipments(EGE_EquipmentRemovalReason Reason = EGE_EquipmentRemovalReason::GenericRemove);

	/** Cycle delta (+1 next, -1 previous). */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	void Cycle(int32 Delta);

	/** Equip slot by index. */
	UFUNCTION(BlueprintCallable, Category="Equipment")
	void EquipSlot(int32 SlotIndex);

	/** Unequip current (keeps selection, mainly for UI/testing). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Equipment")
	void UnequipCurrent();

protected:
	ACharacter* GetCharacter() const;

	bool UseFirstPersonLocally() const;

	void EnsureCapacity();
	int32 FindNextIndex(int32 Delta) const;

	// Authoritative flow (server)
	void BeginServerSwap(int32 ToIndex);
	void Server_UnequipFinished();
	void Server_EquipFinished();

	// Cosmetic flow (all clients)
	void Local_BeginUnequip(int32 FromIndex, int32 ChainToIndex = INDEX_NONE);
	void Local_BeginEquip(int32 ToIndex);
	void Local_ClearTimer();

	void Local_CancelUnequip(int32 Index);

	float StartUnequipOnRole(AGE_Equipment* Eq, bool bLocalFP) const;
	float StartEquipOnRole(AGE_Equipment* Eq, bool bLocalFP) const;
	void FinishUnequipOnRole(AGE_Equipment* Eq) const;
	void FinishEquipOnRole(AGE_Equipment* Eq) const;

	void BroadcastChanged(AGE_Equipment* NewEq, AGE_Equipment* OldEq);

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_CurrentIndex(int32 OldIndex);

protected:
	UFUNCTION(Server, Reliable)
	void ServerEquipSlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void ServerCycle(int32 Delta);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginUnequip(int32 FromIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginEquip(int32 ToIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCancelUnequip(int32 Index);

public:
	UFUNCTION(BlueprintPure, Category="Equipment")
	FORCEINLINE AGE_Equipment* GetEquipmentAt(int32 SlotIndex) const { return Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : nullptr; }

	UFUNCTION(BlueprintPure, Category="Equipment")
	FORCEINLINE AGE_Equipment* GetCurrentEquipment() const { return GetEquipmentAt(CurrentIndex); }

	UFUNCTION(BlueprintPure, Category="Equipment")
	FORCEINLINE int32 GetCurrentIndex() const { return CurrentIndex; }

};
