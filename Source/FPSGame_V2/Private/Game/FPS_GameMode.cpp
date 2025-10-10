#include "Game/FPS_GameMode.h"

AFPS_GameMode::AFPS_GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void AFPS_GameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AFPS_GameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPS_GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AFPS_GameMode::StartPlay()
{
	Super::StartPlay();
}

APlayerController* AFPS_GameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	return Super::SpawnPlayerController(InRemoteRole, Options);
}

void AFPS_GameMode::PostLogin(class APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

bool AFPS_GameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
}

AActor* AFPS_GameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName /*= TEXT("")*/)
{
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

void AFPS_GameMode::CalcDamage(float& OutDamageAmount, AController* Controller, AController* OtherController)
{
	// #TODO: Finish
}