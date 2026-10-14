// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DungeonCore : ModuleRules
{
	public DungeonCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"SlateCore",
			"Landscape"
		});
	}
}
