#include "Equipments/GE_FireWeapon.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Sound/SoundBase.h"
#include "PhysicsEngine/PhysicsSettings.h"

#include "Misc/GE_EquipmentAnimData.h"
#include "Interfaces/GE_CharacterInterface.h"

#define LOCTEXT_NAMESPACE "GE_FireWeapon"

static float GE_FireIntervalFromRPM(float RPM)
{
	return RPM <= 0.f ? 0.1f : 60.f / RPM;
}

AGE_FireWeapon::AGE_FireWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AmmoInMag = 0;
	TotalAmmo = InitialTotalAmmo;

	BurstLeft = 0;
	bBurstSequencing = false;
	bRunFireDelayActive = false;
	bWantsToFireInput = false;

	ReloadKeys = { EKeys::R };
	FireModeKeys = { EKeys::B };
	bBindedInputs = false;

	RepFireMode = FireMode;
}

#if WITH_EDITOR
void AGE_FireWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AGE_FireWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AGE_FireWeapon, FireMode))
	{
		RepFireMode = FireMode;
	}
}
#endif

void AGE_FireWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGE_FireWeapon, AmmoInMag);
	DOREPLIFETIME(AGE_FireWeapon, TotalAmmo);
	DOREPLIFETIME(AGE_FireWeapon, bIsReloading);
	DOREPLIFETIME(AGE_FireWeapon, RepFireMode);

	DOREPLIFETIME_CONDITION(AGE_FireWeapon, bIsFiring, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGE_FireWeapon, BulletData, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AGE_FireWeapon, bIsAiming, COND_SkipOwner);
}

void AGE_FireWeapon::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AGE_FireWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGE_FireWeapon::BeginPlay()
{
	Super::BeginPlay();

	Imprecision = MaxImprecision;
	BulletFired = 0;

	if (HasAuthority())
	{
		AmmoInMag = FMath::Clamp(MagazineCapacity, 0, MagazineCapacity);
		TotalAmmo = FMath::Max(0, InitialTotalAmmo);

		RepFireMode = FireMode;
	}

	if (ACharacter* C = Cast<ACharacter>(GetOwner()))
	{
		SetOwningCharacter(C);
	}

	RefreshADSOffset();
}

void AGE_FireWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Fire);
	GetWorldTimerManager().ClearTimer(TimerHandle_RunFireDelay);
	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (bBindedInputs && LastInputComponent)
	{
		LastInputComponent->ClearBindingsForObject(this);

		bBindedInputs = false;
	}

	Super::EndPlay(EndPlayReason);
}

void AGE_FireWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RefreshAimingTimeline(DeltaTime);
}

void AGE_FireWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if (ACharacter* C = Cast<ACharacter>(GetOwner()))
	{
		SetOwningCharacter(C);
	}
}

void AGE_FireWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
}

void AGE_FireWeapon::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);
}

void AGE_FireWeapon::SetOwningCharacter(ACharacter* NewOwner)
{
	Super::SetOwningCharacter(NewOwner);

	if (IsValid(NewOwner))
	{
		if (IsLocallyControlled() && !bBindedInputs)
		{
			if (APlayerController* PC = Cast<APlayerController>(NewOwner->GetController()))
			{
				if (UInputComponent* IC = PC->InputComponent)
				{
					LastInputComponent = IC;

					for (const FKey& K : ReloadKeys)
					{
						if (K.IsValid())
						{
							IC->BindKey(K, IE_Pressed, this, &AGE_FireWeapon::InputReload);
						}
					}
					for (const FKey& K : FireModeKeys)
					{
						if (K.IsValid())
						{
							IC->BindKey(K, IE_Pressed, this, &AGE_FireWeapon::InputCycleFireMode);
						}
					}

					bBindedInputs = true;
				}
			}
		}
	}
	else
	{
		if (bBindedInputs && LastInputComponent)
		{
			LastInputComponent->ClearBindingsForObject(this);

			bBindedInputs = false;
		}
	}
}

void AGE_FireWeapon::OnRep_Equipped()
{
	Super::OnRep_Equipped();
}

