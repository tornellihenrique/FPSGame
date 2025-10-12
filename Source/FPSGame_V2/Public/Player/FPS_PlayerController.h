#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FPS_Types.h"
#include "FPS_PlayerController.generated.h"

class AFPS_Character;

UCLASS()
class FPSGAME_V2_API AFPS_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	FPlayerLoadout LoadoutPresets;

public:
	AFPS_PlayerController(const FObjectInitializer& ObjectInitializer);
	
	//~ AActor
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End of AActor

	//~ AController
	virtual void OnPossess(APawn* InPawn) override;
	//~ End of AController

protected:
	UPROPERTY(BlueprintReadOnly, Category="State", ReplicatedUsing=OnRep_MainCharacter)
	TObjectPtr<AFPS_Character> MainCharacter;

public:
	virtual void SpawnLoadout(const FPlayerLoadout& PlayerLoadout);

protected:
	UFUNCTION()
	virtual void OnRep_MainCharacter();
	
};
