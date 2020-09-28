#include "Tools/MeshExportTool.h"
#include "ToolBuilderUtil.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "Tools/OBJWriter.h"
#include "DynamicMesh3.h"
#include "DynamicMeshAttributeSet.h"


#define LOCTEXT_NAMESPACE "UMeshExportTool"


bool UMeshExportToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return ToolBuilderUtil::CountComponents(SceneState, CanMakeComponentTarget) > 0;
}

UInteractiveTool* UMeshExportToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UMeshExportTool* NewTool = NewObject<UMeshExportTool>(SceneState.ToolManager);
	TArray<UActorComponent*> Components = ToolBuilderUtil::FindAllComponents(SceneState, CanMakeComponentTarget);
	check(Components.Num() > 0);
	TArray<TUniquePtr<FPrimitiveComponentTarget>> ComponentTargets;
	for (UActorComponent* ActorComponent : Components)
	{
		auto* MeshComponent = Cast<UPrimitiveComponent>(ActorComponent);
		if (MeshComponent)
		{
			ComponentTargets.Add(MakeComponentTarget(MeshComponent));
		}
	}

	NewTool->SetSelection(MoveTemp(ComponentTargets));
	return NewTool;
}




void UMeshExportTool::Setup()
{

	for (TUniquePtr<FPrimitiveComponentTarget>& ComponentTarget : ComponentTargets)
	{
		FTransform3d Transform(ComponentTarget->GetWorldTransform());

		TUniquePtr<FDynamicMesh3> Mesh = MakeUnique<FDynamicMesh3>();
		FMeshDescriptionToDynamicMesh Converter;
		Converter.Convert(ComponentTarget->GetMesh(), *Mesh);
		Meshes.Add(MoveTemp(Mesh));
	}


	Properties = NewObject<UMeshExportToolProperties>(this);
	AddToolPropertySource(Properties);

}



void UMeshExportTool::Shutdown(EToolShutdownType ShutdownType)
{
	if (ShutdownType == EToolShutdownType::Accept)
	{
		FOBJWriter Writer;
		Writer.OutputPath = TEXT("C:\\scratch\\__EXPORT.obj");

		FDynamicMesh3& Mesh = *Meshes[0];
		check(Mesh.IsCompact());

		Writer.GetVertexCount = [&]() { return Mesh.MaxVertexID(); };
		Writer.GetVertex = [&](int32 VertexID) { return Mesh.GetVertex(VertexID); };

		FDynamicMeshNormalOverlay* NormalOverlay = nullptr;
		if (Mesh.HasAttributes() && Mesh.Attributes()->NumNormalLayers() > 0)
		{
			NormalOverlay = Mesh.Attributes()->GetNormalLayer(0);
			Writer.GetNormalCount = [&]() { return NormalOverlay->MaxElementID(); };
			Writer.GetNormal = [&](int32 NormalID) { return (FVector3d)NormalOverlay->GetElement(NormalID); };
		}

		FDynamicMeshUVOverlay* UVOverlay = nullptr;
		if (Mesh.HasAttributes() && Mesh.Attributes()->NumUVLayers() > 0)
		{
			UVOverlay = Mesh.Attributes()->GetUVLayer(0);
			Writer.GetUVCount = [&]() { return UVOverlay->MaxElementID(); };
			Writer.GetUV = [&](int32 UVID ) { return (FVector2d)UVOverlay->GetElement(UVID); };
		}

		Writer.GetTriangleCount = [&]() { return Mesh.MaxTriangleID(); };
		Writer.GetTriangle = [&](int32 TriangleID, FIndex3i& Vertices, FIndex3i& UVs, FIndex3i& Normals) 
		{ 
			Vertices = Mesh.GetTriangle(TriangleID);
			UVs = (UVOverlay != nullptr) ? UVOverlay->GetTriangle(TriangleID) : FIndex3i::Zero();
			Normals = (NormalOverlay != nullptr) ? NormalOverlay->GetTriangle(TriangleID) : FIndex3i::Zero();
			if (Properties->bRightHanded)
			{
				Swap(Vertices.B, Vertices.C);
				Swap(UVs.B, UVs.C);
				Swap(Normals.B, Normals.C);
			}
		};

		bool bOK = Writer.Write();
		check(bOK);
	}
}



#undef LOCTEXT_NAMESPACE