void AGE_FireWeapon::OnBeforeRemoved(UGE_EquipmentManagerComponent* Manager, EGE_EquipmentRemovalReason Reason)
{
	// #TODO
}

float AGE_FireWeapon::BeginEquip(bool bFirstPerson, bool bThirdPerson)
{
	return Super::BeginEquip(bFirstPerson, bThirdPerson);
}

float AGE_FireWeapon::BeginUnequip(bool bFirstPerson, bool bThirdPerson)
{
	EndFire();
	CancelReload();

	if (bRunFireDelayActive)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_RunFireDelay);
		bRunFireDelayActive = false;
		bWantsToFireInput = false;
	}

	return Super::BeginUnequip(bFirstPerson, bThirdPerson);
}

bool AGE_FireWeapon::CanRun() const
{
	if (IsAiming() || bIsFiring || bRunFireDelayActive)
	{
		return false;
	}

	return Super::CanRun();
}

void AGE_FireWeapon::PrimaryAction_Implementation(bool bPressed)
{
	if (bPressed)
	{
		bWantsToFireInput = true;
		BeginFire();
	}
	else
	{
		bWantsToFireInput = false;
		EndFire();
	}
}

void AGE_FireWeapon::SecondaryAction_Implementation(bool bPressed)
{
	SetIsAiming(bPressed);
}

void AGE_FireWeapon::StartReload()
{
	if (!CanReload()) return;

	if (HasAuthority())
	{
		if (!bIsReloading)
		{
			bIsReloading = true;
			OnRep_IsReloading();
		}
	}
	else
	{
		ServerRequestStartReload();
	}
}

void AGE_FireWeapon::Notify_InsertOneRound()
{
	if (!bLoopReload || !IsEquipped()) return;

	if (IsLocallyControlled())
	{
		ServerInsertOneRound();

		PlayEquipmentMontageSet(ReloadAnim, /*FP*/true, /*TP*/true, /*PlayRate*/1.0f, ReloadLoopSectionName);
	}
}

void AGE_FireWeapon::Notify_CommitReload(bool bForceFull /*= false*/)
{
	if (!IsEquipped()) return;

	if (IsLocallyControlled())
	{
		ServerCommitReload(bForceFull);
	}
}

void AGE_FireWeapon::CancelReload()
{
	if (HasAuthority())
	{
		if (bIsReloading)
		{
			bIsReloading = false;
			OnRep_IsReloading();
		}
	}
	else
	{
		ServerCancelReload();
	}
}

void AGE_FireWeapon::SetTotalAmmo(int32 NewTotal)
{
	if (HasAuthority())
	{
		const int32 OldTotalAmmo = TotalAmmo;
		TotalAmmo = FMath::Max(0, NewTotal);
		OnRep_TotalAmmo(OldTotalAmmo);
	}
}

void AGE_FireWeapon::AddTotalAmmo(int32 Delta)
{
	if (HasAuthority() && Delta != 0)
	{
		const int32 OldTotalAmmo = TotalAmmo;
		TotalAmmo = FMath::Max(0, TotalAmmo + Delta);
		OnRep_TotalAmmo(OldTotalAmmo);
	}
}

bool AGE_FireWeapon::CanReload() const
{
	if (!IsEquipped())
	{
		return false;
	}
	if (bIsReloading)
	{
		return false;
	}
	if (AmmoInMag >= MagazineCapacity)
	{
		return false;
	}
	if (TotalAmmo <= 0)
	{
		return false;
	}

	return true;
}

void AGE_FireWeapon::CycleFireMode()
{
	TArray<EGEFireMode> Modes;
	Modes.Add(EGEFireMode::Semi);
	if (bBurstSupport) { Modes.Add(EGEFireMode::Burst); }
	if (bAutoSupport) { Modes.Add(EGEFireMode::Auto); }

	int32 Index = Modes.Find(FireMode);
	if (Index == INDEX_NONE) { Index = 0; }
	const EGEFireMode NextMode = Modes[(Index + 1) % Modes.Num()];

	if (FireMode != NextMode)
	{
		if (HasAuthority())
		{
			RepFireMode = NextMode;
			FireMode = NextMode;
			OnRep_FireMode();
		}
		else
		{
			ServerSetFireMode(NextMode);
		}
	}
}

