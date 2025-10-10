// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/FPS_Types.h"
#include "Engine/DamageEvents.h"
#include "FPS_HealthComponent.generated.h"

class UDamageType;

USTRUCT(BlueprintType)
struct FDeathEventPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	AController* InstigatorController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	TObjectPtr<AActor> DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	bool bRadial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FHitResult Hit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector Impulse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FVector ImpulseAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FName BoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	float FinalDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	bool bHeadshot;

	FDeathEventPayload()
		: bRadial(false)
		, Impulse(FVector::ZeroVector)
		, ImpulseAt(FVector::ZeroVector)
		, BoneName(NAME_None)
		, FinalDamage(0.0f)
		, bHeadshot(false)
	{}

	FDeathEventPayload(AController* InCtl, TSubclassOf<UDamageType> InDT, AActor* InCauser, bool bInExplosive,
		const FHitResult& InHit, const FVector& InImpulse, const FVector& InImpulseAt, const FName& InBone,
		float InFinalDamage, bool bInHeadshot)
		: InstigatorController(InCtl), DamageType(InDT), DamageCauser(InCauser), bRadial(bInExplosive)
		, Hit(InHit), Impulse(InImpulse), ImpulseAt(InImpulseAt), BoneName(InBone)
		, FinalDamage(InFinalDamage), bHeadshot(bInHeadshot)
	{}
};

USTRUCT(BlueprintType)
struct FBoneDamage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	FName BoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Defaults)
	float DamageFactor = 1.0f;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTookDamage, const FHitResult&, Hit, float, DamageAmount, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathWithPayload, const FDeathEventPayload&, Data);

UCLASS(ClassGroup=(FPS), meta=(BlueprintSpawnableComponent, DisplayName="Health Component"))
class FPSGAME_V2_API UFPS_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	float MaxHealth = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings", meta=(ClampMin="0", ClampMax="1"))
	float LowHealthThresholdPercentage = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	float DamageableTimeAfterSpawn = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TArray<FBoneDamage> PerBoneDamageMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	float BulletImpulseScale = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	float ExplosionImpulseScale = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UDamageType> SuicideDamageType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TSubclassOf<UDamageType> FallingDamageType;

public:
	UFPS_HealthComponent();

	//~ AActor
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End of AActor

public:
	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnDeathWithPayload OnDeathPayload;

	UPROPERTY(BlueprintAssignable, Category="Health|Events")
	FOnTookDamage OnTookDamage;

public:
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsAlive() const { return Health > 0.f; }

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	void Suicide();

	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	void InstaKill(TSubclassOf<UDamageType> DamageType, AController* InInstigator, AActor* Causer);

	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	void ApplyDamageOfType(TSubclassOf<UDamageType> DamageType, float Amount, AController* InInstigator, AActor* Causer);

	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	void ApplyFallDamage(float Amount);

	// For owners that still override TakeDamage and want to forward explicitly.
	UFUNCTION(BlueprintCallable, Category="Health", BlueprintAuthorityOnly)
	float ApplyDamageEvent(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health", ReplicatedUsing=OnRep_Health)
	float Health = 100.0f;
	
	UPROPERTY(BlueprintReadOnly, Category="Health")
	TArray<FDamageInfo> DamageInfoArray;

	bool bDeathBroadcasted = false;

	FTimerHandle TimerHandle_Damageable;

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Health")
	float AdjustDamage(float InDamage, AController* InstigatorController);

	UFUNCTION()
	void HandleAnyDamage(
		AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void HandlePointDamage(
		AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
		UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
		const UDamageType* DamageType, AActor* DamageCauser);

	UFUNCTION()
	void HandleRadialDamage(
		AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin,
		const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser);

	void StartDamageableDelayIfNeeded();

	void SetHealth(float NewValue);
	
	UFUNCTION()
	void OnRep_Health();

	float GetPerBoneMultiplier(const FName& BoneName) const;
	void BroadcastDeathOnce(const FDeathEventPayload& Payload);
	void BroadcastTookDamage(const FHitResult& Hit, float Amount, AActor* Causer);
		
};
