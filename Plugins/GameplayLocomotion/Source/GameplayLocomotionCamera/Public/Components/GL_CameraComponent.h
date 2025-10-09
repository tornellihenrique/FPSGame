#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GL_CameraComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(GameplayCamera), meta=(BlueprintSpawnableComponent), HideCategories=(ComponentTick,Clothing,Physics,MasterPoseComponent,Collision,AnimationRig,Lighting,Deformer,Rendering,PathTracing,HLOD,Navigation,VirtualTexture,SkeletalMesh,LeaderPoseComponent,Optimization,LOD, MaterialParameters,TextureStreaming,Mobile,RayTracing))
class GAMEPLAYLOCOMOTIONCAMERA_API UGL_CameraComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UGL_CameraComponent(const FObjectInitializer& ObjectInitializer);

	// --- Global ---
	UPROPERTY(EditAnywhere, Category="Camera|General")
	uint8 bIgnoreTimeDilation : 1 { true };

	// Teleport camera if pivot jumps more than this in 1 frame (0 = disabled)
	UPROPERTY(EditAnywhere, Category="Camera|General", meta=(ClampMin=0, ForceUnits="cm"))
	float CameraTeleportDistanceThreshold = 200.f;

	// --- First person ---
	UPROPERTY(EditAnywhere, Category="Camera|FirstPerson", meta=(ClampMin=5, ClampMax=175, ForceUnits="deg"))
	float FirstPersonFOV = 90.f;

	UPROPERTY(EditAnywhere, Category="Camera|FirstPerson")
	FName FirstPersonSocketName = TEXT("FirstPersonCamera");

	// --- Third person ---
	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson", meta=(ClampMin=5, ClampMax=175, ForceUnits="deg"))
	float ThirdPersonFOV = 90.f;

	// Average of the two defines the pivot target
	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	FName FirstPivotSocketName = TEXT("root"); // fallback if detached

	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	FName SecondPivotSocketName = TEXT("head");

	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson", meta=(ClampMin=0, ForceUnits="cm"))
	float TraceRadius = 15.f;

	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// The trace start socket (single side; no shoulder switch)
	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	FName TraceSocketName = TEXT("ThirdPersonTrace");

	// Optional extra offset applied to the trace start when a curve wants override
	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	FVector TraceOverrideOffset = FVector(0.f, 0.f, 40.f);

	// Distance smoothing for camera collision trace
	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson")
	uint8 bEnableTraceDistanceSmoothing : 1 { true };

	UPROPERTY(EditAnywhere, Category="Camera|ThirdPerson", meta=(ClampMin=0, ForceUnits="s"))
	float TraceDistanceHalfLife = 0.2f;

	// --- FOV override / PostProcess ---
	UPROPERTY(EditAnywhere, Category="Camera|Effects")
	uint8 bOverrideFOV : 1;

	UPROPERTY(EditAnywhere, Category="Camera|Effects", meta=(ClampMin=5, ClampMax=175, EditCondition="bOverrideFOV", ForceUnits="deg"))
	float FOVOverride = 90.f;

	UPROPERTY(EditAnywhere, Category="Camera|Effects", meta=(ClampMin=0, ClampMax=1))
	float PostProcessWeight = 0.f;

	UPROPERTY(EditAnywhere, Category="Camera|Effects")
	FPostProcessSettings PostProcess;

	// --- Curve names (Animation-Driven Camera) ---
	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_FirstPersonOverride = TEXT("FirstPersonOverride");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_RotationLag = TEXT("RotationLag");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_LocationLagX = TEXT("LocationLagX");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_LocationLagY = TEXT("LocationLagY");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_LocationLagZ = TEXT("LocationLagZ");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_PivotOffsetX = TEXT("PivotOffsetX");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_PivotOffsetY = TEXT("PivotOffsetY");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_PivotOffsetZ = TEXT("PivotOffsetZ");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_CameraOffsetX = TEXT("CameraOffsetX");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_CameraOffsetY = TEXT("CameraOffsetY");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_CameraOffsetZ = TEXT("CameraOffsetZ");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_FovOffset = TEXT("FovOffset");

	UPROPERTY(EditAnywhere, Category="Camera|Curves")
	FName Curve_TraceOverride = TEXT("TraceOverride");

public:
	UFUNCTION(BlueprintPure, Category="GameplayCamera")
	virtual void GetViewInfo(FMinimalViewInfo& ViewInfo) const;

	UFUNCTION(BlueprintPure, Category="GameplayCamera", meta=(ReturnDisplayName="Location"))
	virtual FVector GetFirstPersonCameraLocation() const;

	UFUNCTION(BlueprintPure, Category="GameplayCamera", meta=(ReturnDisplayName="Pivot"))
	virtual FVector GetThirdPersonPivotLocation() const;

	UFUNCTION(BlueprintPure, Category="GameplayCamera", meta=(ReturnDisplayName="TraceStart"))
	virtual FVector GetThirdPersonTraceStartLocation() const;

public: // USkeletalMeshComponent overrides
	virtual void PostLoad() override;
	virtual void OnRegister() override;
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	virtual void Activate(bool bReset) override;
	virtual void InitAnim(bool bForceReinitialize) override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void CompleteParallelAnimationEvaluation(bool bDoPostAnimationEvaluation) override;

protected:
	virtual void TickCamera(float DeltaTime, bool bAllowLag = true);

	virtual FRotator CalculateCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const;
	virtual FVector CalculatePivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const;
	virtual FVector CalculatePivotOffset() const;
	virtual FVector CalculateCameraOffset() const;
	virtual float CalculateFovOffset() const;

	virtual FVector CalculateCameraTrace(const FVector& CameraTargetLocation, const FVector& PivotOffset, float DeltaTime, bool bAllowLag, float& OutTraceDistanceRatio) const;

	bool TryAdjustLocationBlockedByGeometry(FVector& Location) const;

	// Helpers
	float GetCurve(const FName& Name) const;

protected:
	// State
	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	TObjectPtr<ACharacter> Character;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	TWeakObjectPtr<UAnimInstance> AnimationInstance;

	UPROPERTY(VisibleAnywhere, Transient, Category="State", meta=(ForceUnits="x"))
	float PreviousGlobalTimeDilation = 1.f;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FVector PivotTargetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FVector PivotLagLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FVector PivotLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FVector CameraLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FRotator CameraRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	float TraceDistanceRatio = 1.f;

	UPROPERTY(VisibleAnywhere, Transient, Category="State", meta=(ClampMin=5, ClampMax=175, ForceUnits="deg"))
	float CameraFOV = 90.f;

	// Movement base tracking
	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	TObjectPtr<UPrimitiveComponent> MovementBasePrimitive;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FName MovementBaseBoneName;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FVector PivotMovementBaseRelativeLagLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Transient, Category="State")
	FQuat CameraMovementBaseRelativeRotation = FQuat::Identity;

		
};
