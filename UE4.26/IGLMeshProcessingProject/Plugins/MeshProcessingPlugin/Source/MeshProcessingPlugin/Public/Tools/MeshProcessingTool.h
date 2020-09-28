#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "SingleSelectionTool.h"
#include "MeshOpPreviewHelpers.h"
#include "ModelingOperators.h"
#include "DynamicMesh3.h"
#include "MeshProcessingTool.generated.h"


UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshProcessingToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;

	virtual UMeshProcessingTool* MakeNewMeshProcessingTool(const FToolBuilderState& SceneState) const;
};




class MESHPROCESSINGPLUGIN_API FMeshProcessingOp : public FDynamicMeshOperator
{
public:
	// lambda for calculating result
	TUniqueFunction<void(FDynamicMesh3&)> CalculateResultFunc;

	// inputs
	TSharedPtr<FDynamicMesh3> InputMesh;
	FTransform3d InputTransform;

	// FDynamicMeshOperator implementation
	virtual void CalculateResult(FProgressCancel* Progress) override;
};


UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshProcessingToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Options)
	bool bShowWireframe = true;
};





UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshProcessingTool : public USingleSelectionTool, public IDynamicMeshOperatorFactory
{
	GENERATED_BODY()
public:
	UMeshProcessingTool();

	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual void OnTick(float DeltaTime) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;

	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override;
	virtual bool CanAccept() const override;

	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;

	// IDynamicMeshOperatorFactory API
	virtual TUniquePtr<FDynamicMeshOperator> MakeNewOperator() override;



	virtual TUniqueFunction<void(FDynamicMesh3&)> MakeMeshProcessingFunction();

	// return true here to do a simpler mesh update
	virtual bool DoesEditChangeMeshTopology() const { return true; }

protected:
	UPROPERTY()
	UMeshOpPreviewWithBackgroundCompute* Preview;

	UPROPERTY()
	UMeshProcessingToolProperties* MeshProcessingSettings;

	virtual void RegisterProperties();

protected:
	bool bInvalidationRequested = false;
	void RequestInvalidation();

	TSharedPtr<FDynamicMesh3> InputMeshCopy;
};