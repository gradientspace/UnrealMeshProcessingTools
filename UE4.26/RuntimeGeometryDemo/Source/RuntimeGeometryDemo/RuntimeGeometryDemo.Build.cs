// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RuntimeGeometryDemo : ModuleRules
{
	public RuntimeGeometryDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"HeadMountedDisplay",
				"ProceduralMeshComponent",
				"GeometricObjects",
				"DynamicMesh",
				"RuntimeGeometryUtils",
				"ModelingComponents"
			});
	}
}
