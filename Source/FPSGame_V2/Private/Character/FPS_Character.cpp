#include "Character/FPS_Character.h"

#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Components/FPS_HealthComponent.h"
#include "Misc/GL_GameplayTags.h"
#include "Game/FPS_GameMode.h"
#include "Components/GE_EquipmentManagerComponent.h"
#include "Equipments/GE_Equipment.h"
#include "Misc/GE_EquipmentAnimData.h"
#include "Camera/FPS_CameraComponent.h"
#include "Animation/FPS_AnimInstance.h"

AFPS_Character::AFPS_Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->SetOwnerNoSee(true);

	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFP"));
	MeshFP->SetupAttachment(GetMesh());
	MeshFP->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshFP->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	MeshFP->SetOnlyOwnerSee(true);
	MeshFP->SetCanEverAffectNavigation(false);

	CameraComponent = CreateDefaultSubobject<UFPS_CameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(MeshFP);
	CameraComponent->SetRelativeRotation_Direct(FRotator(0.0f, 90.0f, 0.0f));

	EquipmentHolderTP = CreateDefaultSubobject<USceneComponent>(TEXT("EquipmentHolder"));
	EquipmentHolderTP->SetupAttachment(GetMesh(), TEXT("VB EquipmentPivot"));
	EquipmentHolderTP->SetRelativeRotation_Direct(FRotator(0.0f, 180.0f, 0.0f));

	EquipmentHolderFP = CreateDefaultSubobject<USceneComponent>(TEXT("EquipmentHolderFP"));
	EquipmentHolderFP->SetupAttachment(MeshFP, TEXT("VB EquipmentPivot"));
	EquipmentHolderFP->SetRelativeRotation_Direct(FRotator(0.0f, 180.0f, 0.0f));

	HealthComponent = CreateDefaultSubobject<UFPS_HealthComponent>(TEXT("HealthComponent"));

	EquipmentManager = CreateDefaultSubobject<UGE_EquipmentManagerComponent>(TEXT("EquipmentManager"));

	ViewMode = GameplayViewModeTags::FirstPerson;
}

void AFPS_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPS_Character, EquipmentsCount);
}

void AFPS_Character::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AFPS_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HealthComponent->OnDeathPayload.AddDynamic(this, &AFPS_Character::HandleDeathPayload);

	EquipmentManager->OnEquipmentChanged.AddDynamic(this, &AFPS_Character::OnEquipmentChanged);
	EquipmentManager->OnEquipmentAdded.AddDynamic(this, &AFPS_Character::OnEquipmentAdded);
	EquipmentManager->OnEquipmentRemoved.AddDynamic(this, &AFPS_Character::OnEquipmentRemoved);
}

void AFPS_Character::BeginPlay()
{
	Super::BeginPlay();
}

void AFPS_Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeathPayload.RemoveDynamic(this, &AFPS_Character::HandleDeathPayload);
	}

	if (IsValid(EquipmentManager))
	{
		EquipmentManager->OnEquipmentChanged.RemoveDynamic(this, &AFPS_Character::OnEquipmentChanged);
		EquipmentManager->OnEquipmentAdded.RemoveDynamic(this, &AFPS_Character::OnEquipmentAdded);
		EquipmentManager->OnEquipmentRemoved.RemoveDynamic(this, &AFPS_Character::OnEquipmentRemoved);
	}

	Super::EndPlay(EndPlayReason);
}

