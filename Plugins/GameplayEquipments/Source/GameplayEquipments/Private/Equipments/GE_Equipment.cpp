#include "Equipments/GE_Equipment.h"

#include "Net/UnrealNetwork.h"

#include "Misc/GE_EquipmentAnimData.h"
#include "Interfaces/GE_CharacterInterface.h"
#include "GameFramework/Character.h"

AGE_Equipment::AGE_Equipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFP"));
	MeshFP->SetupAttachment(RootComponent);
	MeshFP->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshFP->SetOnlyOwnerSee(true);
	MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

	MeshTP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshTP"));
	MeshTP->SetupAttachment(RootComponent);
	MeshTP->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshTP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	MeshTP->SetOwnerNoSee(true);

	bOwnerIsFirstPerson = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

#if WITH_EDITOR
void AGE_Equipment::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AGE_Equipment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AGE_Equipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGE_Equipment, EquipState);
	DOREPLIFETIME(AGE_Equipment, bEquipped);
}

void AGE_Equipment::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGE_Equipment::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGE_Equipment::BeginPlay()
{
	Super::BeginPlay();

	UpdateTickAndVisibility();
}

void AGE_Equipment::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGE_Equipment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGE_Equipment::OnRep_Owner()
{
	Super::OnRep_Owner();

	ACharacter* NewOwner = GetOwner<ACharacter>();
	OwningCharacter = NewOwner;
	OwningCharacterInterface = NewOwner;

	if (IsValid(NewOwner)) { AttachToHolders(); }
	else { DetachFromHolders(); }

	UpdateTickAndVisibility();
}

void AGE_Equipment::OnRep_Instigator()
{
	Super::OnRep_Instigator();
}

void AGE_Equipment::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);
}

bool AGE_Equipment::IsLocallyControlled() const
{
	if (OwningCharacter.IsValid()) return OwningCharacter->IsLocallyControlled();

	return GetInstigator() ? GetInstigator()->IsLocallyControlled() : false;
}

bool AGE_Equipment::IsLocallyViewed() const
{
	if (OwningCharacter.IsValid()) return OwningCharacter->IsLocallyViewed();

	return GetInstigator() ? GetInstigator()->IsLocallyViewed() : false;
}

bool AGE_Equipment::IsPlayerControlled() const
{
	if (OwningCharacter.IsValid()) return OwningCharacter->IsPlayerControlled();

	return GetInstigator() ? GetInstigator()->IsPlayerControlled() : false;
}

float AGE_Equipment::PlayDual(const FDualAnimMontageData& Dual, USkeletalMeshComponent* TargetMesh, bool bFirstPerson, float PlayRate /*= 1.0f*/, FName StartSectionName /*= NAME_None*/)
{
	float OutDuration = 0.f;

	if (UAnimMontage* M = bFirstPerson ? Dual.MontageFP : Dual.MontageTP)
	{
		if (UAnimInstance* AI = TargetMesh ? TargetMesh->GetAnimInstance() : nullptr)
		{
			OutDuration = AI->Montage_Play(M, PlayRate);

			if (OutDuration > 0.f && StartSectionName != NAME_None)
			{
				AI->Montage_JumpToSection(StartSectionName, M);
			}
		}
	}

	return OutDuration;
}

void AGE_Equipment::StopDual(const FDualAnimMontageData& Dual, USkeletalMeshComponent* TargetMesh, bool bFirstPerson, float BlendOutDuration /*= 0.1f*/)
{
	if (UAnimMontage* M = bFirstPerson ? Dual.MontageFP : Dual.MontageTP)
	{
		if (UAnimInstance* I = TargetMesh ? TargetMesh->GetAnimInstance() : nullptr)
		{
			I->Montage_Stop(BlendOutDuration, M);
		}
	}
}

float AGE_Equipment::PlayEquipmentMontageSet(const FEquipmentAnimMontageData& Data, bool bFirstPerson, bool bThirdPerson, float PlayRate /*= 1.0f*/, FName StartSectionName /*= NAME_None*/) const
{
	if (!OwningCharacterInterface || !OwningCharacter.IsValid()) return 0.f;

	float Duration = 0.0f;

	if (bFirstPerson)
	{
		USkeletalMeshComponent* CharMesh = OwningCharacterInterface->GetMeshFP();
		USkeletalMeshComponent* EqMesh = MeshFP.Get();

		const float CharDur = PlayDual(Data.CharacterAnimData, CharMesh, true, PlayRate, StartSectionName);
		const float EqDur = PlayDual(Data.EquipmentAnimData, EqMesh, true, PlayRate, StartSectionName);

		Duration = FMath::Max(CharDur, EqDur);
	}

	if (bThirdPerson)
	{
		USkeletalMeshComponent* CharMesh = OwningCharacter->GetMesh();
		USkeletalMeshComponent* EqMesh = MeshTP.Get();

		const float CharDur = PlayDual(Data.CharacterAnimData, CharMesh, false, PlayRate, StartSectionName);
		const float EqDur = PlayDual(Data.EquipmentAnimData, EqMesh, false, PlayRate, StartSectionName);

		Duration = FMath::Max(CharDur, EqDur);
	}

	return Duration;
}

