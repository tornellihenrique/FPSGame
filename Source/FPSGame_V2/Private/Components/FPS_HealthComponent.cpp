// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/FPS_HealthComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Game/FPS_GameMode.h"

UFPS_HealthComponent::UFPS_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}

void UFPS_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(Health, 0.0f, MaxHealth);
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UFPS_HealthComponent::HandleAnyDamage);
		Owner->OnTakePointDamage.AddDynamic(this, &UFPS_HealthComponent::HandlePointDamage);
		Owner->OnTakeRadialDamage.AddDynamic(this, &UFPS_HealthComponent::HandleRadialDamage);
	}

	OnHealthChanged.Broadcast(Health);

	StartDamageableDelayIfNeeded();
}

void UFPS_HealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Damageable);

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.RemoveDynamic(this, &UFPS_HealthComponent::HandleAnyDamage);
		Owner->OnTakePointDamage.RemoveDynamic(this, &UFPS_HealthComponent::HandlePointDamage);
		Owner->OnTakeRadialDamage.RemoveDynamic(this, &UFPS_HealthComponent::HandleRadialDamage);
	}

	Super::EndPlay(EndPlayReason);
}

void UFPS_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFPS_HealthComponent, Health);
}

void UFPS_HealthComponent::Heal(float Amount)
{
	if (!GetOwner() || GetOwnerRole() < ROLE_Authority) return;
	if (Amount <= 0.f || !IsAlive()) return;

	const float Old = Health;
	SetHealth(Health + Amount);

	if (Health > Old && DamageInfoArray.Num() > 0)
	{
		const float Healed = FMath::Min(Amount, MaxHealth - Old);
		const float Fraction = Healed / FMath::Max(1.f, MaxHealth);
		for (int32 i = DamageInfoArray.Num() - 1; i >= 0; --i)
		{
			FDamageInfo& Info = DamageInfoArray[i];
			Info.DamageAmount -= Info.DamageAmount * Fraction;
			if (Info.DamageAmount <= KINDA_SMALL_NUMBER)
			{
				DamageInfoArray.RemoveAtSwap(i);
			}
		}
	}
}

void UFPS_HealthComponent::Suicide()
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority) return;

	TSubclassOf<UDamageType> DT = SuicideDamageType ? SuicideDamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass());
	ApplyDamageOfType(DT, MaxHealth + 1.f, GetOwner()->GetInstigatorController(), GetOwner());
}

void UFPS_HealthComponent::InstaKill(TSubclassOf<UDamageType> DamageType, AController* InInstigator, AActor* Causer)
{
	ApplyDamageOfType(DamageType, MaxHealth + 1.f, InInstigator, Causer);
}

void UFPS_HealthComponent::ApplyDamageOfType(TSubclassOf<UDamageType> DamageType, float Amount, AController* InInstigator, AActor* Causer)
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority) return;
	if (!GetOwner()->CanBeDamaged()) return;

	FDamageEvent Event(DamageType);
	ApplyDamageEvent(Amount, Event, InInstigator, Causer);
}

void UFPS_HealthComponent::ApplyFallDamage(float Amount)
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority) return;
	if (Amount <= 0.f) return;

	TSubclassOf<UDamageType> DT = FallingDamageType ? FallingDamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass());
	ApplyDamageOfType(DT, Amount, GetOwner()->GetInstigatorController(), GetOwner());
}

float UFPS_HealthComponent::ApplyDamageEvent(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!GetOwner() || GetOwner()->GetLocalRole() < ROLE_Authority) return 0.f;
	if (!GetOwner()->CanBeDamaged() || DamageAmount <= 0.f || !DamageCauser) return 0.f;
	if (!IsAlive()) return 0.f;

	FHitResult Hit(ForceInit);
	FVector ImpulseDir = GetOwner()->GetActorForwardVector();
	bool bRadial = false;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* Evt = static_cast<const FPointDamageEvent*>(&DamageEvent);
		Hit = Evt->HitInfo;
		const FVector SrcLoc = DamageCauser->GetInstigator()
			? DamageCauser->GetInstigator()->GetActorLocation()
			: DamageCauser->GetActorLocation();
		ImpulseDir = (Hit.ImpactPoint - SrcLoc).GetSafeNormal();
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent* Evt = static_cast<const FRadialDamageEvent*>(&DamageEvent);
		FHitResult Tmp;
		Evt->GetBestHitInfo(DamageCauser, EventInstigator, Tmp, ImpulseDir);
		Hit = Tmp;
		bRadial = true;
	}

	DamageAmount *= GetPerBoneMultiplier(Hit.BoneName);

	DamageAmount = AdjustDamage(DamageAmount, EventInstigator);

	const bool bHeadshot = Hit.BoneName.ToString().ToLower().Equals(TEXT("head"));

	if (DamageAmount > 0.f && EventInstigator)
	{
		bool bFound = false;
		for (FDamageInfo& Info : DamageInfoArray)
		{
			if (Info.Instigator == EventInstigator)
			{
				Info.DamageTypeClass = DamageEvent.DamageTypeClass;
				Info.DamageCauser = DamageCauser;
				Info.DamageAmount += DamageAmount;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			DamageInfoArray.Add(FDamageInfo(EventInstigator, DamageEvent.DamageTypeClass, DamageCauser, DamageAmount, bHeadshot));
		}
	}

	const float NewHealth = Health - DamageAmount; // Final health value (don't change afterwards)

	if (NewHealth <= 0.0f)
	{
		GetOwner()->SetCanBeDamaged(false);
		SetHealth(0.0f);

		const float Scale = bRadial ? ExplosionImpulseScale : BulletImpulseScale;
		FVector Impulse = ImpulseDir;
		if (!Impulse.IsNearlyZero())
		{
			Impulse.Normalize();
			if (bRadial) { Impulse.Z = FMath::Max(Impulse.Z, 0.5f); }
			Impulse *= Scale;

			const FName Bone = Hit.BoneName.IsNone() ? FName(TEXT("pelvis")) : Hit.BoneName;
			const FVector At = Hit.Location.IsNearlyZero() ? GetOwner()->GetActorLocation() : Hit.Location;

			const FDeathEventPayload Payload(
				EventInstigator,
				DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass : TSubclassOf<UDamageType>(UDamageType::StaticClass()),
				DamageCauser,
				bRadial,
				Hit, Impulse, At, Bone,
				DamageAmount, bHeadshot);

			BroadcastDeathOnce(Payload);
		}
	}
	else
	{
		SetHealth(NewHealth);

		if (DamageAmount > 0.f)
		{
			BroadcastTookDamage(Hit, DamageAmount, DamageCauser);
		}
	}

	return DamageAmount;
}

