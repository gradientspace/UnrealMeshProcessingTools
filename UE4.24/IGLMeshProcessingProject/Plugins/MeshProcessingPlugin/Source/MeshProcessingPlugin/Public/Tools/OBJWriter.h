#pragma once

#include "CoreMinimal.h"
#include "VectorTypes.h"
#include "IndexTypes.h"

#include <fstream>

class FOBJWriter
{
public:

	FString OutputPath;
	std::ofstream FileOut;

	TFunction<bool(const FString&)> OpenFile = [this](const FString& Path) { FileOut.open(*Path, std::ofstream::out | std::ofstream::trunc); return !!FileOut; };
	TFunction<void()> CloseFile = [this]() { FileOut.close(); };
	TFunction<void(const TCHAR*)> WriteLine = [this](const TCHAR* Line) { FileOut << TCHAR_TO_ANSI(Line) << std::endl; };

	TFunction<int32(void)> GetVertexCount = []() { return 0; };
	TFunction<FVector3d(int32)> GetVertex = [](int32 VertexID) { return FVector3d::Zero(); };

	TFunction<int32(void)> GetUVCount = []() { return 0; };
	TFunction<FVector2d(int32)> GetUV = [](int32 UVID) { return FVector2d::Zero(); };

	TFunction<int32(void)> GetNormalCount = []() { return 0; };
	TFunction<FVector3d(int32)> GetNormal = [](int32 NormalID) { return FVector3d::UnitZ(); };

	TFunction<int32(void)> GetTriangleCount = []() { return 0; };
	TFunction<bool(int32)> IsTriangle = [](int32 TriangleID) { return true; };
	TFunction<void(int32, FIndex3i&, FIndex3i&, FIndex3i&)> GetTriangle = [](int32 TriangleID, FIndex3i& Vertices, FIndex3i& UVs, FIndex3i& Normals) {};


	bool Write()
	{
		if (!OpenFile(OutputPath))
		{
			return false;
		}

		int32 NumVertices = GetVertexCount();
		for (int32 vi = 0; vi < NumVertices; ++vi)
		{
			FVector3d Pos = GetVertex(vi);
   			 WriteLine( *FString::Printf(TEXT("v %f %f %f"), Pos.X, Pos.Y, Pos.Z) );
		}

		int32 NumUVs = GetUVCount();
		for (int32 ui = 0; ui < NumUVs; ++ui)
		{
			FVector2d UV = GetUV(ui);
			WriteLine( *FString::Printf(TEXT("vt %f %f"), UV.X, UV.Y) );
		}

		int32 NumNormals = GetNormalCount();
		for (int32 ni = 0; ni < NumNormals; ++ni)
		{
			FVector3d Normal = GetNormal(ni);
			WriteLine(*FString::Printf(TEXT("vn %f %f %f"), Normal.X, Normal.Y, Normal.Z));
		}

		int32 NumTriangles = GetTriangleCount();
		for (int32 ti = 0; ti < NumTriangles; ++ti)
		{
			if (IsTriangle(ti))
			{
				FIndex3i Vertices, UVs, Normals;
				GetTriangle(ti, Vertices, UVs, Normals);
				if (NumUVs == 0 && NumNormals == 0)
				{
					WriteLine(*FString::Printf(TEXT("f %d %d %d"), Vertices.A+1, Vertices.B+1, Vertices.C+1));
				}
				else if (NumUVs == 0)
				{
					WriteLine(*FString::Printf(TEXT("f %d//%d %d//%d %d//%d"),
						Vertices.A+1, Normals.A+1, Vertices.B+1, Normals.B+1, Vertices.C+1, Normals.C+1));
				}
				else if (NumUVs > 0 && NumNormals > 0)
				{
					WriteLine(*FString::Printf(TEXT("f %d/%d/%d %d/%d/%d %d/%d/%d"),
						Vertices.A+1, UVs.A+1, Normals.A+1, 
						Vertices.B+1, UVs.B+1, Normals.B+1, 
						Vertices.C+1, UVs.C+1, Normals.C+1));
				}
				else
				{
					check(false);
				}
			}
		}

		CloseFile();

		return true;
	}

};