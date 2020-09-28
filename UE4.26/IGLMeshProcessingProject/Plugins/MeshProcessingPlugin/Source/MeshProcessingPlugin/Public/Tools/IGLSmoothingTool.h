#pragma once
#include "Tools/MeshProcessingTool.h"
#include "BaseTools/BaseMeshProcessingTool.h"
#include "IGLSmoothingTool.generated.h"		// Unreal will generate this file



// This UInteractiveToolPropertySet provides a list of settings. These settings will appear in a DetailsView panel in the Unreal Editor.
UCLASS()
class MESHPROCESSINGPLUGIN_API UIGLSmoothingToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	/** How strong the mesh smoothing should be */
	UPROPERTY(EditAnywhere, Category = Options)
	float Smoothness = 100.0f;

	/** How many iterations of mesh smoothing to compute */
	UPROPERTY(EditAnywhere, Category = Options)
	int Iterations = 1;
};



// Instances of your UMeshProcessingTool subclass will be created on-demand when the user clicks an associated
// button in the UnrealEditor. The button-to-Tool mapping is configured in MeshProcessingPluginEdMode.h and Toolkit.h
UCLASS()
class MESHPROCESSINGPLUGIN_API UIGLSmoothingTool : public UMeshProcessingTool
{
	GENERATED_BODY()
public:

	/*
	 * UBaseMeshProcessingTool API overrides
	 */
	virtual void InitializeProperties() override;
	virtual void OnShutdown(EToolShutdownType ShutdownType) override;

	/**
	 * This function returns a lambda that calculates a new mesh based on the input mesh (eg calls your libigl code).
	 * UMeshProcessingTool will call this function from a background thread, inside a FDynamicMeshOperator it creates.
	 */
	virtual TUniqueFunction<void(FDynamicMesh3&)> MakeMeshProcessingFunction() override;


	/*
	 * The functions below are UBaseMeshProcessingTool that you can override to control it's behavior
	 * and/or what data structures it can precompute for you. Default values are shown below. 
	 * Note that these cannot be modified after Setup().
	 */
	//virtual bool RequiresScaleNormalization() const { return true; }
	//virtual bool RequiresInitialVtxNormals() const { return false; }
	//virtual bool RequiresInitialBoundaryLoops() const { return false; }


public:
	// The UProperties of this object will appear in a DetailsView panel on the left-hand side of the Unreal Editor
	UPROPERTY()
	UIGLSmoothingToolProperties* SmoothProperties;
};




// The ToolBuilder implementation just creates a new instance of your UMeshProcessingTool (ie boilerplate factory pattern)
UCLASS()
class MESHPROCESSINGPLUGIN_API UIGLSmoothingToolBuilder : public UBaseMeshProcessingToolBuilder
{
	GENERATED_BODY()
public:
	virtual UBaseMeshProcessingTool* MakeNewToolInstance(UObject* Outer) const {
		return NewObject<UIGLSmoothingTool>(Outer);
	}
};