FTransform AGE_FireWeapon::GetMuzzleTransform(bool bUseFirstPerson) const
{
	static const FName MuzzleSocket(TEXT("Muzzle"));

	const USkeletalMeshComponent* EqMesh = bUseFirstPerson ? GetMeshFP() : GetMeshTP();
	const USkeletalMeshComponent* Target = (EqMesh && EqMesh->DoesSocketExist(MuzzleSocket)) ? EqMesh : nullptr;

	if (!Target) return FTransform::Identity;

	FTransform T = Target->GetSocketTransform(MuzzleSocket, RTS_World);
	T.SetScale3D(FVector::OneVector);
	return T;
}

FTransform AGE_FireWeapon::GetBulletSpawnTransform() const
{
	FTransform Muzzle = GetMuzzleTransform(/*bUseFirstPerson*/ bOwnerIsFirstPerson);

	if (!OwningCharacter.IsValid()) return Muzzle;
	if (!IsLocallyControlled()) return Muzzle;

	FVector EyeLoc; FRotator EyeRot;
	OwningCharacter->GetActorEyesViewPoint(EyeLoc, EyeRot);

	const FTransform CameraTf(OwningCharacter->GetControlRotation(), EyeLoc);

	static const FName ADSSocket(TEXT("ADS"));

	const USkeletalMeshComponent* EqFP = bOwnerIsFirstPerson ? GetMeshFP() : GetMeshTP();
	FTransform SightTf = EqFP ? EqFP->GetComponentTransform() : CameraTf;
	
	if (EqFP && EqFP->DoesSocketExist(ADSSocket))
	{
		SightTf.SetLocation(EqFP->GetSocketLocation(ADSSocket));
		if (EqFP->DoesSocketExist(TEXT("ADS_End")))
		{
			SightTf.SetRotation(FRotationMatrix::MakeFromX(EqFP->GetSocketLocation(TEXT("ADS_End")) - EyeLoc).ToQuat());
		}
		else
		{
			SightTf.SetRotation(EyeRot.Quaternion());
		}
	}

	const FTransform AimTf = bIsAiming && bOwnerIsFirstPerson ? SightTf : CameraTf;

	FVector MuzzleInAim = UKismetMathLibrary::InverseTransformLocation(AimTf, Muzzle.GetLocation());
	MuzzleInAim = UKismetMathLibrary::TransformLocation(AimTf, FVector(MuzzleInAim.X * 0.8f, 0.f, 0.f));

	return FTransform(AimTf.GetRotation(), MuzzleInAim);
}

void AGE_FireWeapon::InputReload()
{
	StartReload();
}

void AGE_FireWeapon::InputCycleFireMode()
{
	CycleFireMode();
}

void AGE_FireWeapon::BeginFire()
{
	if (bBlockFireWhileReloading && bIsReloading)
	{
		return;
	}

	if (AmmoInMag <= 0)
	{
		BP_OnOutOfAmmo();
		return;
	}

	if (bRunFireDelayActive)
	{
		return;
	}

	if (ShouldUseRunFireDelay() && RunFireDelay > KINDA_SMALL_NUMBER)
	{
		bRunFireDelayActive = true;
		GetWorldTimerManager().SetTimer(TimerHandle_RunFireDelay, this, &AGE_FireWeapon::OnRunFireDelayElapsed, RunFireDelay, false);
		return;
	}

	BeginFire_Internal();
}

void AGE_FireWeapon::EndFire()
{
	if (FireMode == EGEFireMode::Burst && bBurstSequencing) return;

	GetWorldTimerManager().ClearTimer(TimerHandle_Fire);

	if (bIsFiring)
	{
		bIsFiring = false;
		OnRep_IsFiring();

		if (!HasAuthority())
		{
			ServerSetIsFiring(false);
		}
	}
}

