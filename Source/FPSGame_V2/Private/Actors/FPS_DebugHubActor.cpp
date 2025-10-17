#include "Actors/FPS_DebugHubActor.h"

#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

#if WITH_IMGUI
#include <imgui.h>
#include "ImGuiDelegates.h"
#endif
#include "GameFramework/CharacterMovementComponent.h"

static TAutoConsoleVariable<int32> CVarDebugHub(
	TEXT("Debug.Hub"),
	0,
	TEXT("Debug hub (ImGui)\n<=0: off, 1: on"),
	ECVF_Default);

AFPS_DebugHubActor::AFPS_DebugHubActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITOR
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent && !IsRunningCommandlet())
	{
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> Icon;
			FName Cat; FText Name;
			FConstructorStatics()
				: Icon(TEXT("/Engine/EditorResources/S_Player.S_Player"))
				, Cat(TEXT("DebugHubActor"))
				, Name(NSLOCTEXT("SpriteCategory", "FPS_DebugHubActor", "DebugHubActor"))
			{
			}
		};
		static FConstructorStatics CS;
		SpriteComponent->Sprite = CS.Icon.Get();
#if WITH_EDITORONLY_DATA
		SpriteComponent->SpriteInfo.Category = CS.Cat;
		SpriteComponent->SpriteInfo.DisplayName = CS.Name;
#endif
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SetRelativeScale3D(FVector(0.5f));
		SpriteComponent->SetupAttachment(RootComponent);
	}
#endif

	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetActorTickEnabled(false);
}

#if WITH_EDITOR
void AFPS_DebugHubActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AFPS_DebugHubActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AFPS_DebugHubActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFPS_DebugHubActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AFPS_DebugHubActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFPS_DebugHubActor::BeginPlay()
{
	Super::BeginPlay();

#if WITH_IMGUI
	if (UWorld* World = GetWorld())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (PC->IsLocalController())
				{
					CachedGI = World->GetGameInstance();
					CachedPC = PC;
					CachedPS = PC->PlayerState;
					CachedChar = PC->GetCharacter();
					CachedGS = World->GetGameState();

					BuildSections();

					FSimpleMulticastDelegate& D = FImGuiDelegates::OnWorldDebug(World);
					DebugHandle = D.AddUObject(this, &AFPS_DebugHubActor::OnWorldDebug);
				}
			}
		}
	}
#endif
}

void AFPS_DebugHubActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if WITH_IMGUI
	if (DebugHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			FImGuiDelegates::OnWorldDebug(World).Remove(DebugHandle);
		}

		DebugHandle.Reset();
	}
#endif

	Super::EndPlay(EndPlayReason);
}

void AFPS_DebugHubActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_IMGUI
void AFPS_DebugHubActor::OnWorldDebug()
{
	if (CVarDebugHub.GetValueOnGameThread() != 1) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController()) return;

	CachedPC = PC;
	CachedPS = PC->PlayerState;
	CachedChar = PC->GetCharacter();
	CachedGS = World->GetGameState();
	CachedGI = World->GetGameInstance();

	// static bool bShowDemoWindow = true;
	// ImGui::ShowDemoWindow(&bShowDemoWindow);

	static float HubFontScale = 1.4f;
	if (ImGui::Begin("Debug Hub", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowFontScale(HubFontScale);

		if (ImGui::CollapsingHeader("Hub Options"))
		{
			ImGui::SliderFloat("Font scale", &HubFontScale, 0.6f, 2.0f, "%.2f");
			
			for (int32 i = 0; i < Sections.Num(); ++i)
			{
				ImGui::Checkbox(TCHAR_TO_ANSI(*FString::Printf(TEXT("Enable: %s"), *Sections[i].Label)), &Sections[i].bEnabled);
			}
		}

		for (FZoDbgSection& S : Sections)
		{
			if (!S.bEnabled) continue;

			if (ImGui::CollapsingHeader(TCHAR_TO_ANSI(*S.Label)))
			{
				if (S.Draw) S.Draw();
			}
		}
	}

	ImGui::End();
}

void AFPS_DebugHubActor::BuildSections()
{
	AddSection(TEXT("Context"), [this]() {
		UWorld* World = GetWorld();
		if (!World) { ImGui::TextDisabled("No world."); return; }

		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (!PC->IsLocalController()) { ImGui::TextDisabled("Not local."); return; }

			CachedPC = PC;
			CachedPS = PC->PlayerState;
			CachedChar = PC->GetCharacter();
			CachedGS = World->GetGameState();
			CachedGI = World->GetGameInstance();
		}

		const int32 NM = (int32) World->GetNetMode();
		ImGui::Text("NetMode: %d", NM);

		if (APlayerController* PC = CachedPC.Get())
		{
			ImGui::Text("PC: %s", TCHAR_TO_ANSI(*PC->GetName()));
		}

		if (APlayerState* PS = CachedPS.Get())
		{
			ImGui::Text("PlayerState: %s", TCHAR_TO_ANSI(*PS->GetPlayerName()));
			ImGui::Text("Ping: %.1f", PS->GetPingInMilliseconds());
		}

		if (ACharacter* C = CachedChar.Get())
		{
			const FVector L = C->GetActorLocation();
			const FRotator R = C->GetActorRotation();
			const float Spd = C->GetVelocity().Size2D();
			ImGui::Text("Character: %s  (%.1f, %.1f, %.1f) (%.1f, %.1f, %.1f)  Speed2D=%.1f", TCHAR_TO_ANSI(*C->GetName()), L.X, L.Y, L.Z, R.Pitch, R.Yaw, R.Roll, Spd);

			if (UCharacterMovementComponent* MovementComponent = C->GetCharacterMovement())
			{
				static int Samples = 50;
				static TArray<float> InputX;
				static TArray<float> InputY;

				if (InputX.IsValidIndex(Samples)) { InputX.RemoveAt(Samples); }
				if (InputY.IsValidIndex(Samples)) { InputY.RemoveAt(Samples); }

				const FVector& LastInputVector = MovementComponent->GetLastInputVector();

				InputX.Insert(LastInputVector.X, 0);
				InputY.Insert(LastInputVector.Y, 0);

				static float InputXArr[10];
				static float InputYArr[10];

				if (int32 N = InputX.Num()) { FMemory::Memcpy(InputXArr, InputX.GetData(), N * sizeof(float)); }
				if (int32 N = InputY.Num()) { FMemory::Memcpy(InputYArr, InputY.GetData(), N * sizeof(float)); }

				ImGui::PlotLines(TCHAR_TO_ANSI(*FString::Printf(TEXT("Input Vector X: %.1f"), LastInputVector.X)), InputXArr, Samples, 0, NULL, -1, 1, ImVec2(0, 0), sizeof(float));
				ImGui::PlotLines(TCHAR_TO_ANSI(*FString::Printf(TEXT("Input Vector Y: %.1f"), LastInputVector.Y)), InputYArr, Samples, 0, NULL, -1, 1, ImVec2(0, 0), sizeof(float));
			}
		}

		if (AGameStateBase* GS = CachedGS.Get())
		{
			ImGui::Text("Players: %d", GS->PlayerArray.Num());
		}

		ImGui::Text("RealTimeSec: %.1f", World->GetRealTimeSeconds());
	});
}

void AFPS_DebugHubActor::AddSection(const FString& Label, TFunction<void()> InDraw)
{
	FZoDbgSection S;
	S.Label = Label;
	S.Draw = MoveTemp(InDraw);
	Sections.Add(S);
}
#endif
