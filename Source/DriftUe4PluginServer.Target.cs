// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
 
using UnrealBuildTool;
using System.Collections.Generic;
 
public class DriftUe4PluginServerTarget : TargetRules
{
    public DriftUe4PluginServerTarget(TargetInfo Target)
    {
        Type = TargetType.Server;
    }
 
    //
    // TargetRules interface.
    //
    public override void SetupBinaries(
        TargetInfo Target,
        ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
        ref List<string> OutExtraModuleNames
        )
    {
        base.SetupBinaries(Target, ref OutBuildBinaryConfigurations, ref OutExtraModuleNames);
        OutExtraModuleNames.Add("DriftUe4Plugin");
    }
}
