#pragma once
#include "CoreMinimal.h"
#include "EdMode.h"
#include "InputState.h"
#include "InteractiveToolManager.h"
#include "EdModeInteractiveToolsContext.h"

//=======================================
// EVERYTHING BELOW HERE IS BOILERPLATE!
//=======================================

class FMeshProcessingPluginEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_MeshProcessingPluginEdModeId;
public:
	FMeshProcessingPluginEdMode();
	virtual ~FMeshProcessingPluginEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	bool UsesToolkits() const override;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual bool ProcessEditDelete() override;
	virtual bool CanAutoSave() const override;
	virtual bool ShouldDrawWidget() const override;
	virtual bool AllowWidgetMove() override { return false;  }
	virtual bool UsesTransformWidget() const override { return true; }


	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;

	virtual bool MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;

	virtual bool StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	virtual bool EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;


	// End of FEdMode interface

	void RegisterModeTools();

	UEdModeInteractiveToolsContext* ToolsContext = nullptr;
	UInteractiveToolManager* GetToolManager() const { return ToolsContext->ToolManager;	}

};
