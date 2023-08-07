#include "DynamicMeshBaseActor.h"

#include "Generators/SphereGenerator.h"
#include "Generators/GridBoxMeshGenerator.h"
#include "MeshQueries.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/MeshNormals.h"
#include "DynamicMesh/MeshTransforms.h"
#include "MeshSimplification.h"
#include "Operations/MeshBoolean.h"
#include "Implicit/Solidify.h"

#include "DynamicMeshOBJReader.h"

// Sets default values
ADynamicMeshBaseActor::ADynamicMeshBaseActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AccumulatedTime = 0;
	MeshAABBTree.SetMesh(&SourceMesh);

	FastWinding = MakeUnique<UE::Geometry::TFastWindingTree<UE::Geometry::FDynamicMesh3>>(&MeshAABBTree, false);
}

void ADynamicMeshBaseActor::PostLoad()
{
	Super::PostLoad();
	OnMeshGenerationSettingsModified();
}


void ADynamicMeshBaseActor::PostActorCreated()
{
	Super::PostActorCreated();
	OnMeshGenerationSettingsModified();
}



// Called when the game starts or when spawned
void ADynamicMeshBaseActor::BeginPlay()
{
	Super::BeginPlay();

	AccumulatedTime = 0;
	OnMeshGenerationSettingsModified();
}

// Called every frame
void ADynamicMeshBaseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AccumulatedTime += DeltaTime;
	if (bRegenerateOnTick && SourceType == EDynamicMeshActorSourceType::Primitive)
	{
		OnMeshGenerationSettingsModified();
	}
}


#if WITH_EDITOR
void ADynamicMeshBaseActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnMeshGenerationSettingsModified();
}
#endif


void ADynamicMeshBaseActor::EditMesh(TFunctionRef<void(UE::Geometry::FDynamicMesh3&)> EditFunc)
{
	EditFunc(SourceMesh);

	// update spatial data structures
	if (bEnableSpatialQueries || bEnableInsideQueries)
	{
		MeshAABBTree.Build();
		if (bEnableInsideQueries)
		{
			FastWinding->Build();
		}
	}

	OnMeshEditedInternal();
}


void ADynamicMeshBaseActor::GetMeshCopy(UE::Geometry::FDynamicMesh3& MeshOut)
{
	MeshOut = SourceMesh;
}

const UE::Geometry::FDynamicMesh3& ADynamicMeshBaseActor::GetMeshRef() const
{
	return SourceMesh;
}

void ADynamicMeshBaseActor::OnMeshEditedInternal()
{
	OnMeshModified.Broadcast(this);
}


void ADynamicMeshBaseActor::OnMeshGenerationSettingsModified()
{
	EditMesh([this](UE::Geometry::FDynamicMesh3& MeshToUpdate) {
		RegenerateSourceMesh(MeshToUpdate);
		});
}


void ADynamicMeshBaseActor::RegenerateSourceMesh(UE::Geometry::FDynamicMesh3& MeshOut)
{
	if (SourceType == EDynamicMeshActorSourceType::Primitive)
	{
		double UseRadius = (this->MinimumRadius + this->VariableRadius)
			+ (this->VariableRadius) * FMathd::Sin(PulseSpeed * AccumulatedTime);

		// generate new mesh
		if (this->PrimitiveType == EDynamicMeshActorPrimitiveType::Sphere)
		{
			UE::Geometry::FSphereGenerator SphereGen;
			SphereGen.NumPhi = SphereGen.NumTheta = FMath::Clamp(this->TessellationLevel, 3, 50);
			SphereGen.Radius = UseRadius;
			MeshOut.Copy(&SphereGen.Generate());
		}
		else
		{
			UE::Geometry::FGridBoxMeshGenerator BoxGen;
			int TessLevel = FMath::Clamp(this->TessellationLevel, 2, 50);
			BoxGen.EdgeVertices = UE::Geometry::FIndex3i(TessLevel, TessLevel, TessLevel);
			FVector3d BoxExtents = UseRadius * FVector3d::One();
			BoxExtents.Z *= BoxDepthRatio;
			BoxGen.Box = UE::Geometry::FOrientedBox3d(FVector3d::Zero(), BoxExtents);
			MeshOut.Copy(&BoxGen.Generate());
		}

	}
	else if (SourceType == EDynamicMeshActorSourceType::ImportedMesh)
	{
		FString UsePath = ImportPath;
		if (FPaths::FileExists(UsePath) == false && FPaths::IsRelative(UsePath))
		{
			UsePath = FPaths::ProjectContentDir() + ImportPath;
		}

		MeshOut = UE::Geometry::FDynamicMesh3();
		if (!RTGUtils::ReadOBJMesh(UsePath, MeshOut, true, true, true, bReverseOrientation))
		{
			UE_LOG(LogTemp, Warning, TEXT("Error reading mesh file %s"), *UsePath);
			UE::Geometry::FSphereGenerator SphereGen;
			SphereGen.NumPhi = SphereGen.NumTheta = 8;
			SphereGen.Radius = this->MinimumRadius;
			MeshOut.Copy(&SphereGen.Generate());
		}

		if (bCenterPivot)
		{
			MeshTransforms::Translate(MeshOut, -MeshOut.GetBounds().Center());
		}

		if (ImportScale != 1.0)
		{
			MeshTransforms::Scale(MeshOut, ImportScale * FVector3d::One(), FVector3d::Zero());
		}

		MeshTransforms::ApplyTransform(MeshOut, ImportTransform);
	}

	RecomputeNormals(MeshOut);
}







