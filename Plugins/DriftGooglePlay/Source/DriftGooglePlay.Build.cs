// Copyright 2015-2017 Directive Games Limited - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class DriftGooglePlay : ModuleRules
{
	public DriftGooglePlay(ReadOnlyTargetRules TargetRules) : base(TargetRules)
    {
		bEnableShadowVariableWarnings = false;
		PCHUsage = PCHUsageMode.NoSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				"DriftGooglePlay/Public",		
			}
			);
				

		PrivateIncludePaths.AddRange(
			new string[] {
				"DriftGooglePlay/Private",
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
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "Drift",
            }
            );
    }
}
