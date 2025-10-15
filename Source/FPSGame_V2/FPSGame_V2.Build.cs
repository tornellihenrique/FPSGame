// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FPSGame_V2 : ModuleRules
{
	public FPSGame_V2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Uncomment if you are not using Public/Private folders
        //         PublicIncludePaths.AddRange(new string[]
        //         {
        //             "FPSGame_V2"
        //         });

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayTags",
            "AIModule",
            "UMG",
            "CoreOnline",
            "DeveloperSettings",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "NetCore",
            "PhysicsCore",
            "Niagara",
            "EngineSettings",
        });

        PublicDependencyModuleNames.AddRange(new string[] {
            "GameplayEquipments",
            "GameplayAnimation",
            "GameplayLocomotion",
            "GameplayLocomotionCamera"
        });
    }
}