void AFPS_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AGE_Equipment* CurrentEquipment = GetCurrentEquipment())
	{
		if (const UGE_EquipmentAnimData* EquipmentAnimData = CurrentEquipment->AnimData)
		{
			AnimState.PoseFP = EquipmentAnimData->PoseFP;
			AnimState.PoseTP = EquipmentAnimData->PoseTP;

			AnimState.OriginRelativeRotation = EquipmentAnimData->OriginRelativeRotation;

			AnimState.RightHandOffset = EquipmentAnimData->RightHandOffset;
			AnimState.LeftHandOffset = EquipmentAnimData->LeftHandOffset;

			AnimState.RightClavicleOffset = EquipmentAnimData->RightClavicleOffset;
			AnimState.LeftClavicleOffset = EquipmentAnimData->LeftClavicleOffset;

			AnimState.RightLowerarmOffset = EquipmentAnimData->RightLowerarmOffset;
			AnimState.LeftLowerarmOffset = EquipmentAnimData->LeftLowerarmOffset;
		}

		AnimState.PivotOffset = -CurrentEquipment->GetPivotPoint();

		float TargetForceDisableRunningAlpha = 0.0f;
		if (!CurrentEquipment->CanRun())
		{
			TargetForceDisableRunningAlpha = 1.0f;
		}

		AnimState.ForceDisableRunningAlpha = FMath::FInterpTo(AnimState.ForceDisableRunningAlpha, TargetForceDisableRunningAlpha, DeltaTime, 15.0f);
	}
	else
	{
		AnimState.Reset();

		AnimState.PoseFP = EmptyPoseFP;
		AnimState.PoseTP = EmptyPoseTP;
	}
}

void AFPS_Character::BecomeViewTarget(APlayerController* PC)
{
	Super::BecomeViewTarget(PC);
}

void AFPS_Character::EndViewTarget(APlayerController* PC)
{
	Super::EndViewTarget(PC);
}

void AFPS_Character::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	if (CameraComponent && CameraComponent->IsActive())
	{
		FMinimalViewInfo ViewInfo;
		CameraComponent->GetViewInfo(ViewInfo);

		// ...

		OutResult = ViewInfo;

		return;
	}

	Super::CalcCamera(DeltaTime, OutResult);
}

void AFPS_Character::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	Super::GetActorEyesViewPoint(OutLocation, OutRotation);

	if (CameraComponent && CameraComponent->IsActive())
	{
		FMinimalViewInfo ViewInfo;
		CameraComponent->GetViewInfo(ViewInfo);

		OutLocation = ViewInfo.Location;
		OutRotation = ViewInfo.Rotation;
	}
}

void AFPS_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AFPS_Character::UnPossessed()
{
	Super::UnPossessed();
}

void AFPS_Character::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void AFPS_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AFPS_Character::Restart()
{
	Super::Restart();
}

void AFPS_Character::PawnClientRestart()
{
	Super::PawnClientRestart();
}

void AFPS_Character::NotifyControllerChanged()
{
	const APlayerController* PreviousPlayer = Cast<APlayerController>(PreviousController);
	if (PreviousPlayer && ::IsValid(PreviousPlayer))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer());
		if (InputSubsystem && ::IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}

	APlayerController* NewPlayer = Cast<APlayerController>(GetController());
	if (NewPlayer && ::IsValid(NewPlayer))
	{
		NewPlayer->InputYawScale_DEPRECATED = 1.0f;
		NewPlayer->InputPitchScale_DEPRECATED = 1.0f;
		NewPlayer->InputRollScale_DEPRECATED = 1.0f;

		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayer->GetLocalPlayer());
		if (InputSubsystem && ::IsValid(InputSubsystem))
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	Super::NotifyControllerChanged();
}

void AFPS_Character::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(Input))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Move);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Look);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Sprint);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Canceled, this, &AFPS_Character::Input_Sprint);
		EIC->BindAction(IA_Walk, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Walk);
		EIC->BindAction(IA_Crouch, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Crouch);
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Jump);
		EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &AFPS_Character::Input_Jump);
		EIC->BindAction(IA_View, ETriggerEvent::Triggered, this, &AFPS_Character::Input_View);
		EIC->BindAction(IA_Cycle, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Cycle);
		EIC->BindAction(IA_Slot, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Slot);
		EIC->BindAction(IA_Primary, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Primary);
		EIC->BindAction(IA_Secondary, ETriggerEvent::Triggered, this, &AFPS_Character::Input_Secondary);
	}
}

bool AFPS_Character::CanRun() const
{
	if (GetCurrentEquipment() && !GetCurrentEquipment()->CanRun())
	{
		return false;
	}

	return Super::CanRun();
}

void AFPS_Character::OnJumpedNetworked()
{
	Super::OnJumpedNetworked();

	if (UFPS_AnimInstance* AnimInstance = Cast<UFPS_AnimInstance>(MeshFP->GetAnimInstance()))
	{
		AnimInstance->Jump();
	}
}