void ADynamicMeshBaseActor::RecomputeNormals(UE::Geometry::FDynamicMesh3& MeshOut)
{
	if (this->NormalsMode == EDynamicMeshActorNormalsMode::PerVertexNormals)
	{
		MeshOut.EnableAttributes();
		UE::Geometry::FMeshNormals::InitializeOverlayToPerVertexNormals(MeshOut.Attributes()->PrimaryNormals(), false);
	}
	else if (this->NormalsMode == EDynamicMeshActorNormalsMode::FaceNormals)
	{
		MeshOut.EnableAttributes();
		UE::Geometry::FMeshNormals::InitializeOverlayToPerTriangleNormals(MeshOut.Attributes()->PrimaryNormals());
	}
}



int ADynamicMeshBaseActor::GetTriangleCount()
{
	return SourceMesh.TriangleCount();
}


float ADynamicMeshBaseActor::DistanceToPoint(FVector WorldPoint, FVector& NearestWorldPoint, int& NearestTriangle, FVector& TriBaryCoords)
{
	NearestWorldPoint = WorldPoint;
	NearestTriangle = -1;
	if (bEnableSpatialQueries == false)
	{
		return TNumericLimits<float>::Max();
	}

	FTransform3d ActorToWorld(GetActorTransform());
	FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);

	double NearDistSqr;
	NearestTriangle = MeshAABBTree.FindNearestTriangle(LocalPoint, NearDistSqr);
	if (NearestTriangle < 0)
	{
		return TNumericLimits<float>::Max();
	}

	UE::Geometry::FDistPoint3Triangle3d DistQuery = UE::Geometry::TMeshQueries<UE::Geometry::FDynamicMesh3>::TriangleDistance(SourceMesh, NearestTriangle, LocalPoint);
	NearestWorldPoint = (FVector)ActorToWorld.TransformPosition(DistQuery.ClosestTrianglePoint);
	TriBaryCoords = (FVector)DistQuery.TriangleBaryCoords;
	return (float)FMathd::Sqrt(NearDistSqr);
}


FVector ADynamicMeshBaseActor::NearestPoint(FVector WorldPoint)
{
	if (bEnableSpatialQueries)
	{
		FTransform3d ActorToWorld(GetActorTransform());
		FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);
		return (FVector)ActorToWorld.TransformPosition(MeshAABBTree.FindNearestPoint(LocalPoint));
	}
	return WorldPoint;
}

bool ADynamicMeshBaseActor::ContainsPoint(FVector WorldPoint, float WindingThreshold)
{
	if (bEnableInsideQueries)
	{
		FTransform3d ActorToWorld(GetActorTransform());
		FVector3d LocalPoint = ActorToWorld.InverseTransformPosition((FVector3d)WorldPoint);
		return FastWinding->IsInside(LocalPoint, WindingThreshold);
	}
	return false;
}