void AGE_Equipment::StopEquipmentMontageSet(const FEquipmentAnimMontageData& Data, bool bFirstPerson, bool bThirdPerson, float BlendOutDuration /*= 0.1f*/) const
{
	if (!OwningCharacterInterface || !OwningCharacter.IsValid()) return;

	if (bFirstPerson)
	{
		USkeletalMeshComponent* CharMesh = OwningCharacterInterface->GetMeshFP();
		USkeletalMeshComponent* EqMesh = MeshFP.Get();

		StopDual(Data.CharacterAnimData, CharMesh, true, BlendOutDuration);
		StopDual(Data.EquipmentAnimData, EqMesh, true, BlendOutDuration);
	}

	if (bThirdPerson)
	{
		USkeletalMeshComponent* CharMesh = OwningCharacter->GetMesh();
		USkeletalMeshComponent* EqMesh = MeshTP.Get();

		StopDual(Data.CharacterAnimData, CharMesh, false, BlendOutDuration);
		StopDual(Data.EquipmentAnimData, EqMesh, false, BlendOutDuration);
	}
}

float AGE_Equipment::BeginEquip(bool bFirstPerson, bool bThirdPerson)
{
	EquipState = EGE_EquipmentState::Equipping;
	OnRep_EquipState();

	bEquipped = false;
	OnRep_Equipped();

	return AnimData ? PlayEquipmentMontageSet(AnimData->EquipAnim, bFirstPerson, bThirdPerson) : 0.f;
}

void AGE_Equipment::EndEquip()
{
	EquipState = EGE_EquipmentState::Equipped;
	OnRep_EquipState();

	bEquipped = true;
	OnRep_Equipped();
}

float AGE_Equipment::BeginUnequip(bool bFirstPerson, bool bThirdPerson)
{
	EquipState = EGE_EquipmentState::Unequipping;
	OnRep_EquipState();

	return AnimData ? PlayEquipmentMontageSet(AnimData->UnEquipAnim, bFirstPerson, bThirdPerson) : 0.f;
}

void AGE_Equipment::EndUnequip()
{
	EquipState = EGE_EquipmentState::Idle;
	OnRep_EquipState();

	bEquipped = false;
	OnRep_Equipped();

	if (AnimData)
	{
		StopEquipmentMontageSet(AnimData->UnEquipAnim, true, true);
	}
}

void AGE_Equipment::ForceEquippedVisible(bool bFirstPerson)
{
	EquipState = EGE_EquipmentState::Equipped;
	OnRep_EquipState();

	bEquipped = true;
	OnRep_Equipped();
}

void AGE_Equipment::CancelUnequip()
{
	if (EquipState != EGE_EquipmentState::Unequipping) { return; }

	if (AnimData)
	{
		StopEquipmentMontageSet(AnimData->UnEquipAnim, true, true, 0.35f);
	}

	EquipState = EGE_EquipmentState::Equipped;
	OnRep_EquipState();

	bEquipped = true;
	OnRep_Equipped();
}

void AGE_Equipment::AttachToHolders()
{
	if (GetOwner())
	{
		AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	if (!OwningCharacterInterface) return;

	if (USceneComponent* FPHolder = OwningCharacterInterface->GetEquipmentHolderFP())
	{
		MeshFP->AttachToComponent(FPHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MeshFP->SetRelativeTransform_Direct(AnimData ? AnimData->AttachOffsetTransform : FTransform::Identity);
	}
	if (USceneComponent* TPHolder = OwningCharacterInterface->GetEquipmentHolderTP())
	{
		MeshTP->AttachToComponent(TPHolder, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MeshTP->SetRelativeTransform_Direct(AnimData ? AnimData->AttachOffsetTransform : FTransform::Identity);
	}
}

void AGE_Equipment::DetachFromHolders()
{
	MeshFP->SetRelativeTransform_Direct(FTransform::Identity);
	MeshTP->SetRelativeTransform_Direct(FTransform::Identity);
	MeshFP->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	MeshTP->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	MeshFP->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	MeshTP->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void AGE_Equipment::UpdateTickAndVisibility()
{
	const bool bActive = (EquipState != EGE_EquipmentState::Idle);
	SetActorTickEnabled(bActive);
	SetActorHiddenInGame(!bActive);
}

void AGE_Equipment::OnRep_EquipState()
{
	UpdateTickAndVisibility();
}

void AGE_Equipment::OnRep_Equipped()
{
	UpdateTickAndVisibility();
}

void AGE_Equipment::PrimaryAction_Implementation(bool bPressed) {}

void AGE_Equipment::SecondaryAction_Implementation(bool bPressed) {}

void AGE_Equipment::SetOwningCharacter(ACharacter* NewOwner)
{
	OwningCharacter = NewOwner;
	OwningCharacterInterface = NewOwner;

	SetOwner(NewOwner);
	SetInstigator(NewOwner);

	if (IsValid(NewOwner)) { AttachToHolders(); }
	else { DetachFromHolders(); }
}

void AGE_Equipment::UpdateViewMode(bool bFirstPerson)
{
	bOwnerIsFirstPerson = bFirstPerson;

	if (bFirstPerson)
	{
		MeshTP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
		MeshTP->SetOwnerNoSee(true);

		MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
		MeshFP->SetVisibility(true, true);
	}
	else
	{
		MeshTP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;
		MeshTP->SetOwnerNoSee(false);

		MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
		MeshFP->SetVisibility(false, true);
	}
}

FVector AGE_Equipment::GetPivotPoint() const
{
	if (IsValid(MeshFP))
	{
		FVector P = MeshFP->GetSocketTransform(TEXT("PivotPoint"), RTS_Component).GetLocation();
		P.Z *= -1.f;
		return P;
	}

	return FVector::ZeroVector;
}
