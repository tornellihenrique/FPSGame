#pragma once

#include "CoreMinimal.h"
#include "Equipments/GE_Equipment.h"
#include "GE_FireWeapon.generated.h"

class USoundBase;
class UDamageType;

UENUM(BlueprintType)
enum class EGEFireMode : uint8
{
	Semi  = 0,
	Burst = 1,
	Auto  = 2
};

USTRUCT(BlueprintType)
struct FGE_BulletData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector_NetQuantize100 StartLocation = FVector_NetQuantize100::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector_NetQuantizeNormal> Directions;

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector_NetQuantize100> EndLocations;

	FGE_BulletData() {};

	FGE_BulletData(const FVector& InStart, const FVector& InDir, const FVector& InEnd)
	{
		StartLocation = InStart;
		Directions.Add(InDir.GetSafeNormal());
		EndLocations.Add(InEnd);
	}

	FGE_BulletData(const FVector& InStart, const TArray<FVector>& InDirs, const TArray<FVector>& InEnds)
	{
		StartLocation = InStart;
		Directions.Reserve(InDirs.Num());
		EndLocations.Reserve(InEnds.Num());
		for (int32 i = 0; i < InDirs.Num(); ++i)
		{
			Directions.Add(InDirs[i].GetSafeNormal());
		}
		for (int32 i = 0; i < InEnds.Num(); ++i)
		{
			EndLocations.Add(InEnds[i]);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGE_OnAmmunitionChanged, int32, Ammunition, int32, TotalAmmunition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGE_OnMagazineChanged, int32, MagazineCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGE_OnFireModeChanged, EGEFireMode, FireMode);

UCLASS(Abstract, meta=(DisplayName="Fire Weapon"))
class GAMEPLAYEQUIPMENTS_API AGE_FireWeapon : public AGE_Equipment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire")
	EGEFireMode FireMode = EGEFireMode::Semi;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire")
	bool bBurstSupport = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire")
	bool bAutoSupport = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire", meta=(ClampMin=60.0, ClampMax=2000.0))
	float FireRateRPM = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire", meta=(ClampMin=1))
	int32 BurstCount = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire", meta=(ClampMin=0.0, ClampMax=0.25))
	float RunFireDelay = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire", meta=(ClampMin=1))
	int32 ProjectilesPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Fire", meta=(ClampMin=1))
	float TraceDist = 20000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Ammo", meta=(ClampMin=1))
	int32 MagazineCapacity = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Ammo", meta=(ClampMin=0))
	int32 InitialTotalAmmo = 90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Ammo")
	TObjectPtr<USoundBase> AmmoPickupSound;
	
	// #TODO
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|AmmoDrop")
	// TSubclassOf<AGE_AmmoPickup> AmmoDropClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|AmmoDrop", meta=(ClampMin=0.0, ClampMax=1.0))
	float DropAmmoFraction = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|AmmoDrop")
	bool bDropIncludesMagazine = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|AmmoDrop", meta=(ClampMin=-1))
	int32 DropAmmoMax = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Reload")
	bool bLoopReload = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Reload")
	FName ReloadLoopSectionName = TEXT("LoopReload");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Reload")
	bool bAimedReload = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Reload")
	bool bBlockFireWhileReloading = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Reload")
	FEquipmentAnimMontageData ReloadAnim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Damage")
	float BaseDamage = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|VFX")
	// TSubclassOf<AGE_ImpactFXHandler> ImpactFXHandlerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Spread")
	float ImprecisionBaseAmount = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Spread")
	float ImprecisionAmountMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Spread")
	int32 MaxImprecision = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Spread")
	float MinImprecisionAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Spread")
	float MaxImprecisionAmount = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|ADS")
	float AimZoom = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|ADS")
	float AimInSpeed = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|ADS")
	float AimOutSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|ADS")
	float AimAnimAlpha = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Input")
	TArray<FKey> ReloadKeys;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Weapon|Input")
	TArray<FKey> FireModeKeys;

public:
	AGE_FireWeapon(const FObjectInitializer& ObjectInitializer);

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

	//~ AGE_Equipment
	virtual void SetOwningCharacter(ACharacter* NewOwner) override;

	virtual void OnRep_Equipped() override;

	virtual void OnBeforeRemoved(UGE_EquipmentManagerComponent* Manager, EGE_EquipmentRemovalReason Reason) override;

	virtual float BeginEquip(bool bFirstPerson, bool bThirdPerson) override;
	virtual float BeginUnequip(bool bFirstPerson, bool bThirdPerson) override;

	virtual bool CanRun() const override;

	virtual void PrimaryAction_Implementation(bool bPressed) override;
	virtual void SecondaryAction_Implementation(bool bPressed) override;
	//~ End of AGE_Equipment

public:
	UPROPERTY(BlueprintAssignable)
	FGE_OnAmmunitionChanged OnAmmunitionChanged;
	UPROPERTY(BlueprintAssignable)
	FGE_OnMagazineChanged OnMagazineChanged;
	UPROPERTY(BlueprintAssignable)
	FGE_OnFireModeChanged OnFireModeChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_AmmoInMag)
	int32 AmmoInMag = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_TotalAmmo)
	int32 TotalAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_IsFiring)
	bool bIsFiring = false;

	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_IsReloading)
	bool bIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_IsAiming)
	bool bIsAiming = false;
	
	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_BulletData)
	FGE_BulletData BulletData;
	
	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_FireMode)
	EGEFireMode RepFireMode = EGEFireMode::Semi;
	
	UPROPERTY(BlueprintReadOnly, Category="State")
	FTransform ADSOffset = FTransform::Identity;

	int32 BurstLeft = 0;
	int32 Imprecision = 0;
	int32 BulletFired = 0;

	float AimHoldTime = 0.f;

	float FOVPlayback = 0.f;
	float ADSPlayback = 0.f;
	float FOVAlpha = 0.f;
	float ADSAlpha = 0.f;

	uint8 bBurstSequencing : 1;

	uint8 bRunFireDelayActive : 1;
	uint8 bWantsToFireInput : 1;

	uint8 bBindedInputs : 1;

	UPROPERTY()
	TObjectPtr<UInputComponent> LastInputComponent;

	FTimerHandle TimerHandle_Fire;
	FTimerHandle TimerHandle_RunFireDelay;

