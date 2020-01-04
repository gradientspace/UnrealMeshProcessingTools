#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "MultiSelectionTool.h"
#include "MeshExportTool.generated.h"


class FDynamicMesh3;


UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshExportToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};



UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshExportToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = Options)
	bool bTriangleUVs = true;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bTriangleNormals = true;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bRightHanded = true;
};





UCLASS()
class MESHPROCESSINGPLUGIN_API UMeshExportTool : public UMultiSelectionTool
{
	GENERATED_BODY()

public:
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;

	virtual bool HasCancel() const override { return true; }
	virtual bool HasAccept() const override { return true; }
	virtual bool CanAccept() const override { return true; }

protected:

	UPROPERTY()
	UMeshExportToolProperties* Properties;

protected:
	TArray<TUniquePtr<FDynamicMesh3>> Meshes;
};

