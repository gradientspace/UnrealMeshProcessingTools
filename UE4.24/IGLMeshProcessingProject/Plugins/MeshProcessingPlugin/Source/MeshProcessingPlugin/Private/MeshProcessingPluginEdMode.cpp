#include "MeshProcessingPluginEdMode.h"
#include "MeshProcessingPluginEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"
#include "MeshProcessingPluginCommands.h"
#include "BaseTools/MeshSurfacePointTool.h"

// [1] 
// include the header files for your various Tools here
//
#include "Tools/IGLSmoothingTool.h"
#include "Tools/MeshExportTool.h"



void FMeshProcessingPluginEdMode::RegisterModeTools()
{
	//// BOILERPLATE BETWEEN THESE LINES
	const TSharedRef<FUICommandList>& CommandList = Toolkit->GetToolkitCommands();
	const FMeshProcessingPluginCommands& PluginCommands = FMeshProcessingPluginCommands::Get();
	// this is just a helper function
	auto RegisterToolFunc = [this, &CommandList](TSharedPtr<FUICommandInfo> UICommand, FString ToolIdentifier, UInteractiveToolBuilder* Builder)
	{
		ToolsContext->ToolManager->RegisterToolType(ToolIdentifier, Builder);
		CommandList->MapAction(UICommand,
			FExecuteAction::CreateLambda([this, ToolIdentifier]() { ToolsContext->StartTool(ToolIdentifier); }),
			FCanExecuteAction::CreateLambda([this, ToolIdentifier]() { return ToolsContext->CanStartTool(ToolIdentifier); }));
	};
	//// BOILERPLATE BETWEEN THESE LINES


	// [2]
	// For each tool you want to appear in the toolbar, you have to register a Command/Identifier/ToolBuilder mapping.
	// The Commands are stored in MeshProcessingPluginCommands.h, so for a new Tool you have to add a new Command there.
	// To get the Command to appear in the Toolbar, add it in MeshProcessingPluginEdModeToolkit.cpp
	//

	RegisterToolFunc(PluginCommands.BeginIGLSmoothingTool, TEXT("IGLSmoothTool"), NewObject<UIGLSmoothingToolBuilder>());

	RegisterToolFunc(PluginCommands.BeginMeshExportTool, TEXT("Export Mesh"), NewObject<UMeshExportToolBuilder>());
}




//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================


const FEditorModeID FMeshProcessingPluginEdMode::EM_MeshProcessingPluginEdModeId = TEXT("EM_MeshProcessingPluginEdMode");

void FMeshProcessingPluginEdMode::Enter()
{
	FEdMode::Enter();

	ToolsContext = NewObject<UEdModeInteractiveToolsContext>();
	ToolsContext->InitializeContextFromEdMode(this);

	const FMeshProcessingPluginCommands& PluginCommands = FMeshProcessingPluginCommands::Get();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FMeshProcessingPluginEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());

		const TSharedRef<FUICommandList>& CommandList = Toolkit->GetToolkitCommands();

		CommandList->MapAction(
			PluginCommands.AcceptActiveTool,
			FExecuteAction::CreateLambda([this]() { ToolsContext->EndTool(EToolShutdownType::Accept); }),
			FCanExecuteAction::CreateLambda([this]() { return ToolsContext->CanAcceptActiveTool(); }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([this]() {return ToolsContext->ActiveToolHasAccept(); }),
			EUIActionRepeatMode::RepeatDisabled
		);

		CommandList->MapAction(
			PluginCommands.CancelActiveTool,
			FExecuteAction::CreateLambda([this]() { ToolsContext->EndTool(EToolShutdownType::Cancel); }),
			FCanExecuteAction::CreateLambda([this]() { return ToolsContext->CanCancelActiveTool(); }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([this]() {return ToolsContext->ActiveToolHasAccept(); }),
			EUIActionRepeatMode::RepeatDisabled
		);

		CommandList->MapAction(
			PluginCommands.CompleteActiveTool,
			FExecuteAction::CreateLambda([this]() { ToolsContext->EndTool(EToolShutdownType::Completed); }),
			FCanExecuteAction::CreateLambda([this]() { return ToolsContext->CanCompleteActiveTool(); }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([this]() {return ToolsContext->CanCompleteActiveTool(); }),
			EUIActionRepeatMode::RepeatDisabled
		);

	}

	RegisterModeTools();
}


FMeshProcessingPluginEdMode::FMeshProcessingPluginEdMode()
{

}

FMeshProcessingPluginEdMode::~FMeshProcessingPluginEdMode()
{
	if (ToolsContext != nullptr)
	{
		ToolsContext->ShutdownContext();
		ToolsContext = nullptr;
	}
}


void FMeshProcessingPluginEdMode::Exit()
{
	ToolsContext->ShutdownContext();
	ToolsContext = nullptr;

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FMeshProcessingPluginEdMode::UsesToolkits() const
{
	return true;
}


void FMeshProcessingPluginEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	FEdMode::Tick(ViewportClient, DeltaTime);
	ToolsContext->Tick(ViewportClient, DeltaTime);
}


void FMeshProcessingPluginEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	FEdMode::Render(View, Viewport, PDI);
	if (PDI->IsHitTesting() == false && ToolsContext != nullptr)
	{
		ToolsContext->Render(View, Viewport, PDI);
	}
}


void FMeshProcessingPluginEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolsContext);
}

bool FMeshProcessingPluginEdMode::ProcessEditDelete()
{
	return ToolsContext->ProcessEditDelete();
}

bool FMeshProcessingPluginEdMode::CanAutoSave() const
{
	return ToolsContext->ToolManager->HasAnyActiveTool() == false;
}

bool FMeshProcessingPluginEdMode::ShouldDrawWidget() const
{
	return (ToolsContext->ToolManager->HasAnyActiveTool() == false);
}



bool FMeshProcessingPluginEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	bool bHandled = ToolsContext->InputKey(ViewportClient, Viewport, Key, Event);
	if (bHandled == false)
	{
		bHandled = FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
	}
	return bHandled;
}


bool FMeshProcessingPluginEdMode::MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	return ToolsContext->MouseEnter(ViewportClient, Viewport, x, y);
}
bool FMeshProcessingPluginEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
{
	return ToolsContext->MouseMove(ViewportClient, Viewport, x, y);
}
bool FMeshProcessingPluginEdMode::MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	return ToolsContext->MouseLeave(ViewportClient, Viewport);
}


bool FMeshProcessingPluginEdMode::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	return ToolsContext->StartTracking(InViewportClient, InViewport);
}

bool FMeshProcessingPluginEdMode::CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	return ToolsContext->CapturedMouseMove(InViewportClient, InViewport, InMouseX, InMouseY);
}

bool FMeshProcessingPluginEdMode::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	return ToolsContext->EndTracking(InViewportClient, InViewport);
}