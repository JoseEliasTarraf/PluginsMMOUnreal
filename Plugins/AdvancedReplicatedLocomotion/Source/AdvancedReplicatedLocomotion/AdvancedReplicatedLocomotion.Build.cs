// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class AdvancedReplicatedLocomotion : ModuleRules
{
	public AdvancedReplicatedLocomotion(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine",
            "InputCore", "EnhancedInput",
            "AnimGraphRuntime",
            "Niagara","PhysicsCore","GameplayTags","GameplayAbilities","GameplayTasks"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate", "SlateCore"
        });

        // PublicDependencyModuleNames.Add("DeveloperSettings");

        // PublicDependencyModuleNames.AddRange(new[] {"GameplayTags","GameplayAbilities","GameplayTasks"});

        // PublicDependencyModuleNames.AddRange(new[] {"ControlRig","IKRig"});
    }
}
