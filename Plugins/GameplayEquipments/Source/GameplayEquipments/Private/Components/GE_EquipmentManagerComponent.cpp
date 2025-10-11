#include "Components/GE_EquipmentManagerComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

#include "Equipments/GE_Equipment.h"

UGE_EquipmentManagerComponent::UGE_EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UGE_EquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGE_EquipmentManagerComponent, Slots);
	DOREPLIFETIME(UGE_EquipmentManagerComponent, CurrentIndex);
}

void UGE_EquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UGE_EquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGE_EquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FlowTimer);
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UGE_EquipmentManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

int32 UGE_EquipmentManagerComponent::AddEquipment(AGE_Equipment* Equipment, int32 PreferredSlot /*= -1*/)
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return INDEX_NONE;

	EnsureCapacity();

	if (!IsValid(Equipment)) { return INDEX_NONE; }

	int32 Slot = PreferredSlot;
	if (Slot == INDEX_NONE || !Slots.IsValidIndex(Slot) || Slots[Slot] != nullptr)
	{
		for (int32 i = 0; i < MaxSlots; ++i) { if (!Slots[i]) { Slot = i; break; } }
	}
	if (Slot == INDEX_NONE) { return INDEX_NONE; }

	Slots[Slot] = Equipment;

	Equipment->SetOwningCharacter(GetCharacter());
	OnEquipmentAdded.Broadcast(Equipment, Slot);
	return Slot;
}

bool UGE_EquipmentManagerComponent::RemoveEquipmentAt(int32 SlotIndex, EGE_EquipmentRemovalReason Reason /*= EGE_EquipmentRemovalReason::GenericRemove*/)
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return false;

	EnsureCapacity();

	if (!Slots.IsValidIndex(SlotIndex)) return false;

	AGE_Equipment* Eq = Slots[SlotIndex];
	if (!Eq) return false;

	if (SlotIndex == CurrentIndex)
	{
		FinishUnequipOnRole(Eq);
		CurrentIndex = INDEX_NONE;
	}

	Eq->OnBeforeRemoved(this, Reason);

	Slots[SlotIndex] = nullptr;

	OnEquipmentRemoved.Broadcast(Eq, SlotIndex);

	if (bDestroyOnRemove)
	{
		Eq->SetLifeSpan(0.01f);
	}

	return true;
}

bool UGE_EquipmentManagerComponent::RemoveEquipment(AGE_Equipment* Equipment, EGE_EquipmentRemovalReason Reason /*= EGE_EquipmentRemovalReason::GenericRemove*/)
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return false;

	if (!Equipment) return false;

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i] == Equipment)
		{
			return RemoveEquipmentAt(i, Reason);
		}
	}
	return false;
}

bool UGE_EquipmentManagerComponent::RemoveEquipments(EGE_EquipmentRemovalReason Reason /*= EGE_EquipmentRemovalReason::GenericRemove*/)
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return false;

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i])
		{
			RemoveEquipmentAt(i, Reason);
		}
	}

	return true;
}

void UGE_EquipmentManagerComponent::Cycle(int32 Delta)
{
	if (Delta == 0) return;

	if (GetOwner()->HasAuthority())
	{
		const int32 To = FindNextIndex(Delta);

		if (To != INDEX_NONE)
		{
			BeginServerSwap(To);
		}
	}
	else
	{
		ServerCycle(Delta);
	}
}

void UGE_EquipmentManagerComponent::EquipSlot(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex]) return;

	if (GetOwner()->HasAuthority())
	{
		BeginServerSwap(SlotIndex);
	}
	else
	{
		ServerEquipSlot(SlotIndex);
	}
}

void UGE_EquipmentManagerComponent::UnequipCurrent()
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return;

	if (CurrentIndex == INDEX_NONE) return;

	PendingIndex = INDEX_NONE;
	BeginServerSwap(INDEX_NONE);
}

ACharacter* UGE_EquipmentManagerComponent::GetCharacter() const
{
	return GetOwner<ACharacter>();
}

bool UGE_EquipmentManagerComponent::UseFirstPersonLocally() const
{
	const APawn* P = GetOwner<APawn>();
	return P && P->IsLocallyControlled();
}

void UGE_EquipmentManagerComponent::EnsureCapacity()
{
	if (MaxSlots <= 0) MaxSlots = 1;
	if (Slots.Num() < MaxSlots) Slots.SetNumZeroed(MaxSlots);
	if (CurrentIndex >= MaxSlots)
	{
		CurrentIndex = INDEX_NONE;
	}
}

