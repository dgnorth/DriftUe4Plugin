// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class DriftUe4PluginEditorTarget : TargetRules
{
    public DriftUe4PluginEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        ExtraModuleNames.Add("DriftUe4Plugin");
    }
}
