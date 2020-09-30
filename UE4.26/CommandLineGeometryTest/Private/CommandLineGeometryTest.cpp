
// required UE headers
#include "CommandLineGeometryTest.h"
#include "RequiredProgramMainCPPInclude.h"

// GeometricObjects types for vector math, point sets, etc
#include "VectorTypes.h"
#include "BoxTypes.h"
#include "OrientedBoxTypes.h"
#include "FrameTypes.h"
#include "TransformTypes.h"
#include "Sampling/SphericalFibonacci.h"

// FDynamicMesh3 class and attribute types
#include "DynamicMesh3.h"
#include "DynamicMeshAttributeSet.h"

// Mesh Generators
#include "Generators/MinimalBoxMeshGenerator.h"
#include "Generators/SphereGenerator.h"
#include "Generators/GridBoxMeshGenerator.h"

// Mesh Utilities 
#include "MeshQueries.h"
#include "MeshNormals.h"
#include "MeshTransforms.h"
#include "MeshBoundaryLoops.h"

// Spatial Data Structures
#include "DynamicMeshAABBTree3.h"
#include "Spatial/FastWinding.h"

// Simplification/Remeshing
#include "MeshSimplification.h"
#include "QueueRemesher.h"
#include "MeshConstraintsUtil.h"
#include "ProjectionTargets.h"

// Mesh-based Implicit Surfaces/Operations
#include "Implicit/Solidify.h"
#include "Implicit/Morphology.h"

// Mesh Editing
#include "DynamicMeshEditor.h"
#include "Operations/MeshBoolean.h"
#include "Operations/MinimalHoleFiller.h"

// Mesh I/O  (local to this project, not part of GeometryProcessing)
#include "DynamicMeshOBJReader.h"
#include "DynamicMeshOBJWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogGeometryTest, Log, All);
IMPLEMENT_APPLICATION(CommandLineGeometryTest, "CommandLineGeometryTest");



INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
	GEngineLoop.PreInit(ArgC, ArgV);

	// import an OBJ mesh. The path below is relative to the default path that Visual Studio will execute CommandLineGeometryTest.exe,
	// when using a normal UE4.26 auto-generated UE.sln file. If things change you might need to update this path
	FDynamicMesh3 ImportMesh;
	FDynamicMeshOBJReader::Read("..\\..\\Source\\Programs\\CommandLineGeometryTest\\HoleyBunny.obj", ImportMesh, true, true, true);
	// flip to UE orientation
	ImportMesh.ReverseOrientation();

	// print some mesh stats
	UE_LOG(LogGeometryTest, Display, TEXT("Mesh has %d vertices, %d triangles, %d edges"), ImportMesh.VertexCount(), ImportMesh.TriangleCount(), ImportMesh.EdgeCount());
	UE_LOG(LogGeometryTest, Display, TEXT("Mesh has %d normals"), ImportMesh.Attributes()->PrimaryNormals()->ElementCount());
	UE_LOG(LogGeometryTest, Display, TEXT("Mesh has %d UVs"), ImportMesh.Attributes()->PrimaryUV()->ElementCount());

	// compute per-vertex normals
	FMeshNormals::QuickComputeVertexNormals(ImportMesh);

	// generate a small box mesh to append multiple times
	FAxisAlignedBox3d ImportBounds = ImportMesh.GetBounds();
	double ImportRadius = ImportBounds.DiagonalLength() * 0.5;
	FMinimalBoxMeshGenerator SmallBoxGen;
	SmallBoxGen.Box = FOrientedBox3d(FVector3d::Zero(), ImportRadius * 0.05 * FVector3d::One());
	FDynamicMesh3 SmallBoxMesh(&SmallBoxGen.Generate());

	// create a bounding-box tree, then copy the imported mesh and make an Editor for it
	FDynamicMeshAABBTree3 ImportBVTree(&ImportMesh);
	FDynamicMesh3 AccumMesh(ImportMesh);
	FDynamicMeshEditor MeshEditor(&AccumMesh);

	// append the small box mesh a bunch of times, at random-ish locations, based on a Spherical Fibonacci distribution
	TSphericalFibonacci<double> PointGen(64);
	for (int32 k = 0; k < PointGen.Num(); ++k)
	{
		// point on a bounding sphere
		FVector3d Point = (ImportRadius * PointGen.Point(k)) + ImportBounds.Center();

		// compute the nearest point on the imported mesh
		double NearDistSqr;
		int32 NearestTriID = ImportBVTree.FindNearestTriangle(Point, NearDistSqr);
		if (ImportMesh.IsTriangle(NearestTriID) == false)
			continue;
		FDistPoint3Triangle3d DistQueryResult = TMeshQueries<FDynamicMesh3>::TriangleDistance(ImportMesh, NearestTriID, Point);

		// compute the intersection between the imported mesh and a ray from the point to the mesh center
		FRay3d RayToCenter(Point, (ImportBounds.Center() - Point).Normalized() );
		int32 HitTriID = ImportBVTree.FindNearestHitTriangle(RayToCenter);
		if (HitTriID == FDynamicMesh3::InvalidID)
			continue;
		FIntrRay3Triangle3d HitQueryResult = TMeshQueries<FDynamicMesh3>::TriangleIntersection(ImportMesh, HitTriID, RayToCenter);

		// pick the closer point
		bool bUseRayIntersection = (HitQueryResult.RayParameter < DistQueryResult.Get());
		FVector3d UsePoint = (bUseRayIntersection) ? RayToCenter.PointAt(HitQueryResult.RayParameter) : DistQueryResult.ClosestTrianglePoint;

		FVector3d TriBaryCoords = (bUseRayIntersection) ? HitQueryResult.TriangleBaryCoords : DistQueryResult.TriangleBaryCoords;
		FVector3d UseNormal = ImportMesh.GetTriBaryNormal(NearestTriID, TriBaryCoords.X, TriBaryCoords.Y, TriBaryCoords.Z);

		// position/orientation to use to append the box
		FFrame3d TriFrame(UsePoint, UseNormal);

		// append the box via the Editor
		FMeshIndexMappings TmpMappings;
		MeshEditor.AppendMesh(&SmallBoxMesh, TmpMappings,
			[TriFrame](int32 vid, const FVector3d& Vertex) { return TriFrame.FromFramePoint(Vertex); },
			[TriFrame](int32 vid, const FVector3d& Normal) { return TriFrame.FromFrameVector(Normal); });
	}

	// make a new AABBTree for the accumulated mesh-with-boxes
	FDynamicMeshAABBTree3 AccumMeshBVTree(&AccumMesh);
	// build a fast-winding-number evaluation data structure
	TFastWindingTree<FDynamicMesh3> FastWinding(&AccumMeshBVTree);

	// "solidify" the mesh by extracting an iso-surface of the fast-winding field, using marching cubes
	// (this all happens inside TImplicitSolidify)
	int32 TargetVoxelCount = 64;
	double ExtendBounds = 2.0;
	TImplicitSolidify<FDynamicMesh3> SolidifyCalc(&AccumMesh, &AccumMeshBVTree, &FastWinding);
	SolidifyCalc.SetCellSizeAndExtendBounds(AccumMeshBVTree.GetBoundingBox(), ExtendBounds, TargetVoxelCount);
	SolidifyCalc.WindingThreshold = 0.5;
	SolidifyCalc.SurfaceSearchSteps = 5;
	SolidifyCalc.bSolidAtBoundaries = true;
	SolidifyCalc.ExtendBounds = ExtendBounds;
	FDynamicMesh3 SolidMesh(&SolidifyCalc.Generate());
	// position the mesh to the right of the imported mesh
	MeshTransforms::Translate(SolidMesh, SolidMesh.GetBounds().Width() * FVector3d::UnitX());

	// offset the solidified mesh
	double OffsetDistance = ImportRadius * 0.1;
	TImplicitMorphology<FDynamicMesh3> ImplicitMorphology;
	ImplicitMorphology.MorphologyOp = TImplicitMorphology<FDynamicMesh3>::EMorphologyOp::Dilate;
	ImplicitMorphology.Source = &SolidMesh;
	FDynamicMeshAABBTree3 SolidSpatial(&SolidMesh);
	ImplicitMorphology.SourceSpatial = &SolidSpatial;
	ImplicitMorphology.SetCellSizesAndDistance(SolidMesh.GetCachedBounds(), OffsetDistance, 64, 64);
	FDynamicMesh3 OffsetSolidMesh(&ImplicitMorphology.Generate());

	// simplify the offset mesh
	FDynamicMesh3 SimplifiedSolidMesh(OffsetSolidMesh);
	FQEMSimplification Simplifier(&SimplifiedSolidMesh);
	Simplifier.SimplifyToTriangleCount(5000);
	// position to the right
	MeshTransforms::Translate(SimplifiedSolidMesh, SimplifiedSolidMesh.GetBounds().Width() * FVector3d::UnitX());

	// generate a sphere mesh
	FSphereGenerator SphereGen;
	SphereGen.Radius = ImportMesh.GetBounds().MaxDim() * 0.6;
	SphereGen.NumPhi = SphereGen.NumTheta = 10;
	SphereGen.bPolygroupPerQuad = true;
	SphereGen.Generate();
	FDynamicMesh3 SphereMesh(&SphereGen);

	// generate a box mesh
	FGridBoxMeshGenerator BoxGen;
	BoxGen.Box = FOrientedBox3d(FVector3d::Zero(), SphereGen.Radius * FVector3d::One());
	BoxGen.EdgeVertices = FIndex3i(4, 5, 6);
	BoxGen.bPolygroupPerQuad = false;
	BoxGen.Generate();
	FDynamicMesh3 BoxMesh(&BoxGen);

	// subtract the box from the sphere (the box is transformed within the FMeshBoolean)
	FDynamicMesh3 BooleanResult;
	FMeshBoolean DifferenceOp(
		&SphereMesh, FTransform3d::Identity(),
		&BoxMesh, FTransform3d(FQuaterniond(FVector3d::UnitY(), 45.0, true), SphereGen.Radius*FVector3d(1,-1,1)),
		&BooleanResult, FMeshBoolean::EBooleanOp::Difference);
	if (DifferenceOp.Compute() == false)
	{
		UE_LOG(LogGeometryTest, Display, TEXT("Boolean Failed!"));
	}
	FAxisAlignedBox3d BooleanBBox = BooleanResult.GetBounds();
	MeshTransforms::Translate(BooleanResult, 
		(SimplifiedSolidMesh.GetBounds().Max.X + 0.6*BooleanBBox.Width())* FVector3d::UnitX() + 0.5*BooleanBBox.Height()*FVector3d::UnitZ());

	// make a copy of the boolean mesh, and apply Remeshing
	FDynamicMesh3 RemeshBoolMesh(BooleanResult);
	RemeshBoolMesh.DiscardAttributes();
	FQueueRemesher Remesher(&RemeshBoolMesh);
	Remesher.SetTargetEdgeLength(ImportRadius * 0.05);
	Remesher.SmoothSpeedT = 0.5;
	Remesher.FastestRemesh();
	MeshTransforms::Translate(RemeshBoolMesh, 1.1*RemeshBoolMesh.GetBounds().Width() * FVector3d::UnitX());

	// subtract the remeshed sphere from the offset-solidified-cubesbunny
	FDynamicMesh3 FinalBooleanResult;
	FMeshBoolean FinalDifferenceOp(
		&SimplifiedSolidMesh, FTransform3d(-SimplifiedSolidMesh.GetBounds().Center()),
		&RemeshBoolMesh, FTransform3d( (-RemeshBoolMesh.GetBounds().Center()) + 0.5*ImportRadius*FVector3d(0.0,0,0) ),
		&FinalBooleanResult, FMeshBoolean::EBooleanOp::Intersect);
	FinalDifferenceOp.Compute();

	// The boolean probably has some small cracks around the border, find them and fill them
	FMeshBoundaryLoops LoopsCalc(&FinalBooleanResult);
	UE_LOG(LogGeometryTest, Display, TEXT("Final Boolean Mesh has %d holes"), LoopsCalc.GetLoopCount());
	for (const FEdgeLoop& Loop : LoopsCalc.Loops)
	{
		FMinimalHoleFiller Filler(&FinalBooleanResult, Loop);
		Filler.Fill();
	}
	FAxisAlignedBox3d FinalBooleanBBox = FinalBooleanResult.GetBounds();
	MeshTransforms::Translate(FinalBooleanResult,
		(RemeshBoolMesh.GetBounds().Max.X + 0.6*FinalBooleanBBox.Width())*FVector3d::UnitX() + 0.5*FinalBooleanBBox.Height()*FVector3d::UnitZ() );

	// write out the sequence of meshes
	FDynamicMeshOBJWriter::Write("..\\..\\Source\\Programs\\CommandLineGeometryTest\\HoleyBunny_processed.obj", 
		{ AccumMesh, SolidMesh, SimplifiedSolidMesh, BooleanResult, RemeshBoolMesh, FinalBooleanResult }, true);

	return 0;  // done!
}
