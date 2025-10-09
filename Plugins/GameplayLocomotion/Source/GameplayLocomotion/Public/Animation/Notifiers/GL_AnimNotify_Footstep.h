#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "GL_AnimNotify_Footstep.generated.h"

class USoundBase;
class UMaterialInterface;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EGL_FootBone : uint8
{
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class EGL_FootstepSoundType : uint8
{
	Step,
	WalkRun,
	Land
};

UENUM(BlueprintType)
enum class EGL_FootstepSoundSpawnMode : uint8
{
	AtTraceHit,
	AttachToFoot
};

UENUM(BlueprintType)
enum class EGL_FootstepDecalSpawnMode : uint8
{
	AtTraceHit,
	AttachToHitComponent
};

UENUM(BlueprintType)
enum class EGL_FootstepParticleSpawnMode : uint8
{
	AtTraceHit,
	AttachToFoot
};

USTRUCT(BlueprintType)
struct FGL_FootstepSoundSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	EGL_FootstepSoundSpawnMode SpawnMode = EGL_FootstepSoundSpawnMode::AtTraceHit;
};

USTRUCT(BlueprintType)
struct FGL_FootstepDecalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal")
	TSoftObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal")
	EGL_FootstepDecalSpawnMode SpawnMode = EGL_FootstepDecalSpawnMode::AttachToHitComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal", Meta = (AllowPreserveRatio))
	FVector3f Size = FVector3f(10.f, 20.f, 20.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal", Meta = (ClampMin = 0, ForceUnits = "s"))
	float Duration = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal", Meta = (ClampMin = 0, ForceUnits = "s"))
	float FadeOutDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal")
	FVector3f LocationOffset = FVector3f(0.f, -10.f, -1.75f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal")
	FRotator3f LeftFootRotationOffset = FRotator3f(90.f, 0.f, -90.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Decal")
	FRotator3f RightFootRotationOffset = FRotator3f(-90.f, 0.f, 90.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Decal", AdvancedDisplay)
	FQuat4f LeftFootRotationOffsetQuat = FQuat4f(LeftFootRotationOffset);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Decal", AdvancedDisplay)
	FQuat4f RightFootRotationOffsetQuat = FQuat4f(RightFootRotationOffset);

#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent);
#endif
};

USTRUCT(BlueprintType)
struct FGL_FootstepParticleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TSoftObjectPtr<UNiagaraSystem> System;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	EGL_FootstepParticleSpawnMode SpawnMode = EGL_FootstepParticleSpawnMode::AtTraceHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FVector3f LocationOffset = FVector3f(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FRotator3f LeftFootRotationOffset = FRotator3f(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FRotator3f RightFootRotationOffset = FRotator3f(0.f, 0.f, 0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX", AdvancedDisplay)
	FQuat4f LeftFootRotationOffsetQuat = FQuat4f(LeftFootRotationOffset);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="VFX", AdvancedDisplay)
	FQuat4f RightFootRotationOffsetQuat = FQuat4f(RightFootRotationOffset);

#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent);
#endif
};

USTRUCT(BlueprintType)
struct FGL_FootstepEffectSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	FGL_FootstepSoundSettings Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	FGL_FootstepDecalSettings Decal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects")
	FGL_FootstepParticleSettings Particle;

#if WITH_EDITOR
	void PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent);
#endif
};

UCLASS(BlueprintType)
class UGL_FootstepEffectsSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trace")
	TEnumAsByte<ECollisionChannel> SurfaceTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trace", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float SurfaceTraceDistance = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bones")
	FName LeftFootBone = TEXT("foot_l");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bones")
	FName RightFootBone = TEXT("foot_r");

	// Local foot axes used to align decals / particles on the hit surface.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Axes", DisplayName = "Foot Left Y Axis")
	FVector3f LeftFootYAxis = FVector3f(0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Axes", DisplayName = "Foot Left Z Axis")
	FVector3f LeftFootZAxis = FVector3f(1.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Axes", DisplayName = "Foot Right Y Axis")
	FVector3f RightFootYAxis = FVector3f(0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Axes", DisplayName = "Foot Right Z Axis")
	FVector3f RightFootZAxis = FVector3f(-1.f, 0.f, 0.f);

	// Blocks decal if angle between foot Z and surface normal exceeds threshold.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Decal", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float DecalSpawnAngleThreshold = 35.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Decal", AdvancedDisplay, Meta = (ClampMin = 0, ClampMax = 1))
	float DecalSpawnAngleThresholdCos = 0.81915206f; // cos(35 deg)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects", Meta = (ForceInlineRow))
	TMap<TEnumAsByte<EPhysicalSurface>, FGL_FootstepEffectSettings> Effects;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent) override;
#endif
};

UCLASS(meta=(DisplayName="Footstep"))
class GAMEPLAYLOCOMOTION_API UGL_AnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	TObjectPtr<UGL_FootstepEffectsSettings> Settings = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	EGL_FootBone Foot = EGL_FootBone::Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings")
	uint8 bSkipWhenFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Sound")
	uint8 bSpawnSound : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Sound", Meta = (ClampMin = 0, ForceUnits = "x"))
	float SoundVolume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Sound", Meta = (ClampMin = 0, ForceUnits = "x"))
	float SoundPitch = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Sound")
	EGL_FootstepSoundType SoundType = EGL_FootstepSoundType::Step;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Sound")
	uint8 bIgnoreSoundBlockCurve : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|Decal")
	uint8 bSpawnDecal : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Settings|VFX")
	uint8 bSpawnVFX : 1;

public:
	UGL_AnimNotify_Footstep();

	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual void OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& NotifyEvent) override;
#endif

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                    const FAnimNotifyEventReference& EventReference) override;

	FORCEINLINE EGL_FootBone GetFoot() const { return Foot; }
	FORCEINLINE EGL_FootstepSoundType GetFootstepType() const { return SoundType; }

private:
	void SpawnSoundInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepSoundSettings& SoundSettings,
	                        const FVector& Location, const FQuat& Rotation) const;

	void SpawnDecalInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepDecalSettings& DecalSettings,
	                        const FVector& Location, const FQuat& Rotation,
	                        const FHitResult& Hit, const FVector& FootZAxis) const;

	void SpawnVFXInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepParticleSettings& ParticleSettings,
	                      const FVector& Location, const FQuat& Rotation) const;

};
