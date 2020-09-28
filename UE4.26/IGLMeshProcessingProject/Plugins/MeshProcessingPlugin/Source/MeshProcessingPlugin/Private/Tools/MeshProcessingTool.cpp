#include "Tools/MeshProcessingTool.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "DynamicMesh3.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "DynamicMeshToMeshDescription.h"
#include "AssetGenerationUtil.h"
#include "ToolSetupUtil.h"
#include "MeshNormals.h"

#define LOCTEXT_NAMESPACE "UMeshProcessingTool"


bool UMeshProcessingToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return ToolBuilderUtil::CountComponents(SceneState, CanMakeComponentTarget) == 1;
}

UInteractiveTool* UMeshProcessingToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UMeshProcessingTool* NewTool = MakeNewMeshProcessingTool(SceneState);

	UActorComponent* ActorComponent = ToolBuilderUtil::FindFirstComponent(SceneState, CanMakeComponentTarget);
	auto* MeshComponent = Cast<UPrimitiveComponent>(ActorComponent);
	check(MeshComponent != nullptr);

	NewTool->SetSelection(MakeComponentTarget(MeshComponent));

	return NewTool;
}


UMeshProcessingTool* UMeshProcessingToolBuilder::MakeNewMeshProcessingTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UMeshProcessingTool>(SceneState.ToolManager);
}





void FMeshProcessingOp::CalculateResult(FProgressCancel* Progress)
{
	ResultMesh->Copy(*InputMesh);
	ResultTransform = InputTransform;

	if (Progress->Cancelled())
	{
		return;
	}

	if (CalculateResultFunc)
	{
		CalculateResultFunc(*ResultMesh);
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






UMeshProcessingTool::UMeshProcessingTool()
{
}


void UMeshProcessingTool::Setup()
{
	UInteractiveTool::Setup();

	Preview = NewObject<UMeshOpPreviewWithBackgroundCompute>(this, "Preview");
	Preview->Setup(ComponentTarget->GetOwnerActor()->GetWorld(), this);
	Preview->PreviewMesh->SetTransform(ComponentTarget->GetWorldTransform());
	Preview->PreviewMesh->InitializeMesh(ComponentTarget->GetMesh());
	Preview->ConfigureMaterials(
		ToolSetupUtil::GetDefaultMaterial(GetToolManager(), ComponentTarget->GetMaterial(0)),
		ToolSetupUtil::GetDefaultWorkingMaterial(GetToolManager())
	);

	RegisterProperties();

	InputMeshCopy = MakeShared<FDynamicMesh3>(*Preview->PreviewMesh->GetPreviewDynamicMesh());

	ComponentTarget->SetOwnerVisibility(false);
	Preview->SetVisibility(true);

	RequestInvalidation();
}


void UMeshProcessingTool::RegisterProperties()
{
	MeshProcessingSettings = NewObject<UMeshProcessingToolProperties>(this);
	AddToolPropertySource(MeshProcessingSettings);
}


void UMeshProcessingTool::Shutdown(EToolShutdownType ShutdownType)
{
	ComponentTarget->SetOwnerVisibility(true);
	
	TUniquePtr<FDynamicMeshOpResult> Result = Preview->Shutdown();

	if (ShutdownType == EToolShutdownType::Accept)
	{
		GetToolManager()->BeginUndoTransaction(LOCTEXT("MeshProcessingToolTransactionName", "Edit Mesh"));

		bool bIsTopologyEdit = DoesEditChangeMeshTopology();

		ComponentTarget->CommitMesh([&Result, bIsTopologyEdit](FMeshDescription* MeshDescription)
		{
			FDynamicMeshToMeshDescription Converter;

			if (bIsTopologyEdit)
			{
				// only update vertex positions and normals
				Converter.Update(Result->Mesh.Get(), *MeshDescription);
			}
			else
			{
				// full conversion 
				Converter.Convert(Result->Mesh.Get(), *MeshDescription);
			}
		});

		GetToolManager()->EndUndoTransaction();
	}
}



void UMeshProcessingTool::RequestInvalidation()
{
	bInvalidationRequested = true;
}


void UMeshProcessingTool::Tick(float DeltaTime)
{
	if (bInvalidationRequested)
	{
		Preview->InvalidateResult();
		bInvalidationRequested = false;
	}

	Preview->Tick(DeltaTime);
}


TSharedPtr<FDynamicMeshOperator> UMeshProcessingTool::MakeNewOperator()
{
	TSharedPtr<FMeshProcessingOp> Op = MakeShared<FMeshProcessingOp>();
	Op->InputMesh = InputMeshCopy;
	Op->InputTransform = FTransform3d(ComponentTarget->GetWorldTransform());

	Op->CalculateResultFunc = MakeMeshProcessingFunction();

	return Op;
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
			FVector3f Normal = Normals[vid];
			ResultMesh.SetVertex(vid, Position + (FVector3d)Normal);
		}
	};


	return MoveTemp(EditFunction);
}




void UMeshProcessingTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	Preview->PreviewMesh->EnableWireframe(MeshProcessingSettings->bShowWireframe);
}


void UMeshProcessingTool::OnPropertyModified(UObject* PropertySet, UProperty* Property)
{
	RequestInvalidation();
}

bool UMeshProcessingTool::HasAccept() const
{
	return true;
}

bool UMeshProcessingTool::CanAccept() const
{
	return Preview->HaveValidResult();
}




#undef LOCTEXT_NAMESPACE
