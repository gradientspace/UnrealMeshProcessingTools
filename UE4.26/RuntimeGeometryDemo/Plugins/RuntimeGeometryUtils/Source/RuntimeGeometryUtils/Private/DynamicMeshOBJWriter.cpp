#include "DynamicMeshOBJWriter.h"
#include "DynamicMeshAttributeSet.h"
#include "DynamicMeshEditor.h"

#include <fstream>



class FDynamicMeshOBJWriter
{
public:

	std::ofstream FileOut;

	TFunction<bool(const FString&)> OpenFile = [this](const FString& Path) { FileOut.open(*Path, std::ofstream::out | std::ofstream::trunc); return !!FileOut; };
	TFunction<void()> CloseFile = [this]() { FileOut.close(); };
	TFunction<void(const TCHAR*)> WriteLine = [this](const TCHAR* Line) { FileOut << TCHAR_TO_ANSI(Line) << std::endl; };

	bool Write(const char* OutputPath, const FDynamicMesh3& Mesh)
	{
		if (!OpenFile(OutputPath))
		{
			return false;
		}

		int32 NumVertices = Mesh.VertexCount();
		for (int32 vi = 0; vi < NumVertices; ++vi)
		{
			FVector3d Pos = Mesh.GetVertex(vi);
			WriteLine(*FString::Printf(TEXT("v %f %f %f"), Pos.X, Pos.Y, Pos.Z));
		}

		int32 NumUVs = 0;
		const FDynamicMeshUVOverlay* UVs = nullptr;
		if (Mesh.Attributes() && Mesh.Attributes()->PrimaryUV())
		{
			UVs = Mesh.Attributes()->PrimaryUV();
			NumUVs = UVs->ElementCount();
			for (int32 ui = 0; ui < NumUVs; ++ui)
			{
				FVector2f UV = UVs->GetElement(ui);
				WriteLine(*FString::Printf(TEXT("vt %f %f"), UV.X, UV.Y));
			}
		}

		int32 NumNormals = 0;
		const FDynamicMeshNormalOverlay* Normals = nullptr;
		if (Mesh.Attributes() && Mesh.Attributes()->PrimaryNormals())
		{
			Normals = Mesh.Attributes()->PrimaryNormals();
			NumNormals = Normals->ElementCount();
			for (int32 ni = 0; ni < NumNormals; ++ni)
			{
				FVector3f Normal = Normals->GetElement(ni);
				WriteLine(*FString::Printf(TEXT("vn %f %f %f"), Normal.X, Normal.Y, Normal.Z));
			}
		}

		struct FMeshTri
		{
			int32 Index;
			int32 Group;
		};
		TSet<int32> AllGroupIDs;

		TArray<FMeshTri> Triangles;

		int32 NumTriangles = Mesh.TriangleCount();
		for (int32 ti = 0; ti < NumTriangles; ++ti)
		{
			if (Mesh.IsTriangle(ti))
			{
				int32 GroupID = Mesh.GetTriangleGroup(ti);
				AllGroupIDs.Add(GroupID);
				Triangles.Add({ ti, GroupID });
			}
		}
		bool bHaveGroups = AllGroupIDs.Num() > 1;

		Triangles.StableSort([](const FMeshTri& Tri0, const FMeshTri& Tri1)
		{
			return Tri0.Group < Tri1.Group;
		});


		int32 CurGroupID = -99999;
		for (FMeshTri MeshTri : Triangles)
		{
			if (bHaveGroups && MeshTri.Group != CurGroupID)
			{
				WriteLine(*FString::Printf(TEXT("g %d"), MeshTri.Group));
				CurGroupID = MeshTri.Group;
			}

			int32 ti = MeshTri.Index;

			FIndex3i TriVertices = Mesh.GetTriangle(ti);
			FIndex3i TriUVs = (NumUVs > 0) ? UVs->GetTriangle(ti) : FIndex3i::Invalid();
			FIndex3i TriNormals = (NumNormals > 0) ? Normals->GetTriangle(ti) : FIndex3i::Invalid();

			bool bHaveUV = (NumUVs != 0) && UVs->IsSetTriangle(ti);
			bool bHaveNormal = (NumNormals != 0) && Normals->IsSetTriangle(ti);

			if (bHaveUV && bHaveNormal)
			{
				WriteLine(*FString::Printf(TEXT("f %d/%d/%d %d/%d/%d %d/%d/%d"),
					TriVertices.A + 1, TriUVs.A + 1, TriNormals.A + 1,
					TriVertices.B + 1, TriUVs.B + 1, TriNormals.B + 1,
					TriVertices.C + 1, TriUVs.C + 1, TriNormals.C + 1));
			}
			else if (bHaveUV)
			{
				WriteLine(*FString::Printf(TEXT("f %d/%d %d/%d %d/%d"),
					TriVertices.A + 1, TriUVs.A + 1, TriVertices.B + 1, TriUVs.B + 1, TriVertices.C + 1, TriUVs.C + 1));
			}
			else if (bHaveNormal)
			{
				WriteLine(*FString::Printf(TEXT("f %d//%d %d//%d %d//%d"),
					TriVertices.A + 1, TriNormals.A + 1, TriVertices.B + 1, TriNormals.B + 1, TriVertices.C + 1, TriNormals.C + 1));
			}
			else
			{
				WriteLine(*FString::Printf(TEXT("f %d %d %d"), TriVertices.A + 1, TriVertices.B + 1, TriVertices.C + 1));
			}
		}

		CloseFile();

		return true;
	}
};



bool RTGUtils::WriteOBJMesh(
	const FString& OutputPath,
	const FDynamicMesh3& Mesh,
	bool bReverseOrientation)
{
	const FDynamicMesh3* WriteMesh = &Mesh;

	FDynamicMesh3 CopyMesh;
	if (bReverseOrientation)
	{
		CopyMesh = Mesh;
		CopyMesh.ReverseOrientation();
		WriteMesh = &CopyMesh;
	}

	FDynamicMeshOBJWriter Writer;
	std::string OutputFilePath(TCHAR_TO_UTF8(*OutputPath));
	return Writer.Write(OutputFilePath.c_str(), *WriteMesh);
}



bool RTGUtils::WriteOBJMeshes(
	const FString& OutputPath,
	const TArray<FDynamicMesh3>& Meshes,
	bool bReverseOrientation)
{
	FDynamicMesh3 CombinedMesh;
	FDynamicMeshEditor Editor(&CombinedMesh);

	for (const FDynamicMesh3& Mesh : Meshes)
	{
		if (Mesh.HasTriangleGroups())
		{
			CombinedMesh.EnableTriangleGroups();
		}
		if (Mesh.HasAttributes())
		{
			CombinedMesh.EnableAttributes();
		}

		FMeshIndexMappings Mappings;
		Editor.AppendMesh(&Mesh, Mappings);
	}

	if (bReverseOrientation)
	{
		CombinedMesh.ReverseOrientation();
	}

	FDynamicMeshOBJWriter Writer;

	std::string OutputFilePath(TCHAR_TO_UTF8(*OutputPath));
	return Writer.Write(OutputFilePath.c_str(), CombinedMesh);
}


