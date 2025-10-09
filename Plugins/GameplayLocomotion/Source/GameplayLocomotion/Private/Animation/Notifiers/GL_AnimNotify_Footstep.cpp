#include "Animation/Notifiers/GL_AnimNotify_Footstep.h"

#include "Animation/AnimInstance.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GL_AnimNotify_Footstep)

#if WITH_EDITOR
void FGL_FootstepDecalSettings::PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent)
{
	LeftFootRotationOffsetQuat = FQuat4f(LeftFootRotationOffset);
	RightFootRotationOffsetQuat = FQuat4f(RightFootRotationOffset);
}

void FGL_FootstepParticleSettings::PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent)
{
	LeftFootRotationOffsetQuat = FQuat4f(LeftFootRotationOffset);
	RightFootRotationOffsetQuat = FQuat4f(RightFootRotationOffset);
}

void FGL_FootstepEffectSettings::PostEditChangeProperty(const FPropertyChangedEvent& ChangedEvent)
{
	Decal.PostEditChangeProperty(ChangedEvent);
	Particle.PostEditChangeProperty(ChangedEvent);
}

void UGL_FootstepEffectsSettings::PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent)
{
	const FName Member = ChangedEvent.GetMemberPropertyName();

	if (Member == GET_MEMBER_NAME_CHECKED(UGL_FootstepEffectsSettings, DecalSpawnAngleThreshold))
	{
		DecalSpawnAngleThresholdCos = FMath::Cos(FMath::DegreesToRadians(DecalSpawnAngleThreshold));
	}
	else if (Member == GET_MEMBER_NAME_CHECKED(UGL_FootstepEffectsSettings, Effects))
	{
		for (TPair<TEnumAsByte<EPhysicalSurface>, FGL_FootstepEffectSettings>& Pair : Effects)
		{
			Pair.Value.PostEditChangeProperty(ChangedEvent);
		}
	}

	Super::PostEditChangeProperty(ChangedEvent);
}
#endif

UGL_AnimNotify_Footstep::UGL_AnimNotify_Footstep()
{
	bSkipWhenFalling = false;
	bSpawnSound = true;
	bSpawnDecal = true;
	bSpawnVFX = true;
	bIgnoreSoundBlockCurve = false;
}

FString UGL_AnimNotify_Footstep::GetNotifyName_Implementation() const
{
	return Foot == EGL_FootBone::Left ? TEXT("Footstep: Left") : TEXT("Footstep: Right");
}

#if WITH_EDITOR
void UGL_AnimNotify_Footstep::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& NotifyEvent)
{
	Super::OnAnimNotifyCreatedInEditor(NotifyEvent);
	NotifyEvent.bTriggerOnDedicatedServer = false;
}
#endif

void UGL_AnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr || Settings == nullptr)
	{
		return;
	}

	if (bSkipWhenFalling)
	{
		ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
		if (Character && Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
		{
			return;
		}
	}

	const UWorld* World = MeshComp->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const FName BoneName = (Foot == EGL_FootBone::Left) ? Settings->LeftFootBone : Settings->RightFootBone;
	const FTransform FootXform = MeshComp->GetSocketTransform(BoneName);

	const FVector FootZAxisLocal = (Foot == EGL_FootBone::Left)
		? FVector(Settings->LeftFootZAxis)
		: FVector(Settings->RightFootZAxis);

	const FVector FootYAxisLocal = (Foot == EGL_FootBone::Left)
		? FVector(Settings->LeftFootYAxis)
		: FVector(Settings->RightFootYAxis);

	const FVector FootZAxis = FootXform.TransformVectorNoScale(FootZAxisLocal);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(GL_AnimNotify_Footstep), true, MeshComp->GetOwner());
	Params.bReturnPhysicalMaterial = true;

	const float meshScaleZ = MeshComp->GetComponentScale().Z;
	const float traceDist = Settings->SurfaceTraceDistance * meshScaleZ;

	FHitResult Hit;
	const FVector Start = FootXform.GetLocation();
	const FVector EndPrimary = Start - FootZAxis * traceDist;
	const bool bHitPrimary = World->LineTraceSingleByChannel(Hit, Start, EndPrimary, Settings->SurfaceTraceChannel, Params);

	if (!bHitPrimary)
	{
		const FVector EndFallback = Start - FVector(0.f, 0.f, traceDist);
		World->LineTraceSingleByChannel(Hit, Start, EndFallback, Settings->SurfaceTraceChannel, Params);
	}

	if (!Hit.bBlockingHit)
	{
		return;
	}

	const EPhysicalSurface Surface = Hit.PhysMaterial.IsValid()
		? Hit.PhysMaterial->SurfaceType.GetValue()
		: SurfaceType_Default;

	const FGL_FootstepEffectSettings* Effect = Settings->Effects.Find(Surface);
	if (Effect == nullptr && Settings->Effects.Num() > 0)
	{
		for (const TPair<TEnumAsByte<EPhysicalSurface>, FGL_FootstepEffectSettings>& Pair : Settings->Effects)
		{
			Effect = &Pair.Value;
			break;
		}
	}
	if (Effect == nullptr)
	{
		return;
	}

	const FVector Location = Hit.ImpactPoint;
	const FQuat   Rotation = FRotationMatrix::MakeFromZY(Hit.ImpactNormal, FootXform.TransformVectorNoScale(FootYAxisLocal)).ToQuat();

	if (bSpawnSound)
	{
		SpawnSoundInternal(MeshComp, Effect->Sound, Location, Rotation);
	}
	if (bSpawnDecal)
	{
		SpawnDecalInternal(MeshComp, Effect->Decal, Location, Rotation, Hit, FootZAxis);
	}
	if (bSpawnVFX)
	{
		SpawnVFXInternal(MeshComp, Effect->Particle, Location, Rotation);
	}
}

