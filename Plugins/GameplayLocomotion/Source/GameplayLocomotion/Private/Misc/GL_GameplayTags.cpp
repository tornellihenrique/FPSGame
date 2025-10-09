#include "Misc/GL_GameplayTags.h"

namespace GameplayViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, FName(TEXTVIEW("Gameplay.ViewMode.FirstPerson")))
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, FName(TEXTVIEW("Gameplay.ViewMode.ThirdPerson")))
}

namespace GameplayLocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName(TEXTVIEW("Gameplay.LocomotionMode.Grounded")))
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName(TEXTVIEW("Gameplay.LocomotionMode.InAir")))
}

namespace GameplayStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName(TEXTVIEW("Gameplay.Stance.Standing")))
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName(TEXTVIEW("Gameplay.Stance.Crouching")))
}

namespace GameplayGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName(TEXTVIEW("Gameplay.Gait.Walking")))
	UE_DEFINE_GAMEPLAY_TAG(Running, FName(TEXTVIEW("Gameplay.Gait.Running")))
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName(TEXTVIEW("Gameplay.Gait.Sprinting")))
}

namespace GameplayOverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Default, FName(TEXTVIEW("Gameplay.OverlayMode.Default")))
	UE_DEFINE_GAMEPLAY_TAG(Rifle, FName(TEXTVIEW("Gameplay.OverlayMode.Rifle")))
	UE_DEFINE_GAMEPLAY_TAG(Pistol, FName(TEXTVIEW("Gameplay.OverlayMode.Pistol")))
}

namespace GameplayLocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ragdolling, FName(TEXTVIEW("Gameplay.LocomotionAction.Ragdolling")))
}