void AGE_FireWeapon::BeginFire_Internal()
{
	if (!CanFire()) return;

	if (!bIsFiring)
	{
		bIsFiring = true;
		OnRep_IsFiring();

		if (!HasAuthority())
		{
			ServerSetIsFiring(true);
		}
	}

	const float Interval = GE_FireIntervalFromRPM(FireRateRPM);

	if (FireMode == EGEFireMode::Burst)
	{
		if (!bBurstSequencing)
		{
			bBurstSequencing = true;
			BurstLeft = FMath::Max(1, BurstCount);
			FireShot();
		}
	}
	else if (FireMode == EGEFireMode::Auto)
	{
		if (!GetWorldTimerManager().IsTimerActive(TimerHandle_Fire))
		{
			FireShot();
			GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &AGE_FireWeapon::FireShot, Interval, true);
		}
	}
	else // Semi
	{
		FireShot();
	}
}

void AGE_FireWeapon::FireShot()
{
	if (!CanFire())
	{
		if (FireMode == EGEFireMode::Burst)
		{
			bBurstSequencing = false;
			BurstLeft = 0;
		}
		return;
	}

	if (AmmoInMag <= 0)
	{
		BP_OnOutOfAmmo();
		
		if (FireMode == EGEFireMode::Auto)
		{
			EndFire();
		}
		if (FireMode == EGEFireMode::Burst)
		{
			bBurstSequencing = false;
			BurstLeft = 0;
		}
		return;
	}

	FVector Start;
	TArray<FVector> Dirs;
	TArray<FVector> Ends;
	ComputeShot(Start, Dirs, Ends);

	if (HasAuthority())
	{
		ApplyShot(Start, Dirs, Ends);
	}
	else
	{
		BulletData = FGE_BulletData(Start, Dirs, Ends);

		ServerApplyShot(Start, BulletData.Directions, BulletData.EndLocations);
	}

	HandleShotFXAndRecoil();

	if (FireMode == EGEFireMode::Burst)
	{
		BurstLeft = FMath::Max(0, BurstLeft - 1);
		if (BurstLeft > 0)
		{
			const float Interval = GE_FireIntervalFromRPM(FireRateRPM);
			GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &AGE_FireWeapon::FireShot, Interval, false);
		}
		else
		{
			bBurstSequencing = false;
			EndFire();
		}
	}
}

void AGE_FireWeapon::HandleShotFXAndRecoil()
{
	BP_OnFiredFX();
	BP_OnRecoilPlay();

	TArray<FHitResult> Hits;
	if (GetHitResult(Hits))
	{
		for (const FHitResult& Hit : Hits)
		{
			if (Hit.IsValidBlockingHit())
			{
				SpawnImpactFXHandler(GetHitResultWithPhysMaterial(Hit));
			}
		}
	}

	if (Imprecision != 0)
	{
		Imprecision -= 1;
		if (Imprecision <= 0)
		{
			Imprecision = 0;
		}
	}

	BulletFired += 1;
}

bool AGE_FireWeapon::GetHitResult(TArray<FHitResult>& OutHits) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), true, GetOwner());
	Params.bReturnPhysicalMaterial = false;
	Params.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable;

	for (const FVector_NetQuantize100& EndLocation : BulletData.EndLocations)
	{
		FHitResult OutHit{ ForceInit };
		GetWorld()->LineTraceSingleByChannel(OutHit, BulletData.StartLocation, EndLocation, TraceChannel, Params);
		OutHits.Add(OutHit);
	}

	return OutHits.Num() > 0;
}

FHitResult AGE_FireWeapon::GetHitResultWithPhysMaterial(const FHitResult& InHit) const
{
	FHitResult ActualHitResult = InHit;
	EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType_Default;

	// Check for complex phys mat (material physic default)
	if (UPhysicalMaterial* PhysMaterial = InHit.PhysMaterial.Get())
	{
		SurfaceType = PhysMaterial->SurfaceType;
	}

	constexpr float Distance = 10.0f;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bReturnPhysicalMaterial = true;
	CollisionParams.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable;
	CollisionParams.bTraceComplex = false;
	CollisionParams.AddIgnoredActor(GetOwner());

	const FVector ForwardVector = (InHit.TraceEnd - InHit.TraceStart).GetSafeNormal();
	const FVector& Start = InHit.Location - (ForwardVector * Distance);
	const FVector& End = InHit.Location + (ForwardVector * Distance);

	FHitResult Hit(ForceInit);
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, CollisionParams);
	if (bIsHit && InHit.GetActor() == Hit.GetActor())
	{
		if (UPhysicalMaterial* PhysicalMaterial = Hit.PhysMaterial.Get())
		{
			ActualHitResult.PhysMaterial = Hit.PhysMaterial;
		}

		ActualHitResult.Location = Hit.Location;
		ActualHitResult.Normal = Hit.Normal;
		ActualHitResult.ImpactPoint = Hit.ImpactPoint;
		ActualHitResult.ImpactNormal = Hit.ImpactNormal;
	}

	return ActualHitResult;
}

