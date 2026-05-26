// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class SkyfireUprising_Lvl5Target : TargetRules
{
	public SkyfireUprising_Lvl5Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "SkyfireUprising_Lvl5" } );
	}
}