void AFPS_Character::OnRep_ViewMode()
{
	Super::OnRep_ViewMode();

	if (AGE_Equipment* CurrentEquipment = GetCurrentEquipment())
	{
		CurrentEquipment->UpdateViewMode(IsOnFirstPersonView());
	}

	if (IsOnFirstPersonView())
	{
		GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
		GetMesh()->SetOwnerNoSee(true);

		MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
		MeshFP->SetVisibility(true);
	}
	else
	{
		GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;
		GetMesh()->SetOwnerNoSee(false);

		MeshFP->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
		MeshFP->SetVisibility(false);
	}
}

AGE_Equipment* AFPS_Character::GetCurrentEquipment() const
{
	return EquipmentManager ? EquipmentManager->GetCurrentEquipment() : nullptr;
}

bool AFPS_Character::ShouldUseRunFireDelay() const
{
	return LocomotionMode.MatchesTagExact(GameplayLocomotionModeTags::Grounded) &&
		(Gait.MatchesTagExact(GameplayGaitTags::Running) || Gait.MatchesTagExact(GameplayGaitTags::Sprinting));
}

void AFPS_Character::PopulateLoadout(const FPlayerLoadout& PlayerLoadout)
{
	if (!HasAuthority()) return;

	if (!PlayerLoadout.IsValid()) return;

	if (!EquipmentManager) return;

	TArray<TSoftClassPtr<AGE_Equipment>> CheckedClasses = PlayerLoadout.Equipments.FilterByPredicate([](const TSoftClassPtr<AGE_Equipment>& InSoftClass)
	{
		return !InSoftClass.IsNull();
	});

	TArray<TSubclassOf<AGE_Equipment>> ValidClasses;
	ValidClasses.Reserve(CheckedClasses.Num());

	for (const TSoftClassPtr<AGE_Equipment>& InSoftClass : CheckedClasses)
	{
		if (TSubclassOf<AGE_Equipment> EquipementClass = InSoftClass.LoadSynchronous())
		{
			ValidClasses.Add(EquipementClass);
		}
	}

	EquipmentsCount = ValidClasses.Num();
	EquipmentsSpawned = 0;

	if (EquipmentsCount <= 0) return;

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, -50000.0f), FVector::OneVector);

	for (int32 i = 0; i < EquipmentsCount; ++i)
	{
		TSubclassOf<AGE_Equipment> EquipmentClass = ValidClasses[i];
		if (!EquipmentClass) continue;

		AGE_Equipment* SpawnedEquipment = GetWorld()->SpawnActorDeferred<AGE_Equipment>(EquipmentClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (!SpawnedEquipment)
		{
			if (++EquipmentsSpawned == EquipmentsCount)
			{
				// #TODO
				return;
			}

			continue;
		}

		SpawnedEquipment->SetOwningCharacter(this);

		SpawnedEquipment->FinishSpawning(SpawnTransform);

		const int32 SlotIndex = EquipmentManager->AddEquipment(SpawnedEquipment, i);
		if (SlotIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to add equipment %s to slot %i"), *GetNameSafe(SpawnedEquipment), SlotIndex);
		}

		if (++EquipmentsSpawned == EquipmentsCount)
		{
			// #TODO
			return;
		}
	}
}

void AFPS_Character::HandleDeathPayload(const FDeathEventPayload& Data)
{
	if (AFPS_GameMode* GM = GetWorld()->GetAuthGameMode<AFPS_GameMode>())
	{
		// FKillEventData KillData(Data.DamageType, Data.DamageCauser, Data.bHeadshot, Data.FinalDamage);
		// GM->OnCharacterDied(this, Data.InstigatorController, KillData);
	}

	if (GetNetMode() == NM_DedicatedServer)
	{
		FTimerHandle StopTickTh;
		GetWorldTimerManager().SetTimer(StopTickTh, FTimerDelegate::CreateWeakLambda(this, [this]
		{
			SetActorTickEnabled(false);
		}), 5.f, false);
	}

	NetHandleDeathPayload(Data);
}