void UGL_AnimNotify_Footstep::SpawnSoundInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepSoundSettings& SoundSettings,
	const FVector& Location, const FQuat& Rotation) const
{
	float Volume = SoundVolume;

	if (!bIgnoreSoundBlockCurve)
	{
		const UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
		if (AnimInst)
		{
			// Optional curve named "FootstepSoundBlock" (0..1). If missing, GetCurveValue returns 0.
			const float BlockAlpha = FMath::Clamp(AnimInst->GetCurveValue(TEXT("FootstepSoundBlock")), 0.f, 1.f);
			Volume *= (1.f - BlockAlpha);
		}
	}

	if (!FAnimWeight::IsRelevant(Volume))
	{
		return;
	}

	USoundBase* Sound = SoundSettings.Sound.LoadSynchronous();
	if (Sound == nullptr)
	{
		return;
	}

	UAudioComponent* Audio = nullptr;

	if (SoundSettings.SpawnMode == EGL_FootstepSoundSpawnMode::AtTraceHit)
	{
		if (MeshComp->GetWorld() && MeshComp->GetWorld()->WorldType == EWorldType::EditorPreview)
		{
			UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), Sound, Location, Volume, SoundPitch);
		}
		else
		{
			Audio = UGameplayStatics::SpawnSoundAtLocation(MeshComp->GetWorld(), Sound, Location, Rotation.Rotator(), Volume, SoundPitch);
		}
	}
	else
	{
		const FName BoneName = (Foot == EGL_FootBone::Left) ? Settings->LeftFootBone : Settings->RightFootBone;
		Audio = UGameplayStatics::SpawnSoundAttached(Sound, MeshComp, BoneName, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget, true, Volume, SoundPitch);
	}

	if (Audio)
	{
		Audio->SetIntParameter(TEXT("FootstepType"), static_cast<int32>(SoundType));
	}
}

void UGL_AnimNotify_Footstep::SpawnDecalInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepDecalSettings& DecalSettings,
	const FVector& Location, const FQuat& Rotation,
	const FHitResult& Hit, const FVector& FootZAxis) const
{
	const float CosAngle = FVector::DotProduct(Hit.ImpactNormal, FootZAxis);
	if (CosAngle < Settings->DecalSpawnAngleThresholdCos)
	{
		return;
	}

	UMaterialInterface* Mat = DecalSettings.Material.LoadSynchronous();
	if (Mat == nullptr)
	{
		return;
	}

	const FQuat OffsetQuat = (Foot == EGL_FootBone::Left)
		? FQuat(DecalSettings.LeftFootRotationOffsetQuat)
		: FQuat(DecalSettings.RightFootRotationOffsetQuat);

	const FQuat DecalQuat = Rotation * OffsetQuat;
	const float meshScaleZ = MeshComp->GetComponentScale().Z;

	const FVector DecalLocation = Location + DecalQuat.RotateVector(FVector(DecalSettings.LocationOffset) * meshScaleZ);

	UDecalComponent* Decal = nullptr;

	if (DecalSettings.SpawnMode == EGL_FootstepDecalSpawnMode::AtTraceHit || !Hit.Component.IsValid())
	{
		Decal = UGameplayStatics::SpawnDecalAtLocation(MeshComp->GetWorld(), Mat,
			FVector(DecalSettings.Size) * meshScaleZ,
			DecalLocation, DecalQuat.Rotator());
	}
	else
	{
		Decal = UGameplayStatics::SpawnDecalAttached(Mat,
			FVector(DecalSettings.Size) * meshScaleZ,
			Hit.Component.Get(), NAME_None, DecalLocation,
			DecalQuat.Rotator(), EAttachLocation::KeepWorldPosition);
	}

	if (Decal)
	{
		Decal->SetFadeOut(DecalSettings.Duration, DecalSettings.FadeOutDuration, false);
	}
}

void UGL_AnimNotify_Footstep::SpawnVFXInternal(USkeletalMeshComponent* MeshComp, const FGL_FootstepParticleSettings& ParticleSettings,
	const FVector& Location, const FQuat& Rotation) const
{
	UNiagaraSystem* System = ParticleSettings.System.LoadSynchronous();
	if (System == nullptr)
	{
		return;
	}

	const float meshScaleZ = MeshComp->GetComponentScale().Z;

	if (ParticleSettings.SpawnMode == EGL_FootstepParticleSpawnMode::AtTraceHit)
	{
		const FQuat OffsetQuat = (Foot == EGL_FootBone::Left)
			? FQuat(ParticleSettings.LeftFootRotationOffsetQuat)
			: FQuat(ParticleSettings.RightFootRotationOffsetQuat);

		const FQuat  FXQuat = Rotation * OffsetQuat;
		const FVector FXLoc = Location + FXQuat.RotateVector(FVector(ParticleSettings.LocationOffset) * meshScaleZ);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), System, FXLoc, FXQuat.Rotator(),
			FVector(1.f, 1.f, 1.f) * meshScaleZ, true, true, ENCPoolMethod::AutoRelease);
	}
	else
	{
		const FName BoneName = (Foot == EGL_FootBone::Left) ? Settings->LeftFootBone : Settings->RightFootBone;

		UNiagaraFunctionLibrary::SpawnSystemAttached(System, MeshComp, BoneName,
			FVector(ParticleSettings.LocationOffset) * meshScaleZ,
			(Foot == EGL_FootBone::Left)
			? FRotator(ParticleSettings.LeftFootRotationOffset)
			: FRotator(ParticleSettings.RightFootRotationOffset),
			FVector(1.f, 1.f, 1.f) * meshScaleZ,
			EAttachLocation::KeepRelativeOffset, true, ENCPoolMethod::AutoRelease);
	}
}
