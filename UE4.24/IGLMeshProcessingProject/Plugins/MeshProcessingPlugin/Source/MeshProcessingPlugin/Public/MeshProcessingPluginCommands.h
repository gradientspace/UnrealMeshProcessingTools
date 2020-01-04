#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FMeshProcessingPluginCommands : public TCommands<FMeshProcessingPluginCommands>
{
public:

	// [1] 
	// Add instances of FUICommandInfo for each command button you want to appear in the EdMode Toolbar.
	// You must also configure the command in MeshProcessingPluginCommands.cpp
	//

    TSharedPtr<FUICommandInfo> BeginIGLSmoothingTool;
	TSharedPtr<FUICommandInfo> BeginMeshExportTool;



	//=======================================
	// EVERYTHING BELOW HERE IS BOILERPLATE!
	//=======================================

	TSharedPtr<FUICommandInfo> AcceptActiveTool;
	TSharedPtr<FUICommandInfo> CancelActiveTool;
	TSharedPtr<FUICommandInfo> CompleteActiveTool;


public:
	FMeshProcessingPluginCommands();
	virtual void RegisterCommands() override;

};