// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
 
using UnrealBuildTool;
using System.Collections.Generic;
 
public class DriftUe4PluginServerTarget : TargetRules
{
    public DriftUe4PluginServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        ExtraModuleNames.Add("DriftUe4Plugin");
    }
}