float UFPS_HealthComponent::AdjustDamage_Implementation(float InDamage, AController* InstigatorController)
{
	float Dmg = InDamage;

	if (AActor* Owner = GetOwner())
	{
		if (AFPS_GameMode* GM = Owner->GetWorld()->GetAuthGameMode<AFPS_GameMode>())
		{
			GM->CalcDamage(Dmg, Owner->GetInstigatorController(), InstigatorController);
		}
	}

	return Dmg;
}

void UFPS_HealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!DamagedActor || GetOwner() != DamagedActor) return;

	TSubclassOf<UDamageType> DamageTypeClass = DamageType ? DamageType->GetClass() : UDamageType::StaticClass();
	FDamageEvent Evt(DamageTypeClass);
	ApplyDamageEvent(Damage, Evt, InstigatedBy, DamageCauser);
}

void UFPS_HealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	if (!DamagedActor || GetOwner() != DamagedActor) return;

	FPointDamageEvent Evt;
	Evt.Damage = Damage;
	Evt.DamageTypeClass = DamageType ? DamageType->GetClass() : UDamageType::StaticClass();
	Evt.HitInfo = FHitResult(ForceInit);
	Evt.HitInfo.Location = HitLocation;
	Evt.HitInfo.BoneName = BoneName;
	ApplyDamageEvent(Damage, Evt, InstigatedBy, DamageCauser);
}

void UFPS_HealthComponent::HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!DamagedActor || GetOwner() != DamagedActor) return;

	FRadialDamageEvent Evt;
	Evt.DamageTypeClass = DamageType ? DamageType->GetClass() : UDamageType::StaticClass();
	Evt.Params = FRadialDamageParams();
	ApplyDamageEvent(Damage, Evt, InstigatedBy, DamageCauser);
}

void UFPS_HealthComponent::StartDamageableDelayIfNeeded()
{
	if (DamageableTimeAfterSpawn <= 0.f)
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->SetCanBeDamaged(true);
		}

		return;
	}

	if (AActor* Owner = GetOwner())
	{
		Owner->SetCanBeDamaged(false);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Damageable, FTimerDelegate::CreateWeakLambda(this, [this, Owner]()
		{
			if (IsValid(Owner))
			{
				Owner->SetCanBeDamaged(true);
			}
		}), DamageableTimeAfterSpawn, false);
	}
}

void UFPS_HealthComponent::SetHealth(float NewValue)
{
	const float Clamped = FMath::Clamp(NewValue, 0.f, MaxHealth);
	if (!FMath::IsNearlyEqual(Health, Clamped))
	{
		Health = Clamped;
		OnRep_Health();
	}
}

void UFPS_HealthComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health);

	if (Health <= 0.f && !bDeathBroadcasted)
	{
		OnDeath.Broadcast();
	}
}

float UFPS_HealthComponent::GetPerBoneMultiplier(const FName& BoneName) const
{
	if (BoneName.IsNone()) return 1.f;
	
	const AActor* Owner = GetOwner();
	if (!Owner) return 1.f;

	const USkeletalMeshComponent* Skel = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Skel || !Skel->GetSkinnedAsset()) return 1.f;

	const FReferenceSkeleton& RefSkeleton = Skel->GetSkinnedAsset()->GetRefSkeleton();

	float DamageFactor = 1.f;
	int32 MinDepth = MAX_int32;

	const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE) return 1.f;

	for (const FBoneDamage& Entry : PerBoneDamageMultiplier)
	{
		const int32 ParentIndex = RefSkeleton.FindBoneIndex(Entry.BoneName);
		if (ParentIndex == INDEX_NONE) continue;

		const int32 Depth = RefSkeleton.GetDepthBetweenBones(BoneIndex, ParentIndex);
		if (Depth == 0)
		{
			DamageFactor = Entry.DamageFactor;
			MinDepth = 0;
			break;
		}
		if (Depth > 0 && Depth < MinDepth)
		{
			DamageFactor = Entry.DamageFactor;
			MinDepth = Depth;
		}
	}

	return DamageFactor;
}

void UFPS_HealthComponent::BroadcastDeathOnce(const FDeathEventPayload& Payload)
{
	if (bDeathBroadcasted) return;
	bDeathBroadcasted = true;

	OnDeath.Broadcast();
	OnDeathPayload.Broadcast(Payload);
}

void UFPS_HealthComponent::BroadcastTookDamage(const FHitResult& Hit, float Amount, AActor* Causer)
{
	OnTookDamage.Broadcast(Hit, Amount, Causer);
}
