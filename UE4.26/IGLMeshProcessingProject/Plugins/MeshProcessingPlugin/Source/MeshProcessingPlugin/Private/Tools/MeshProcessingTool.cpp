#include "Tools/MeshProcessingTool.h"
#include "InteractiveToolManager.h"
#include "MeshNormals.h"

#define LOCTEXT_NAMESPACE "UMeshProcessingTool"




class FMeshProcessingOp : public FDynamicMeshOperator
{
public:
	virtual ~FMeshProcessingOp() {}

	TUniqueFunction<void(FDynamicMesh3&)> MeshProcessingFunc;

	FMeshProcessingOp(
		const FDynamicMesh3& InputMesh,
		const FTransform3d& TransformIn,
		TUniqueFunction<void(FDynamicMesh3&)> MeshProcessingFuncIn )
	{
		ResultMesh = MakeUnique<FDynamicMesh3>(InputMesh);
		SetResultTransform(TransformIn);
		MeshProcessingFunc = MoveTemp(MeshProcessingFuncIn);
	}

	virtual void CalculateResult(FProgressCancel* Progress) override
	{
		if (Progress->Cancelled())
		{
			return;
		}

		if (MeshProcessingFunc)
		{
			MeshProcessingFunc(*ResultMesh);
		}

		if (Progress->Cancelled())
		{
			return;
		}

		if (ResultMesh->HasAttributes() && ResultMesh->Attributes()->NumNormalLayers() == 1)
		{
			FDynamicMeshNormalOverlay* NormalsAttrib = ResultMesh->Attributes()->GetNormalLayer(0);
			FMeshNormals Normals(&*ResultMesh);
			Normals.RecomputeOverlayNormals(NormalsAttrib);
			Normals.CopyToOverlay(NormalsAttrib);
		}
	}
};





void UMeshProcessingTool::InitializeProperties()
{
	MeshProcessingSettings = NewObject<UMeshProcessingToolProperties>(this);
	AddToolPropertySource(MeshProcessingSettings);
	MeshProcessingSettings->WatchProperty(MeshProcessingSettings->bShowWireframe,
		[this](bool bWireframe) { GetUPreviewMesh()->EnableWireframe(bWireframe); } );
}


void UMeshProcessingTool::OnShutdown(EToolShutdownType ShutdownType)
{
	MeshProcessingSettings->SaveProperties(this);
}



TUniquePtr<FDynamicMeshOperator> UMeshProcessingTool::MakeNewOperator()
{
	return MakeUnique<FMeshProcessingOp>(
		GetInitialMesh(), 
		FTransform3d(GetPreviewTransform()), 
		MakeMeshProcessingFunction());
}




TUniqueFunction<void(FDynamicMesh3&)> UMeshProcessingTool::MakeMeshProcessingFunction()
{
	auto EditFunction = [](FDynamicMesh3& ResultMesh)
	{
		FMeshNormals Normals(&ResultMesh);
		Normals.ComputeVertexNormals();

		for (int32 vid : ResultMesh.VertexIndicesItr())
		{
			FVector3d Position = ResultMesh.GetVertex(vid);
			FVector3d Normal = Normals[vid];
			ResultMesh.SetVertex(vid, Position + (FVector3d)Normal);
		}
	};

	return MoveTemp(EditFunction);
}





#undef LOCTEXT_NAMESPACE
