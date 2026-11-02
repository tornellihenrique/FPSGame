# FPSGame

First- and third-person FPS base in Unreal Engine 5.6 C++, built live over seven free streams in Portuguese. It is the course project rather than a finished game, and its main piece is the procedural weapon animation in `GameplayAnimation`.

## GameplayAnimation

A custom animation node composes weapon motion from components, so a new effect needs no change to the Animation Blueprint.

1. On the game thread, [`FGA_AnimNode_ProceduralMotion`](Plugins/GameplayAnimation/Source/GameplayAnimation/Public/Nodes/GA_AnimNode_ProceduralMotion.h) finds every [`UGA_ProceduralMotionComponent`](Plugins/GameplayAnimation/Source/GameplayAnimation/Public/Components/GA_ProceduralMotionComponent.h) on the owning actor.
2. Each component outputs a transform, with its weight, stage, space and composition mode.
3. On the animation worker, the node applies the outputs in a fixed order: PreOffset, Offset, ADS, Additive, Sway, Recoil and Post.

The plugin includes Offset, ADS, Additive and Sway components. Sway uses springs per axis, with settings in a data asset.

A new effect is a new component:

```cpp
UCLASS()
class UMyBreathingComponent : public UGA_ProceduralMotionComponent
{
	GENERATED_BODY()

protected:
	virtual bool ComputeOutput(float DeltaSeconds, const UAnimInstance* AnimInstance, const FGA_ProceduralInput& Input, FGA_ProceduralOutput& Out) const override
	{
		Out.Stage = EGA_ProcStage::Additive;
		Out.Transform.SetLocation(FVector(0.f, 0.f, FMath::Sin(GetWorld()->GetTimeSeconds()) * 0.2f));
		return true;
	}
};
```

Components can also be limited to first or third person through `bApplyInFirstPerson` and `bApplyInThirdPerson`.

## Other systems

- **[`GameplayEquipments`](Plugins/GameplayEquipments):** equipment slots and a server-driven swap state machine, timed by montages and with cancellation. `AGE_FireWeapon` adds hitscan fire with semi, burst and auto modes, ADS, spread, reload and quantized shots.
- **[`Source/FPSGame_V2`](Source/FPSGame_V2):** dual-mesh character using the engine's First Person Rendering, damage per bone through the hierarchy, spawn protection, ragdoll death and an ImGui debug hub.

## Networking

- Stance, gait, view, overlay and aim apply on the owner at once and reach the server through RPCs.
- Shots play effects on the owner and call `ServerApplyShot` with quantized data. Remote clients play the effects in `OnRep`.
- Equipment uses replicated state, with multicasts for the visual side.

## Live streams

All in Portuguese, about 22 hours in total.

| # | Topic | Date |
|---|---|---|
| 1 | [Setup, template and locomotion](https://www.youtube.com/watch?v=F5eLJ72xXeY) | 2025-10-08 |
| 2 | [Blueprint and C++ review, first-person animation setup](https://www.youtube.com/watch?v=7gN8jTgqsdA) | 2025-10-09 |
| 3 | [First-person animation setup, equipment plugin](https://www.youtube.com/watch?v=_ez9OVcaNFI) | 2025-10-10 |
| 4 | [First- and third-person animation, C++ animation node](https://www.youtube.com/watch?v=Dka7bb88ntI) | 2025-10-11 |
| 5 | [C++ animation node](https://www.youtube.com/watch?v=uLUP_sEpQoQ) | 2025-10-14 |
| 6 | [Procedural weapon sway component](https://www.youtube.com/watch?v=zBvrt1-AS7E) | 2025-10-16 |
| 7 | [Fire weapon class](https://www.youtube.com/watch?v=Y7gQJnzoITQ) | 2025-10-18 |

## Run

1. Install [Git LFS](https://git-lfs.com) before cloning, because `Content` is stored there.
2. Open `FPSGame_V2.uproject` in Unreal Engine 5.6 and build when asked.
3. Press Play.

## Credits

- `Plugins/GameplayLocomotion` derives from [ALS-Refactored](https://github.com/Sixze/ALS-Refactored), under the MIT License. See its [license notice](Plugins/GameplayLocomotion/LICENSE.md).
- `Plugins/UnrealImGui` is the Unreal ImGui plugin, under the MIT License. See its [license](Plugins/UnrealImGui/LICENSE).

## Author

Henrique Tornelli, Senior Gameplay Programmer. Portfolio: [tornellihenrique.github.io](https://tornellihenrique.github.io/)
