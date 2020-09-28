#include "MeshProcessingPluginCommands.h"
#include "EditorStyleSet.h"
#include "MeshProcessingPluginStyle.h"

#define LOCTEXT_NAMESPACE "MeshProcessingPluginCommands"

void FMeshProcessingPluginCommands::RegisterCommands()
{
	// [1] 
	// Configure each Command here (ie cut-and-paste). 
	// The First quoted string is the label that will appear in the toolbar button, and the second is the tooltip.

	UI_COMMAND(BeginIGLSmoothingTool, "IGLSmooth", "Start the LibIGL Mesh Smoothing Tool", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(BeginMeshExportTool, "Export", "Start the Mesh Export Tool", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(AcceptActiveTool, "Accept", "Accept the active tool", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CancelActiveTool, "Cancel", "Cancel the active tool", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CompleteActiveTool, "Complete", "Complete the active tool", EUserInterfaceActionType::Button, FInputChord());
}


//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================


FMeshProcessingPluginCommands::FMeshProcessingPluginCommands() : TCommands<FMeshProcessingPluginCommands>(
	"MeshProcessingPluginCommands",
	NSLOCTEXT("Contexts", "MeshProcessingPluginEditorMode", "Mesh Processing Mode"),
	NAME_None, FMeshProcessingPluginStyle::Get()->GetStyleSetName())
{
}



#undef LOCTEXT_NAMESPACE