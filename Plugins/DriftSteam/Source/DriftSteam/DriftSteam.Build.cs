// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class DriftSteam : ModuleRules
{
	public DriftSteam(TargetInfo Target)
	{
		bEnableShadowVariableWarnings = false;
		PCHUsage = PCHUsageMode.NoSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				"DriftSteam/Public",		
			}
			);
				

		PrivateIncludePaths.AddRange(
			new string[] {
				"DriftSteam/Private",
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