int32 UGE_EquipmentManagerComponent::FindNextIndex(int32 Delta) const
{
	if (Slots.Num() == 0 || Delta == 0) return INDEX_NONE;

	const int32 Dir = (Delta > 0) ? +1 : -1;
	const int32 Steps = FMath::Abs(Delta);

	int32 idx = CurrentIndex;
	for (int s = 0; s < Steps; ++s)
	{
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			idx = (idx + Dir + MaxSlots) % MaxSlots;
			if (Slots[idx]) return idx;
		}
	}

	return INDEX_NONE;
}

void UGE_EquipmentManagerComponent::BeginServerSwap(int32 ToIndex)
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return;

	EnsureCapacity();

	AGE_Equipment* Current = GetEquipmentAt(CurrentIndex);

	// Pressing the same slot during Idle or Equipping -> ignore (rule)
	if (ToIndex == CurrentIndex && (FlowPhase == EGE_EquipFlowPhase::Idle || FlowPhase == EGE_EquipFlowPhase::Equipping))
	{
		PendingIndex = INDEX_NONE;
		return;
	}

	// Any input while Unequipping -> CANCEL and RETURN (do not restart swap)
	if (FlowPhase == EGE_EquipFlowPhase::Unequipping)
	{
		GetWorld()->GetTimerManager().ClearTimer(FlowTimer);

		if (Current) { Current->CancelUnequip(); }
		MulticastCancelUnequip(CurrentIndex);

		FlowPhase = EGE_EquipFlowPhase::Idle;
		PendingIndex = INDEX_NONE;
		return;
	}

	// Refresh pointers (may have changed due to prior transitions)
	Current = GetEquipmentAt(CurrentIndex);
	AGE_Equipment* Next = GetEquipmentAt(ToIndex);

	// If something is equipped, start unequip phase
	if (Current)
	{
		FlowPhase = EGE_EquipFlowPhase::Unequipping;
		PendingIndex = ToIndex;

		MulticastBeginUnequip(CurrentIndex);

		const float UnequipT = FMath::Max(0.f, StartUnequipOnRole(Current, /*bLocalFP=*/false));
		if (UnequipT > UE_KINDA_SMALL_NUMBER)
		{
			GetWorld()->GetTimerManager().SetTimer(FlowTimer, this, &UGE_EquipmentManagerComponent::Server_UnequipFinished, UnequipT, false);
		}
		else
		{
			Server_UnequipFinished();
		}

		return;
	}

	// Nothing equipped -> go straight to equip
	FlowPhase = EGE_EquipFlowPhase::Equipping;
	PendingIndex = INDEX_NONE;

	MulticastBeginEquip(ToIndex);

	CurrentIndex = ToIndex;

	const float EquipT = FMath::Max(0.f, StartEquipOnRole(Next, /*bLocalFP=*/false));
	if (EquipT > UE_KINDA_SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(FlowTimer, this, &UGE_EquipmentManagerComponent::Server_EquipFinished, EquipT, false);
	}
	else
	{
		Server_EquipFinished();
	}
}

void UGE_EquipmentManagerComponent::Server_UnequipFinished()
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return;

	if (AGE_Equipment* Old = GetEquipmentAt(CurrentIndex)) { FinishUnequipOnRole(Old); }

	const int32 ToIndex = PendingIndex;
	PendingIndex = INDEX_NONE;

	AGE_Equipment* Next = GetEquipmentAt(ToIndex);
	if (!Next)
	{
		CurrentIndex = INDEX_NONE;
		FlowPhase = EGE_EquipFlowPhase::Idle;
		return;
	}

	FlowPhase = EGE_EquipFlowPhase::Equipping;
	MulticastBeginEquip(ToIndex);

	CurrentIndex = ToIndex;

	const float EquipT = FMath::Max(0.f, StartEquipOnRole(Next, /*bLocalFP=*/false));
	if (EquipT > UE_KINDA_SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(FlowTimer, this, &UGE_EquipmentManagerComponent::Server_EquipFinished, EquipT, false);
	}
	else
	{
		Server_EquipFinished();
	}
}

void UGE_EquipmentManagerComponent::Server_EquipFinished()
{
	if (!ensure(GetOwner() && GetOwner()->HasAuthority())) return;

	if (AGE_Equipment* NewEq = GetEquipmentAt(CurrentIndex)) { FinishEquipOnRole(NewEq); }
	FlowPhase = EGE_EquipFlowPhase::Idle;
}

void UGE_EquipmentManagerComponent::Local_BeginUnequip(int32 FromIndex, int32 ChainToIndex /*= INDEX_NONE*/)
{
	if (!Slots.IsValidIndex(FromIndex)) return;

	if (AGE_Equipment* Eq = Slots[FromIndex])
	{
		const bool bFP = UseFirstPersonLocally();
		const float T = FMath::Max(0.f, StartUnequipOnRole(Eq, bFP));
		FlowPhase = EGE_EquipFlowPhase::Unequipping;

		Local_ClearTimer();
		if (T <= UE_KINDA_SMALL_NUMBER)
		{
			FinishUnequipOnRole(Eq);
			FlowPhase = EGE_EquipFlowPhase::Idle;
			if (ChainToIndex != INDEX_NONE) Local_BeginEquip(ChainToIndex);
			return;
		}

		GetWorld()->GetTimerManager().SetTimer(FlowTimer, [this, Eq, ChainToIndex]()
		{
			FinishUnequipOnRole(Eq);
			FlowPhase = EGE_EquipFlowPhase::Idle;
			if (ChainToIndex != INDEX_NONE) Local_BeginEquip(ChainToIndex);
		}, T, false);
	}
}

