// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class DriftOculus : ModuleRules
{
	public DriftOculus(TargetInfo Target)
	{
		bEnableShadowVariableWarnings = false;
		PCHUsage = PCHUsageMode.NoSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				"DriftOculus/Public",		
			}
			);
				

		PrivateIncludePaths.AddRange(
			new string[] {
				"DriftOculus/Private",
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
                "Engine",
                "Drift",
                "OnlineSubsystem",
                "OnlineSubsystemOculus",
                "LibOVRPlatform",
            }
            );
    }
}