bool ADynamicMeshBaseActor::IntersectRay(FVector RayOrigin, FVector RayDirection,
	FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords,
	float MaxDistance)
{
	if (bEnableSpatialQueries)
	{
		UE::Geometry::TTransformSRT3<double> ActorToWorld(GetActorTransform());
		FVector3d WorldDirection(RayDirection); WorldDirection.Normalize();
		FRay3d LocalRay(ActorToWorld.InverseTransformPosition((FVector3d)RayOrigin),
			ActorToWorld.InverseTransformNormal(WorldDirection));
		UE::Geometry::IMeshSpatial::FQueryOptions QueryOptions;
		if (MaxDistance > 0)
		{
			QueryOptions.MaxDistance = MaxDistance;
		}
		NearestTriangle = MeshAABBTree.FindNearestHitTriangle(LocalRay, QueryOptions);
		if (SourceMesh.IsTriangle(NearestTriangle))
		{
			UE::Geometry::FIntrRay3Triangle3d IntrQuery = UE::Geometry::TMeshQueries<UE::Geometry::FDynamicMesh3>::TriangleIntersection(SourceMesh, NearestTriangle, LocalRay);
			if (IntrQuery.IntersectionType == EIntersectionType::Point)
			{
				HitDistance = IntrQuery.RayParameter;
				WorldHitPoint = (FVector)ActorToWorld.TransformPosition(LocalRay.PointAt(IntrQuery.RayParameter));
				TriBaryCoords = (FVector)IntrQuery.TriangleBaryCoords;
				return true;
			}
		}
	}
	return false;
}




void ADynamicMeshBaseActor::SubtractMesh(ADynamicMeshBaseActor* OtherMeshActor)
{
	BooleanWithMesh(OtherMeshActor, EDynamicMeshActorBooleanOperation::Subtraction);
}
void ADynamicMeshBaseActor::UnionWithMesh(ADynamicMeshBaseActor* OtherMeshActor)
{
	BooleanWithMesh(OtherMeshActor, EDynamicMeshActorBooleanOperation::Union);
}
void ADynamicMeshBaseActor::IntersectWithMesh(ADynamicMeshBaseActor* OtherMeshActor)
{
	BooleanWithMesh(OtherMeshActor, EDynamicMeshActorBooleanOperation::Intersection);
}


void ADynamicMeshBaseActor::BooleanWithMesh(ADynamicMeshBaseActor* OtherMeshActor, EDynamicMeshActorBooleanOperation Operation)
{
	if (ensure(OtherMeshActor) == false) return;

	FTransform3d ActorToWorld(GetActorTransform());
	FTransform3d OtherToWorld(OtherMeshActor->GetActorTransform());

	UE::Geometry::FDynamicMesh3 OtherMesh;
	OtherMeshActor->GetMeshCopy(OtherMesh);
	MeshTransforms::ApplyTransform(OtherMesh, OtherToWorld);
	MeshTransforms::ApplyTransformInverse(OtherMesh, ActorToWorld);

	EditMesh([&](UE::Geometry::FDynamicMesh3& MeshToUpdate) {

		UE::Geometry::FDynamicMesh3 ResultMesh;

		UE::Geometry::FMeshBoolean::EBooleanOp ApplyOp = UE::Geometry::FMeshBoolean::EBooleanOp::Union;
		switch (Operation)
		{
		default:
			break;
		case EDynamicMeshActorBooleanOperation::Subtraction:
			ApplyOp = UE::Geometry::FMeshBoolean::EBooleanOp::Difference;
			break;
		case EDynamicMeshActorBooleanOperation::Intersection:
			ApplyOp = UE::Geometry::FMeshBoolean::EBooleanOp::Intersect;
			break;
		}

		UE::Geometry::FMeshBoolean Boolean(
			&MeshToUpdate, FTransform3d::Identity,
			&OtherMesh, FTransform3d::Identity,
			&ResultMesh,
			ApplyOp);
		Boolean.bPutResultInInputSpace = true;
		bool bOK = Boolean.Compute();

		if (!bOK)
		{
			// fill holes
		}

		RecomputeNormals(ResultMesh);

		MeshToUpdate = MoveTemp(ResultMesh);
		});
}



