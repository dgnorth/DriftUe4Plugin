// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DriftUe4PluginTarget : TargetRules
{
    public DriftUe4PluginTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        ExtraModuleNames.Add("DriftUe4Plugin");
    }
}