void AGE_FireWeapon::SpawnImpactFXHandler(const FHitResult& InHit)
{
	// #TODO
}

bool AGE_FireWeapon::CanFire() const
{
	if (!IsEquipped())
	{
		return false;
	}
	if (bBlockFireWhileReloading && bIsReloading)
	{
		return false;
	}

	return true;
}

bool AGE_FireWeapon::ShouldUseRunFireDelay() const
{
	return OwningCharacterInterface && OwningCharacterInterface->ShouldUseRunFireDelay();
}

void AGE_FireWeapon::OnRunFireDelayElapsed()
{
	bRunFireDelayActive = false;

	if (bBlockFireWhileReloading && bIsReloading)
	{
		return;
	}

	if (AmmoInMag <= 0)
	{
		BP_OnOutOfAmmo();
		return;
	}

	if (bWantsToFireInput || FireMode == EGEFireMode::Burst)
	{
		BeginFire_Internal();
		return;
	}

	if (!bIsFiring)
	{
		bIsFiring = true;
		if (HasAuthority())
		{
			OnRep_IsFiring();
		}
		else
		{
			ServerSetIsFiring(true);
		}
	}

	FireShot();

	const float Delay = GE_FireIntervalFromRPM(FireRateRPM);
	GetWorldTimerManager().SetTimer(TimerHandle_Fire, this, &AGE_FireWeapon::EndFire, Delay, false);
}

void AGE_FireWeapon::ComputeShot(FVector& OutStart, TArray<FVector>& OutDirs, TArray<FVector>& OutEnds)
{
	const FTransform BulletSpawnTransform = GetBulletSpawnTransform();
	OutStart = BulletSpawnTransform.GetLocation();
	FRotator Rot = BulletSpawnTransform.GetRotation().Rotator();

	const ACharacter* C = Cast<ACharacter>(GetOwner());
	const FVector Vel = C ? C->GetVelocity() : FVector::ZeroVector;
	const float AimRatio = bIsAiming ? 1.f : 0.f;

	const float Spread = ProjectilesPerShot == 1 ? GetImprecision(AimRatio, Vel) : 2.0f;

	for (int32 i = 0; i < ProjectilesPerShot; ++i)
	{
		if (Spread > 0.f)
		{
			const FRotator RandRot(FMath::RandRange(-Spread, Spread), FMath::RandRange(-Spread, Spread), FMath::RandRange(-Spread, Spread));
			Rot += RandRot;
		}

		FVector OutDir = Rot.GetNormalized().Vector();
		OutDirs.Add(OutDir);

		OutEnds.Add(OutStart + OutDir * TraceDist);
	}
}

void AGE_FireWeapon::ApplyShot(const FVector& Start, const TArray<FVector>& Dirs, const TArray<FVector>& Ends)
{
	if (bBlockFireWhileReloading && bIsReloading)
	{
		return;
	}

	if (!ConsumeOneFromMagazine())
	{
		return;
	}

	BulletData = FGE_BulletData(Start, Dirs, Ends);
	OnRep_BulletData();

	TArray<FHitResult> Hits;
	if (GetHitResult(Hits))
	{
		for (int32 i = 0; i < Hits.Num(); ++i)
		{
			if (!Dirs.IsValidIndex(i) || !Ends.IsValidIndex(i)) continue;

			const FHitResult& Hit = Hits[i];
			const FVector& Dir = Dirs[i];
			const FVector& End = Ends[i];

			if (Hit.IsValidBlockingHit())
			{
				if (Hit.GetActor() && Hit.GetActor()->CanBeDamaged())
				{
					FPointDamageEvent Dmg;
					Dmg.DamageTypeClass = UDamageType::StaticClass();
					if (DamageTypeClass) { Dmg.DamageTypeClass = DamageTypeClass; }
					Dmg.ShotDirection = Dir;
					Dmg.HitInfo = Hit;
					Dmg.Damage = BaseDamage;

					AController* Instig = OwningCharacter.IsValid() ? OwningCharacter->GetController() : nullptr;
					Hit.GetActor()->TakeDamage(Dmg.Damage, Dmg, Instig, this);
				}
			}
		}
	}
}