bool ADynamicMeshBaseActor::ImportMesh(FString Path, bool bFlipOrientation, bool bRecomputeNormals)
{
	UE::Geometry::FDynamicMesh3 ImportedMesh;
	if (!RTGUtils::ReadOBJMesh(Path, ImportedMesh, true, true, true, bFlipOrientation))
	{
		UE_LOG(LogTemp, Warning, TEXT("Error reading mesh file %s"), *Path);
		return false;
	}

	if (bRecomputeNormals)
	{
		RecomputeNormals(ImportedMesh);
	}

	EditMesh([&](UE::Geometry::FDynamicMesh3& MeshToUpdate)
		{
			MeshToUpdate = MoveTemp(ImportedMesh);
		});

	return true;
}


void ADynamicMeshBaseActor::CopyFromMesh(ADynamicMeshBaseActor* OtherMesh, bool bRecomputeNormals)
{
	if (!ensure(OtherMesh)) return;

	// the part where we generate a new mesh
	UE::Geometry::FDynamicMesh3 TmpMesh;
	OtherMesh->GetMeshCopy(TmpMesh);

	// apply our normals setting
	if (bRecomputeNormals)
	{
		RecomputeNormals(TmpMesh);
	}

	// update the mesh
	EditMesh([&](UE::Geometry::FDynamicMesh3& MeshToUpdate)
		{
			MeshToUpdate = MoveTemp(TmpMesh);
		});
}




void ADynamicMeshBaseActor::SolidifyMesh(int VoxelResolution, float WindingThreshold)
{
	if (MeshAABBTree.IsValid(false) == false)
	{
		MeshAABBTree.Build();
	}
	if (FastWinding->IsBuilt() == false)
	{
		FastWinding->Build();
	}

	// ugh workaround for bug
	UE::Geometry::FDynamicMesh3 CompactMesh;
	CompactMesh.CompactCopy(SourceMesh, false, false, false, false);
	UE::Geometry::FDynamicMeshAABBTree3 AABBTree(&CompactMesh, true);
	UE::Geometry::TFastWindingTree<UE::Geometry::FDynamicMesh3> Winding(&AABBTree, true);

	double ExtendBounds = 2.0;
	//UE::Geometry::TImplicitSolidify<UE::Geometry::FDynamicMesh3> SolidifyCalc(&SourceMesh, &MeshAABBTree, FastWinding.Get());
	//SolidifyCalc.SetCellSizeAndExtendBounds(MeshAABBTree.GetBoundingBox(), ExtendBounds, VoxelResolution);
	UE::Geometry::TImplicitSolidify<UE::Geometry::FDynamicMesh3> SolidifyCalc(&CompactMesh, &AABBTree, &Winding);
	SolidifyCalc.SetCellSizeAndExtendBounds(AABBTree.GetBoundingBox(), ExtendBounds, VoxelResolution);
	SolidifyCalc.WindingThreshold = WindingThreshold;
	SolidifyCalc.SurfaceSearchSteps = 5;
	SolidifyCalc.bSolidAtBoundaries = true;
	SolidifyCalc.ExtendBounds = ExtendBounds;
	UE::Geometry::FDynamicMesh3 SolidMesh(&SolidifyCalc.Generate());

	SolidMesh.EnableAttributes();
	RecomputeNormals(SolidMesh);

	EditMesh([&](UE::Geometry::FDynamicMesh3& MeshToUpdate)
		{
			MeshToUpdate = MoveTemp(SolidMesh);
		});
}

void ADynamicMeshBaseActor::SimplifyMeshToTriCount(int32 TargetTriangleCount)
{
	TargetTriangleCount = FMath::Max(1, TargetTriangleCount);
	if (TargetTriangleCount >= SourceMesh.TriangleCount()) return;

	// make compacted copy because it seems to change the results?
	UE::Geometry::FDynamicMesh3 SimplifyMesh;
	SimplifyMesh.CompactCopy(SourceMesh, false, false, false, false);
	SimplifyMesh.EnableTriangleGroups();			// workaround for failing check()
	UE::Geometry::FQEMSimplification Simplifier(&SimplifyMesh);
	Simplifier.SimplifyToTriangleCount(TargetTriangleCount);
	SimplifyMesh.EnableAttributes();
	RecomputeNormals(SimplifyMesh);

	EditMesh([&](UE::Geometry::FDynamicMesh3& MeshToUpdate)
		{
			MeshToUpdate.CompactCopy(SimplifyMesh);
		});
}
