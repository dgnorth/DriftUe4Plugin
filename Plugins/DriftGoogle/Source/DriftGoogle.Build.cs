// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class DriftGoogle : ModuleRules
{
	public DriftGoogle(ReadOnlyTargetRules TargetRules) : base(TargetRules)
    {
		bEnableShadowVariableWarnings = false;
		PCHUsage = PCHUsageMode.NoSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				"DriftGoogle/Public",		
			}
			);
				

		PrivateIncludePaths.AddRange(
			new string[] {
				"DriftGoogle/Private",
			}
			);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Projects",
			}
			);

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "HTTP",
                "Json",
                "JsonUtilities",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Drift",
            }
            );
    }
}