float AGE_FireWeapon::GetImprecision(float AimingRatio, const FVector& OwnerVelocity) const
{
	const float ImprecisionRatio = static_cast<float>(Imprecision) / static_cast<float>(MaxImprecision);
	const float ImprecisionRatioValue = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 1.0f), FVector2D(MaxImprecisionAmount, MinImprecisionAmount), ImprecisionRatio);
	const float TargetImprecision = ((ImprecisionRatioValue + OwnerVelocity.Size()) * 0.001f) * (ImprecisionBaseAmount - AimingRatio * ImprecisionAmountMultiplier);

	return TargetImprecision;
}

bool AGE_FireWeapon::ConsumeOneFromMagazine()
{
	if (!HasAuthority()) return false;

	if (AmmoInMag <= 0) return false;

	SetAmmoInMag(AmmoInMag - 1);
	return true;
}

int32 AGE_FireWeapon::LoadFromReserve(int32 Desired)
{
	if (!HasAuthority() || Desired <= 0) return 0;

	const int32 Space = FMath::Max(0, MagazineCapacity - AmmoInMag);

	if (Space <= 0 || TotalAmmo <= 0) return 0;

	const int32 ToMove = FMath::Min3(Desired, Space, TotalAmmo);
	SetAmmoInMag(AmmoInMag + ToMove);

	const int32 OldTotalAmmo = TotalAmmo;
	TotalAmmo -= ToMove;
	OnRep_TotalAmmo(OldTotalAmmo);

	return ToMove;
}

void AGE_FireWeapon::SetAmmoInMag(int32 NewAmount)
{
	if (!HasAuthority()) return;

	const int32 Clamped = FMath::Clamp(NewAmount, 0, MagazineCapacity);
	if (AmmoInMag != Clamped)
	{
		AmmoInMag = Clamped;
		OnRep_AmmoInMag();
	}
}

void AGE_FireWeapon::SetIsAiming(bool bNew)
{
	if (bIsAiming == bNew) return;

	RefreshADSOffset();

	bIsAiming = bNew;
	OnRep_IsAiming();

	if (OwningCharacterInterface)
	{
		OwningCharacterInterface->SetIsAimingEquipment(bNew);
	}

	if (!HasAuthority())
	{
		ServerSetIsAiming(bNew);
	}
}

void AGE_FireWeapon::RefreshAimingTimeline(float DeltaSeconds)
{
	const float TargetADS = bIsAiming ? 1.f : 0.f;
	const float TargetFOV = bIsAiming ? 1.f : 0.f;

	const float InSpeed = bIsAiming ? AimInSpeed : AimOutSpeed;
	const float OutSpeed = bIsAiming ? AimInSpeed : AimOutSpeed;

	ADSPlayback = FMath::FInterpTo(ADSPlayback, TargetADS, DeltaSeconds, InSpeed);
	FOVPlayback = FMath::FInterpTo(FOVPlayback, TargetFOV, DeltaSeconds, OutSpeed);

	ADSAlpha = FMath::Clamp(ADSPlayback, 0.f, 1.f);
	FOVAlpha = FMath::Clamp(FOVPlayback, 0.f, 1.f);

	BP_OnRefreshADSAlpha(ADSAlpha);
	BP_OnRefreshFOVAlpha(FOVAlpha);
}

void AGE_FireWeapon::RefreshADSOffset()
{
	USkeletalMeshComponent* EqMesh = bOwnerIsFirstPerson ? GetMeshFP() : GetMeshTP();
	ADSOffset = EqMesh ? EqMesh->GetSocketTransform(TEXT("ADS"), RTS_Component) : FTransform::Identity;

	BP_OnRefreshADSOffset(ADSOffset);
}