void AFPS_Character::NetHandleDeathPayload_Implementation(const FDeathEventPayload& Data)
{
	if (IsLocallyControlled()) { SetViewModeTag(GameplayViewModeTags::ThirdPerson); }

	StartRagdollingImplementation();

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (!Data.Impulse.IsNearlyZero() && MeshComp->IsSimulatingPhysics(Data.BoneName))
		{
			MeshComp->AddImpulseAtLocation(Data.Impulse * MeshComp->GetMass(), Data.ImpulseAt, Data.BoneName);
		}
	}
}

void AFPS_Character::OnEquipmentChanged(AGE_Equipment* NewEquipment, AGE_Equipment* OldEquipment)
{
	if (OldEquipment)
	{
		OldEquipment->UpdateViewMode(false);
	}

	if (NewEquipment)
	{
		if (const UGE_EquipmentAnimData* EquipmentAnimData = NewEquipment->AnimData)
		{
			SetOverlayModeTag(EquipmentAnimData->OverlayMode);
		}

		NewEquipment->UpdateViewMode(IsOnFirstPersonView());
	}
}

void AFPS_Character::OnEquipmentAdded(AGE_Equipment* Equipment, int32 SlotIndex)
{

}

void AFPS_Character::OnEquipmentRemoved(AGE_Equipment* Equipment, int32 SlotIndex)
{

}

void AFPS_Character::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller && Axis.SizeSquared() > 0.f)
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector  Fwd = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector  Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		AddMovementInput(Fwd, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void AFPS_Character::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void AFPS_Character::Input_Sprint(const FInputActionValue& ActionValue)
{
	SetDesiredGait(ActionValue.Get<bool>() ? GameplayGaitTags::Running : GameplayGaitTags::Walking);
}

void AFPS_Character::Input_Walk()
{
	if (DesiredGait == GameplayGaitTags::Walking)
	{
		SetDesiredGait(GameplayGaitTags::Running);
	}
	else if (DesiredGait == GameplayGaitTags::Running)
	{
		SetDesiredGait(GameplayGaitTags::Walking);
	}
}

void AFPS_Character::Input_Crouch()
{
	if (DesiredStance == GameplayStanceTags::Standing)
	{
		SetDesiredStance(GameplayStanceTags::Crouching);
	}
	else if (DesiredStance == GameplayStanceTags::Crouching)
	{
		SetDesiredStance(GameplayStanceTags::Standing);
	}
}

void AFPS_Character::Input_Jump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		if (Stance == GameplayStanceTags::Crouching)
		{
			SetDesiredStance(GameplayStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void AFPS_Character::Input_View()
{
	SetViewModeTag(ViewMode == GameplayViewModeTags::ThirdPerson ? GameplayViewModeTags::FirstPerson : GameplayViewModeTags::ThirdPerson);
}

void AFPS_Character::Input_Cycle(const FInputActionValue& ActionValue)
{
	if (EquipmentManager) EquipmentManager->Cycle(ActionValue.Get<float>() > 0.f ? +1 : -1);
}

void AFPS_Character::Input_Slot(const FInputActionValue& ActionValue)
{
	if (EquipmentManager) EquipmentManager->EquipSlot(ActionValue.Get<float>() - 1);
}

void AFPS_Character::Input_Primary(const FInputActionValue& ActionValue)
{
	if (AGE_Equipment* Eq = EquipmentManager ? EquipmentManager->GetCurrentEquipment() : nullptr)
	{
		Eq->PrimaryAction(ActionValue.Get<bool>());
	}
}

void AFPS_Character::Input_Secondary(const FInputActionValue& ActionValue)
{
	if (AGE_Equipment* Eq = EquipmentManager ? EquipmentManager->GetCurrentEquipment() : nullptr)
	{
		Eq->SecondaryAction(ActionValue.Get<bool>());
	}
}

bool AFPS_Character::IsOnFirstPersonView() const
{
	if (IsLocallyControlled() && IsPlayerControlled())
	{
		return ViewMode == GameplayViewModeTags::FirstPerson;
	}

	return false;
}
