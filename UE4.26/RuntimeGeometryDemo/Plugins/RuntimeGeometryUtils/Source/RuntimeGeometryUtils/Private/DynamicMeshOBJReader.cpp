#include "DynamicMeshOBJReader.h"
#include "DynamicMeshAttributeSet.h"
#include "tinyobj/tiny_obj_loader.h"


bool RTGUtils::ReadOBJMesh(
	const FString& Path,
	FDynamicMesh3& MeshOut,
	bool bNormals,
	bool bTexCoords,
	bool bVertexColors,
	bool bReverseOrientation)
{
	std::string inputfile(TCHAR_TO_UTF8(*Path));
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

	if (!warn.empty()) {
		UE_LOG(LogTemp, Display, TEXT("%s"), warn.c_str());
	}

	if (!err.empty()) {
		UE_LOG(LogTemp, Display, TEXT("%s"), err.c_str());
	}

	if (!ret) {
		return false;
	}

	// append vertices
	for (size_t vi = 0; vi < attrib.vertices.size() / 3; ++vi)
	{
		tinyobj::real_t vx = attrib.vertices[3 * vi + 0];
		tinyobj::real_t vy = attrib.vertices[3 * vi + 1];
		tinyobj::real_t vz = attrib.vertices[3 * vi + 2];

		MeshOut.AppendVertex(FVector3d(vx, vy, vz));
	}


	if (bVertexColors)
	{
		MeshOut.EnableVertexColors(FVector3f::Zero());
		for (size_t vi = 0; vi < attrib.vertices.size() / 3; ++vi)
		{
			tinyobj::real_t r = attrib.colors[3 * vi + 0];
			tinyobj::real_t g = attrib.colors[3 * vi + 1];
			tinyobj::real_t b = attrib.colors[3 * vi + 2];

			MeshOut.SetVertexColor(vi, FVector3f((float)r, (float)g, (float)b));
		}
	}

	if (bNormals || bTexCoords)
	{
		MeshOut.EnableAttributes();
	}
	FDynamicMeshNormalOverlay* Normals = (bNormals) ? MeshOut.Attributes()->PrimaryNormals() : nullptr;
	FDynamicMeshUVOverlay* UVs = (bTexCoords) ? MeshOut.Attributes()->PrimaryUV() : nullptr;
	if (Normals)
	{
		for (size_t ni = 0; ni < attrib.normals.size() / 3; ++ni)
		{
			tinyobj::real_t nx = attrib.normals[3 * ni + 0];
			tinyobj::real_t ny = attrib.normals[3 * ni + 1];
			tinyobj::real_t nz = attrib.normals[3 * ni + 2];

			Normals->AppendElement(FVector3f((float)nx, (float)ny, (float)nz));
		}
	}
	if (UVs)
	{
		for (size_t ti = 0; ti < attrib.texcoords.size() / 2; ++ti)
		{
			tinyobj::real_t tx = attrib.texcoords[2 * ti + 0];
			tinyobj::real_t ty = attrib.texcoords[2 * ti + 1];

			UVs->AppendElement(FVector2f((float)tx, (float)ty));
		}
	}

	// append faces as triangles
	for (size_t s = 0; s < shapes.size(); s++) {	// Loop over shapes

		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {	// Loop over faces(polygon)
			int fv = shapes[s].mesh.num_face_vertices[f];

			TArray<FIndex3i> Triangles;
			for (size_t v = 1; v < fv - 1; v++)
			{
				Triangles.Add(FIndex3i(0, v, v + 1));
			}

			int32 NumTris = Triangles.Num();
			for (int32 ti = 0; ti < NumTris; ++ti)
			{
				FIndex3i TriVerts = Triangles[ti];
				tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + TriVerts.A];
				tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + TriVerts.B];
				tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + TriVerts.C];

				int32 tid = MeshOut.AppendTriangle(idx0.vertex_index, idx1.vertex_index, idx2.vertex_index);

				if (Normals && Normals->IsElement(idx0.normal_index) && Normals->IsElement(idx1.normal_index) && Normals->IsElement(idx2.normal_index))
				{
					Normals->SetTriangle(tid, FIndex3i(idx0.normal_index, idx1.normal_index, idx2.normal_index));
				}
				if (UVs && UVs->IsElement(idx0.texcoord_index) && UVs->IsElement(idx1.texcoord_index) && UVs->IsElement(idx2.texcoord_index))
				{
					UVs->SetTriangle(tid, FIndex3i(idx0.texcoord_index, idx1.texcoord_index, idx2.texcoord_index));
				}
			}

			index_offset += fv;

			// per-face material
			//shapes[s].mesh.material_ids[f];
		}
	}

	if (bReverseOrientation)
	{
		MeshOut.ReverseOrientation();
	}

	return true;
}