void AGE_FireWeapon::ServerSetIsFiring_Implementation(bool bNew)
{
	bIsFiring = bNew;
	OnRep_IsFiring();
}

void AGE_FireWeapon::ServerSetIsAiming_Implementation(bool bNew)
{
	bIsAiming = bNew;
	OnRep_IsAiming();
}

void AGE_FireWeapon::ServerRequestStartReload_Implementation()
{
	if (CanReload() && !bIsReloading)
	{
		bIsReloading = true;
		OnRep_IsReloading();
	}
}

void AGE_FireWeapon::ServerInsertOneRound_Implementation()
{
	if (!bLoopReload || !bIsReloading) return;

	const int32 Moved = LoadFromReserve(1);
	const bool bDone = (Moved <= 0) || (AmmoInMag >= MagazineCapacity) || (TotalAmmo <= 0);
	if (bDone)
	{
		bIsReloading = false;
		OnRep_IsReloading();
	}
}

void AGE_FireWeapon::ServerCommitReload_Implementation(bool bForceFull)
{
	if (!bIsReloading) return;

	const int32 Need = FMath::Max(0, MagazineCapacity - AmmoInMag);
	if (Need > 0)
	{
		LoadFromReserve(Need);
	}

	bIsReloading = false;
	OnRep_IsReloading();
}

void AGE_FireWeapon::ServerCancelReload_Implementation()
{
	if (bIsReloading)
	{
		bIsReloading = false;
		OnRep_IsReloading();
	}
}

void AGE_FireWeapon::ServerApplyShot_Implementation(const FVector_NetQuantize100& Start, const TArray<FVector_NetQuantizeNormal>& Dirs, const TArray<FVector_NetQuantize100>& Ends)
{
	TArray<FVector> TargetDirs;
	TArray<FVector> TargetEnds;
	TargetDirs.Reserve(Dirs.Num());
	TargetEnds.Reserve(Ends.Num());

	for (const FVector_NetQuantizeNormal& Dir : Dirs)
	{
		TargetDirs.Add(Dir);
	}

	for (const FVector_NetQuantizeNormal& End : Ends)
	{
		TargetEnds.Add(End);
	}

	ApplyShot(Start, TargetDirs, TargetEnds);
}

void AGE_FireWeapon::ServerSetFireMode_Implementation(EGEFireMode NewMode)
{
	RepFireMode = NewMode;
	FireMode = NewMode;
	OnRep_FireMode();
}

void AGE_FireWeapon::OnRep_AmmoInMag()
{
	OnAmmunitionChanged.Broadcast(AmmoInMag, TotalAmmo);
}

void AGE_FireWeapon::OnRep_TotalAmmo(int32 OldTotalAmmo)
{
	OnAmmunitionChanged.Broadcast(AmmoInMag, TotalAmmo);

	if (GetNetMode() != NM_DedicatedServer)
	{
		if (TotalAmmo > OldTotalAmmo)
		{
			UGameplayStatics::SpawnSoundAtLocation(this, AmmoPickupSound, GetActorLocation());
		}
	}
}

void AGE_FireWeapon::OnRep_IsFiring()
{
	if (bIsFiring)
	{
		// ...
	}
	else
	{
		BP_OnRecoilStop();

		Imprecision = MaxImprecision;
		BulletFired = 0;
	}
}

void AGE_FireWeapon::OnRep_IsReloading()
{
	if (bIsReloading)
	{
		PlayEquipmentMontageSet(ReloadAnim, /*FP*/IsLocallyControlled(), /*TP*/true);
	}
}

void AGE_FireWeapon::OnRep_IsAiming()
{
	BP_OnAimChanged(bIsAiming);
}

void AGE_FireWeapon::OnRep_BulletData()
{
	if (!IsLocallyControlled())
	{
		HandleShotFXAndRecoil();
	}
}

void AGE_FireWeapon::OnRep_FireMode()
{
	FireMode = RepFireMode;
	OnFireModeChanged.Broadcast(FireMode);
}

#undef LOCTEXT_NAMESPACE