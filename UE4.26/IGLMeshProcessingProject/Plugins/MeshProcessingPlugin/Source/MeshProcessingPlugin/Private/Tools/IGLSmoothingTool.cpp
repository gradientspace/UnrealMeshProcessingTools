#include "Tools/IGLSmoothingTool.h"
#include "Tools/IGLIncludes.h"		// include this before your igl includes (or add them to the list there)
#include "Tools/IGLUtil.h"

UMeshProcessingTool* UIGLSmoothingToolBuilder::MakeNewMeshProcessingTool(const FToolBuilderState& SceneState) const
{
	return NewObject<UIGLSmoothingTool>(SceneState.ToolManager);
}

void UIGLSmoothingTool::RegisterProperties()
{
	Properties = NewObject<UIGLSmoothingToolProperties>(this);
	AddToolPropertySource(Properties);
	UMeshProcessingTool::RegisterProperties();
}

TUniqueFunction<void(FDynamicMesh3&)> UIGLSmoothingTool::MakeMeshProcessingFunction()
{
	// Do *not* use & capture or reference this class directly in the lambda constructed below.
	// Make copies of values. Otherwise compute thread may reference changing values.
	int SolveIterations = Properties->Iterations;
	float Smoothness = Properties->Smoothness;

	// construct compute lambda
	auto EditFunction = [Smoothness, SolveIterations](FDynamicMesh3& ResultMesh)
	{
		// this code is based on libigl tutorial 205_Laplacian
		// https://github.com/libigl/libigl/blob/master/tutorial/205_Laplacian/main.cpp

		// convert FDynamicMesh3 to igl mesh representation
		Eigen::MatrixXd V;
		Eigen::MatrixXi F;
		iglext::DynamicMeshToIGLMesh(ResultMesh, V, F);

		// Compute Laplace-Beltrami operator L (#V by #V matrix)
		Eigen::SparseMatrix<double> L;
		igl::cotmatrix(V, F, L);

		// smoothed positions will be computed in U
		Eigen::MatrixXd U = V;

		for (int k = 0; k < SolveIterations; ++k)
		{
			// Recompute mass matrix on each step
			Eigen::SparseMatrix<double> M;
			igl::massmatrix(U, F, igl::MASSMATRIX_TYPE_BARYCENTRIC, M);

			// Solve (M-delta*L) U = M*U
			const auto& S = (M - Smoothness * L);
			Eigen::SimplicialLLT<Eigen::SparseMatrix<double>> solver(S);
			assert(solver.info() == Eigen::Success);
			U = solver.solve(M * U).eval();
		}
		
		iglext::SetVertexPositions(ResultMesh, U);   // copy updated positions back to FDynamicMesh3
	};

	return MoveTemp(EditFunction);  // return compute lambda
}