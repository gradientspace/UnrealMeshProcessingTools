using UnrealBuildTool;

public class CommandLineGeometryTest : ModuleRules
{
	public CommandLineGeometryTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicIncludePaths.Add("Runtime/Launch/Public");
		PrivateIncludePaths.Add("Runtime/Launch/Private");

		PrivateDependencyModuleNames.Add("Core");
		PrivateDependencyModuleNames.Add("Projects");

		// modules from GeometryProcessing Plugin
		PrivateDependencyModuleNames.Add("GeometricObjects");
		PrivateDependencyModuleNames.Add("DynamicMesh");
	}
}
