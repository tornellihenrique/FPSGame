#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPS_GameMode.generated.h"

UCLASS()
class FPSGAME_V2_API AFPS_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPS_GameMode(const FObjectInitializer& ObjectInitializer);

	//~ AActor
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~ End of AActor

	//~ AGameModeBase
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	virtual void PostLogin(class APlayerController* NewPlayer) override;
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;
	//~ End of AGameModeBase
	
};
