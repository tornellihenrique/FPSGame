// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayLocomotion : ModuleRules
{
	public GameplayLocomotion (ReadOnlyTargetRules Target) : base(Target)
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
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"Engine",
                "GameplayTags",
                "AnimGraphRuntime",
				"RigVM",
				"ControlRig",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "NetCore",
                "PhysicsCore",
                "EnhancedInput",
                "Niagara",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...
			}
			);

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new[]
            {
                "MessageLog"
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
