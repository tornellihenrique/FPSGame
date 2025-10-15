#include "Components/GA_ProceduralMotionComponent.h"

UGA_ProceduralMotionComponent::UGA_ProceduralMotionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGA_ProceduralMotionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UGA_ProceduralMotionComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UGA_ProceduralMotionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGA_ProceduralMotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UGA_ProceduralMotionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGA_ProceduralMotionComponent::UpdateOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input)
{
	check(IsInGameThread());

	CachedOps.Reset();

	if (!bEnabled)
	{
		return;
	}

	if ((!bApplyInFirstPerson && Input.bFirstPerson) || (!bApplyInThirdPerson && !Input.bFirstPerson))
	{
		return;
	}

	TArray<FGA_ProceduralOutput> Temp;
	ComputeOutputs(DeltaSeconds, AnimInstance, Input, Temp);

	for (FGA_ProceduralOutput& O : Temp)
	{
		if (O.Stage == EGA_ProcStage::Count)
		{
			O.Stage = Stage;
		}

		CachedOps.Emplace(MoveTemp(O));
	}
}

void UGA_ProceduralMotionComponent::ComputeOutputs(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, TArray<FGA_ProceduralOutput>& Outs) const
{
	FGA_ProceduralOutput Single;
	if (ComputeOutput(DeltaSeconds, AnimInstance, Input, Single))
	{
		Outs.Add(Single);
	}
}
