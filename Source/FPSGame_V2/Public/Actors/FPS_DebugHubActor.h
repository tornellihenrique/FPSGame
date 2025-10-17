#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/FPS_Types.h"
#include "FPS_DebugHubActor.generated.h"

class UBillboardComponent;

struct FZoDbgSection
{
	FString Label;
	TFunction<void()> Draw;
	bool bEnabled = true;
};

UCLASS()
class FPSGAME_V2_API AFPS_DebugHubActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

public:
	AFPS_DebugHubActor(const FObjectInitializer& ObjectInitializer);

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
	//~ End of AActor

private:
#if WITH_IMGUI
	void OnWorldDebug();

	void BuildSections();
	void AddSection(const FString& Label, TFunction<void()> InDraw);

	// context cache
	TWeakObjectPtr<class UGameInstance> CachedGI;
	TWeakObjectPtr<class APlayerController> CachedPC;
	TWeakObjectPtr<class ACharacter> CachedChar;
	TWeakObjectPtr<class APlayerState> CachedPS;
	TWeakObjectPtr<class AGameStateBase> CachedGS;

	TArray<FZoDbgSection> Sections;
	FDelegateHandle DebugHandle;
#endif
};
