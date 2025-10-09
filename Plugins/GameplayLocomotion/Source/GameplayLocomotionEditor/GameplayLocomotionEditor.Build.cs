// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayLocomotionEditor : ModuleRules
{
	public GameplayLocomotionEditor (ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "AnimGraphRuntime", "AnimationModifiers", "AnimationBlueprintLibrary", "GameplayLocomotion"
        });

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new[]
            {
                "AnimGraph"
            });

            PrivateDependencyModuleNames.AddRange(new[]
            {
                "BlueprintGraph"
            });
        }
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