void UGE_EquipmentManagerComponent::Local_BeginEquip(int32 ToIndex)
{
	if (!Slots.IsValidIndex(ToIndex)) return;
	if (AGE_Equipment* Eq = Slots[ToIndex])
	{
		const bool bFP = UseFirstPersonLocally();
		const float T = FMath::Max(0.f, StartEquipOnRole(Eq, bFP));
		FlowPhase = EGE_EquipFlowPhase::Equipping;

		Local_ClearTimer();
		if (T <= UE_KINDA_SMALL_NUMBER)
		{
			FinishEquipOnRole(Eq);
			FlowPhase = EGE_EquipFlowPhase::Idle;
			return;
		}

		GetWorld()->GetTimerManager().SetTimer(FlowTimer, [this, Eq]()
		{
			FinishEquipOnRole(Eq);
			FlowPhase = EGE_EquipFlowPhase::Idle;
		}, T, false);
	}
}

void UGE_EquipmentManagerComponent::Local_ClearTimer()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FlowTimer);
	}
}

void UGE_EquipmentManagerComponent::Local_CancelUnequip(int32 Index)
{
	if (!Slots.IsValidIndex(Index)) return;

	if (AGE_Equipment* Eq = Slots[Index])
	{
		Local_ClearTimer();
		Eq->CancelUnequip();
		FlowPhase = EGE_EquipFlowPhase::Idle;
	}
}

float UGE_EquipmentManagerComponent::StartUnequipOnRole(AGE_Equipment* Eq, bool bLocalFP) const
{
	if (!Eq) return 0.f;

	if (bLocalFP)
	{
		return Eq->BeginUnequip(true, true);
	}

	return Eq->BeginUnequip(false, true);
}

float UGE_EquipmentManagerComponent::StartEquipOnRole(AGE_Equipment* Eq, bool bLocalFP) const
{
	if (!Eq) return 0.f;

	if (bLocalFP)
	{
		return Eq->BeginEquip(true, true);
	}

	return Eq->BeginEquip(false, true);
}

void UGE_EquipmentManagerComponent::FinishUnequipOnRole(AGE_Equipment* Eq) const
{
	if (Eq) Eq->EndUnequip();
}

void UGE_EquipmentManagerComponent::FinishEquipOnRole(AGE_Equipment* Eq) const
{
	if (Eq) Eq->EndEquip();
}

void UGE_EquipmentManagerComponent::BroadcastChanged(AGE_Equipment* NewEq, AGE_Equipment* OldEq)
{
	OnEquipmentChanged.Broadcast(NewEq, OldEq);
}

void UGE_EquipmentManagerComponent::OnRep_Slots()
{
	if (ACharacter* C = GetCharacter())
	{
		for (AGE_Equipment* Eq : Slots)
		{
			if (Eq)
			{
				Eq->SetOwningCharacter(C);
			}
		}
	}
}

void UGE_EquipmentManagerComponent::OnRep_CurrentIndex(int32 OldIndex)
{
	BroadcastChanged(GetEquipmentAt(CurrentIndex), GetEquipmentAt(OldIndex));
}

void UGE_EquipmentManagerComponent::ServerEquipSlot_Implementation(int32 SlotIndex)
{
	BeginServerSwap(SlotIndex);
}

void UGE_EquipmentManagerComponent::ServerCycle_Implementation(int32 Delta)
{
	const int32 To = FindNextIndex(Delta);

	if (To != INDEX_NONE)
	{
		BeginServerSwap(To);
	}
}

void UGE_EquipmentManagerComponent::MulticastBeginUnequip_Implementation(int32 FromIndex)
{
	Local_BeginUnequip(FromIndex, INDEX_NONE);
}

void UGE_EquipmentManagerComponent::MulticastBeginEquip_Implementation(int32 ToIndex)
{
	const int32 Old = CurrentIndex;
	CurrentIndex = ToIndex;

	if (Old != ToIndex)
	{
		BroadcastChanged(GetEquipmentAt(ToIndex), GetEquipmentAt(Old));
	}

	Local_BeginEquip(ToIndex);
}

void UGE_EquipmentManagerComponent::MulticastCancelUnequip_Implementation(int32 Index)
{
	Local_CancelUnequip(Index);
}
