#pragma once
#include "IGLIncludes.h"
#include "DynamicMesh3.h"

//
// Utility functions for converting between UE and libigl data structures
//

namespace iglext
{

	// Convert a UE FDynamicMesh3 to libigl-style indexed triangle mesh
	// V is Nx3 matrix of vertex positions
	// F is Nx3 matrix of triangle indices
	void DynamicMeshToIGLMesh(const FDynamicMesh3& Mesh, Eigen::MatrixXd& V, Eigen::MatrixXi& F)
	{
		check(Mesh.IsCompactV());

		V.resize(Mesh.MaxVertexID(), 3);
		for (int32 vid : Mesh.VertexIndicesItr())
		{
			FVector3d Pos = Mesh.GetVertex(vid);
			V(vid, 0) = Pos.X;
			V(vid, 1) = Pos.Y;
			V(vid, 2) = Pos.Z;
		}

		F.resize(Mesh.TriangleCount(), 3);
		for (int32 tid : Mesh.TriangleIndicesItr())
		{
			FIndex3i Tri = Mesh.GetTriangle(tid);
			F(tid, 0) = Tri.A;
			F(tid, 1) = Tri.B;
			F(tid, 2) = Tri.C;
		}
	}


	// Copy Nx3 vertex positions from matrix V into DynamicMesh
	void SetVertexPositions(FDynamicMesh3& Mesh, const Eigen::MatrixXd& V)
	{
		check(Mesh.IsCompactV());

		for (int32 vid : Mesh.VertexIndicesItr())
		{
			FVector3d NewPos(V(vid, 0), V(vid, 1), V(vid, 2));
			Mesh.SetVertex(vid, NewPos);
		}
	}



}