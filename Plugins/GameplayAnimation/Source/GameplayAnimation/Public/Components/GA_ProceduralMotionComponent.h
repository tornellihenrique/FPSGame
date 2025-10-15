#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/GA_Types.h"
#include "GA_ProceduralMotionComponent.generated.h"

UCLASS(Abstract)
class GAMEPLAYANIMATION_API UGA_ProceduralMotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGA_ProceduralMotionComponent(const FObjectInitializer& ObjectInitializer);

	//~ UActorComponent
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End of UActorComponent

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion")
	EGA_ProcStage Stage = EGA_ProcStage::Additive;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion")
	bool bApplyInFirstPerson = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Procedural Motion")
	bool bApplyInThirdPerson = true;

	virtual void UpdateOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input);

	const TArray<FGA_ProceduralOutput>& GetCachedOutputs() const { return CachedOps; }

protected:
	virtual bool ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const
	{
		Out = FGA_ProceduralOutput{};
		return false;
	}

	virtual void ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const;

private:
	TArray<FGA_ProceduralOutput> CachedOps;

};
