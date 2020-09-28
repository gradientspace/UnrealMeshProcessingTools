#pragma once
#include "Tools/MeshProcessingTool.h"
#include "IGLSmoothingTool.generated.h"		// Unreal will generate this file


// The ToolBuilder implementation just creates a new instance of your UMeshProcessingTool (ie boilerplate factory pattern)
UCLASS()
class MESHPROCESSINGPLUGIN_API UIGLSmoothingToolBuilder : public UMeshProcessingToolBuilder
{
	GENERATED_BODY()
public:
	virtual UMeshProcessingTool* MakeNewMeshProcessingTool(const FToolBuilderState& SceneState) const;
};


// This UInteractiveToolPropertySet provides a list of settings. These settings will appear in a DetailsView panel in the Unreal Editor.
UCLASS()
class MESHPROCESSINGPLUGIN_API UIGLSmoothingToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Options)
	float Smoothness = 100.0f;

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

	// This function returns a lambda that calculates a new mesh based on the input mesh (eg calls your libigl code)
	virtual TUniqueFunction<void(FDynamicMesh3&)> MakeMeshProcessingFunction() override;

	// return true here to do a simpler mesh update that only changes vertex positions
	virtual bool DoesEditChangeMeshTopology() const override { return false; }

	// The UProperties of this object will appear in a DetailsView panel on the left-hand side of the Unreal Editor
	UPROPERTY()
	UIGLSmoothingToolProperties* Properties;

	// extend UMeshProcessingTool::RegisterProperties() to register new Properties 
	virtual void RegisterProperties() override;
};