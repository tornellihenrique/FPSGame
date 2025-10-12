#include "Player/FPS_PlayerController.h"

#include "Character/FPS_Character.h"

AFPS_PlayerController::AFPS_PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void AFPS_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFPS_PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPS_PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFPS_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AFPS_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	MainCharacter = Cast<AFPS_Character>(InPawn);
	OnRep_MainCharacter();

	SpawnLoadout(LoadoutPresets);
}

void AFPS_PlayerController::SpawnLoadout(const FPlayerLoadout& PlayerLoadout)
{
	if (!MainCharacter) return;

	MainCharacter->PopulateLoadout(PlayerLoadout);
}

void AFPS_PlayerController::OnRep_MainCharacter()
{

}
