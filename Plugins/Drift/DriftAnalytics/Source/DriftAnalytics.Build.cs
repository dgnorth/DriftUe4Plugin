// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

using UnrealBuildTool;

public class DriftAnalytics : ModuleRules
{
	public DriftAnalytics(ReadOnlyTargetRules TargetRules) : base(TargetRules)
    {
		bFasterWithoutUnity = true;
		bEnableShadowVariableWarnings = false;
		PCHUsage = PCHUsageMode.NoSharedPCHs;

		
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
				// ... add other public dependencies that you statically link with here ...
				"Core",
                "CoreUObject",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
				"Analytics",
                "Engine",
                "Drift",
			}
			);
		
        PublicIncludePathModuleNames.Add("Analytics");

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
