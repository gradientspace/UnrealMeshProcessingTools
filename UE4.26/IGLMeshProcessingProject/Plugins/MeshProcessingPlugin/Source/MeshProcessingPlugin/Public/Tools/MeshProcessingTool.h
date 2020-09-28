#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "DynamicMesh3.h"
#include "BaseTools/BaseMeshProcessingTool.h"
#include "MeshProcessingTool.generated.h"



UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshProcessingToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Options)
	bool bShowWireframe = true;
};



UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshProcessingTool : public UBaseMeshProcessingTool
{
	GENERATED_BODY()
public:

	/*
	 * UBaseMeshProcessingTool API
	 */
	virtual void InitializeProperties() override;
	virtual void OnShutdown(EToolShutdownType ShutdownType) override;

	virtual TUniquePtr<FDynamicMeshOperator> MakeNewOperator() override;


	/* 
	 * UMeshProcessingTool API for subclasses to implement
	 */

	virtual TUniqueFunction<void(FDynamicMesh3&)> MakeMeshProcessingFunction();

protected:
	UPROPERTY()
	UMeshProcessingToolProperties* MeshProcessingSettings;
};