public:
	UFUNCTION(BlueprintCallable, Category="Weapon|Ammo")
	void StartReload();
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Reload")
	void Notify_InsertOneRound();
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Reload")
	void Notify_CommitReload(bool bForceFull = false);

	UFUNCTION(BlueprintCallable, Category="Weapon|Reload")
	void CancelReload();
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Ammo")
	void SetTotalAmmo(int32 NewTotal);

	UFUNCTION(BlueprintCallable, Category="Weapon|Ammo")
	void AddTotalAmmo(int32 Delta);

	UFUNCTION(BlueprintPure, Category="Weapon|Ammo")
	int32 GetTotalAmmo() const { return TotalAmmo; }

	UFUNCTION(BlueprintPure, Category="Weapon|Ammo")
	int32 GetAmmoInMagazine() const { return AmmoInMag; }
	
	UFUNCTION(BlueprintPure, Category="Weapon|Ammo")
	bool CanReload() const;
	
	UFUNCTION(BlueprintCallable, Category="Weapon|Fire")
	void CycleFireMode();

public:
	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Recoil Play"))
	void BP_OnRecoilPlay();

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Recoil Stop"))
	void BP_OnRecoilStop();

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Fired FX"))
	void BP_OnFiredFX();

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Out Of Ammo"))
	void BP_OnOutOfAmmo();

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Aim Changed"))
	void BP_OnAimChanged(bool bNewAiming);

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Refresh ADS Alpha"))
	void BP_OnRefreshADSAlpha(float InADSAlpha);

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Refresh FOV Alpha"))
	void BP_OnRefreshFOVAlpha(float InFOVAlpha);

	UFUNCTION(BlueprintImplementableEvent, Category="Weapon|BP Events", meta=(DisplayName="On Refresh ADS Offset"))
	void BP_OnRefreshADSOffset(const FTransform& InADSOffset);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon|Util")
	FTransform GetMuzzleTransform(bool bUseFirstPerson) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Weapon|Util")
	FTransform GetBulletSpawnTransform() const;
	
	UFUNCTION(BlueprintPure, Category="State")
	bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintPure, Category="State")
	bool IsFiring() const { return bIsFiring; }

	UFUNCTION(BlueprintPure, Category="State")
	float GetADSAlpha() const { return ADSAlpha; }

	UFUNCTION(BlueprintPure, Category="State")
	float GetFOVAlpha() const { return FOVAlpha; }

protected:
	void InputReload();
	void InputCycleFireMode();

protected:
	void BeginFire();
	void EndFire();
	void BeginFire_Internal();
	void FireShot();
	void HandleShotFXAndRecoil();

	bool GetHitResult(TArray<FHitResult>& OutHits) const;
	FHitResult GetHitResultWithPhysMaterial(const FHitResult& InHit) const;

	void SpawnImpactFXHandler(const FHitResult& InHit);

	bool CanFire() const;

	bool ShouldUseRunFireDelay() const;
	void OnRunFireDelayElapsed();

	void ComputeShot(FVector& OutStart, TArray<FVector>& OutDirs, TArray<FVector>& OutEnds);
	void ApplyShot(const FVector& Start, const TArray<FVector>& Dirs, const TArray<FVector>& Ends);

	float GetImprecision(float AimingRatio, const FVector& OwnerVelocity) const;

protected:
	bool ConsumeOneFromMagazine();
	int32 LoadFromReserve(int32 Desired);
	void SetAmmoInMag(int32 NewAmount);

protected:
	void SetIsAiming(bool bNew);
	void RefreshAimingTimeline(float DeltaSeconds);
	void RefreshADSOffset();

protected:
	UFUNCTION(Server, Reliable)
	void ServerSetIsFiring(bool bNew);
	
	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bNew);

	UFUNCTION(Server, Reliable)
	void ServerRequestStartReload();
	
	UFUNCTION(Server, Reliable)
	void ServerInsertOneRound();
	
	UFUNCTION(Server, Reliable)
	void ServerCommitReload(bool bForceFull);
	
	UFUNCTION(Server, Reliable)
	void ServerCancelReload();

	UFUNCTION(Server, Reliable)
	void ServerApplyShot(const FVector_NetQuantize100& Start, const TArray<FVector_NetQuantizeNormal>& Dirs, const TArray<FVector_NetQuantize100>& Ends);

	UFUNCTION(Server, Reliable)
	void ServerSetFireMode(EGEFireMode NewMode);

protected:
	UFUNCTION()
	void OnRep_AmmoInMag();
	
	UFUNCTION()
	void OnRep_TotalAmmo(int32 OldTotalAmmo);
	
	UFUNCTION()
	void OnRep_IsFiring();
	
	UFUNCTION()
	void OnRep_IsReloading();
	
	UFUNCTION()
	void OnRep_IsAiming();
	
	UFUNCTION()
	void OnRep_BulletData();
	
	UFUNCTION()
	void OnRep_FireMode();

};
