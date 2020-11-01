#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DynamicMesh3.h"
#include "DynamicMeshAABBTree3.h"
#include "Spatial/FastWinding.h"
#include "DynamicMeshBaseActor.generated.h"


/**
 * Type of Normals computation used by ADynamicMeshBaseActor
 */
UENUM()
enum class EDynamicMeshActorNormalsMode : uint8
{
	SplitNormals = 0,
	PerVertexNormals = 1,
	FaceNormals = 2
};


/**
 * Source of mesh used to initialize ADynamicMeshBaseActor
 */
UENUM()
enum class EDynamicMeshActorSourceType : uint8
{
	Primitive,
	ImportedMesh
};


/**
 * Geometric primitive types supported of by ADynamicMeshBaseActor
 */
UENUM()
enum class EDynamicMeshActorPrimitiveType : uint8
{
	Sphere,
	Box
};


/**
 * Boolean operation types supported of by ADynamicMeshBaseActor
 */
UENUM(BlueprintType)
enum class EDynamicMeshActorBooleanOperation : uint8
{
	Union,
	Subtraction,
	Intersection
};


UENUM(BlueprintType)
enum class EDynamicMeshActorCollisionMode : uint8
{
	NoCollision,
	ComplexAsSimple,
	ComplexAsSimpleAsync
};



/**
 * ADynamicMeshBaseActor is a base class for Actors that support being
 * rebuilt in-game after mesh editing operations. The base Actor itself
 * does not have any Components, it should be used via one of the 
 * DynamicPMCActor, DynamicSMCActor, or DynamicSDMCActor subclasses.
 *
 * ADynamicMeshBaseActor provides a FDynamicMesh3 "Source Mesh", which can
 * be modified via lambdas passed to the EditMesh() function, which will
 * then cause necessary updates to happen to the implementing Components.
 * An AABBTree and FastWindingTree can optionally be enabled with the
 * bEnableSpatialQueries and bEnableInsideQueries flags.
 *
 * When Spatial queries are enabled, a set of UFunctions DistanceToPoint(), 
 * NearestPoint(), ContainsPoint(), and IntersectRay() are available via Blueprints
 * on the relevant Actor. These functions *do not* depend on the UE4 Physics
 * system to work.
 *
 * A small set of mesh modification UFunctions are also available via Blueprints,
 * including BooleanWithMesh(), SolidifyMesh(), SimplifyMeshToTriCount(), and 
 * CopyFromMesh().
 *
 * Meshes can be read from OBJ files either using the ImportedMesh type for
 * the SourceType property, or by calling the ImportMesh() UFunction from a Blueprint.
 * Note that calling this in a Construction Script will be problematic in the Editor
 * as the OBJ will be re-read any time the Actor is modified (including translated/rotated).
 *
 * Any Material set on the subclass Components will be overriden by the Material property.
 *
 *
 */
UCLASS(Abstract)
class RUNTIMEGEOMETRYUTILS_API ADynamicMeshBaseActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynamicMeshBaseActor();


public:
	/** Type of mesh used to initialize this Actor - either a generated mesh Primitive or an Imported OBJ file */
	UPROPERTY(EditAnywhere, Category = MeshOptions)
	EDynamicMeshActorSourceType SourceType = EDynamicMeshActorSourceType::Primitive;

	/** Type of normals computed for the Mesh */
	UPROPERTY(EditAnywhere, Category = MeshOptions)
	EDynamicMeshActorNormalsMode NormalsMode = EDynamicMeshActorNormalsMode::SplitNormals;

	/** Material assigned to child Components created by subclasses */
	UPROPERTY(EditAnywhere, Category = MaterialOptions)
	UMaterialInterface* Material;

	/** If true, mesh will be regenerated or re-imported on tick. Can be useful for prototyping procedural animation, but not the most efficient way to do it */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	bool bRegenerateOnTick = false;

	//
	// Parameters for SourceType = Imported
	// 

	/** Path to OBJ file read to initialize mesh in SourceType=Imported mode */
	UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::ImportedMesh", EditConditionHides))
	FString ImportPath;

	/** Whether the imported mesh should have it's triangles reversed (commonly required for meshes authored in DCC tools) */
	UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::ImportedMesh", EditConditionHides))
	bool bReverseOrientation = true;

	/** If true the imported mesh will be recentered around the origin */
	UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::ImportedMesh", EditConditionHides))
	bool bCenterPivot = true;

	/** Uniform scaling applied to the imported mesh (baked into the mesh vertices, not the actor Transform) */
	UPROPERTY(EditAnywhere, Category = ImportOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::ImportedMesh", EditConditionHides))
	float ImportScale = 1.0;


	//
	// Parameters for SourceType = Primitive
	//

	/** Type of generated mesh primitive */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	EDynamicMeshActorPrimitiveType PrimitiveType = EDynamicMeshActorPrimitiveType::Box;

	/** Triangle density of generated primitive */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (UIMin = 0, UIMax = 50, EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	int TessellationLevel = 8;

	/** Radius of generated sphere / Width of generated box */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (UIMin = 0, EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	float MinimumRadius = 50;

	/** Multiplier on MinimumRadius used to define box depth (ie dimension on Z axis) */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (UIMin = 0.01, UIMax = 1.0, EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive && PrimitiveType == EDynamicMeshActorPrimitiveType::Box", EditConditionHides))
	float BoxDepthRatio = 1.0;

	/** A random value in range [-VariableRadius, VariableRadius] is added to MinimumRadius */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (UIMin = 0, EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	float VariableRadius = 0;

	/** Speed of variation of VariableRadius, only has an effect when bRegenerateOnTick is true */
	UPROPERTY(EditAnywhere, Category = PrimitiveOptions, meta = (UIMin = 0, EditCondition = "SourceType == EDynamicMeshActorSourceType::Primitive", EditConditionHides))
	float PulseSpeed = 3.0;



	//
	// ADynamicMeshBaseActor API
	//
public:

	/**
	 * Call EditMesh() to safely modify the SourceMesh owned by this Actor.
	 * Your EditFunc will be called with the Current SourceMesh as argument,
	 * and you are expected to pass back the new/modified version.
	 * (If you are generating an entirely new mesh, MoveTemp can be used to do this without a copy)
	 */
	virtual void EditMesh(TFunctionRef<void(FDynamicMesh3&)> EditFunc);

	/**
	 * Get a copy of the current SourceMesh stored in MeshOut
	 */
	virtual void GetMeshCopy(FDynamicMesh3& MeshOut);

	/**
	 * Get a reference to the current SourceMesh
	 */
	virtual const FDynamicMesh3& GetMeshRef() const;


	/**
	 * This delegate is broadcast whenever the internal SourceMesh is updated
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMeshModified, ADynamicMeshBaseActor*);
	FOnMeshModified OnMeshModified;

protected:

	/** The SourceMesh used to initialize the mesh Components in the various subclasses */
	FDynamicMesh3 SourceMesh;

	/** Accumulated time since Actor was created, this is used for the animated primitives when bRegenerateOnTick = true*/
	double AccumulatedTime = 0;

	/** Called whenever the initial Source mesh needs to be regenerated / re-imported. Calls EditMesh() to do so. */
	virtual void OnMeshGenerationSettingsModified();

	/** Called to generate or import a new source mesh. Override this to provide your own generated mesh. */
	virtual void RegenerateSourceMesh(FDynamicMesh3& MeshOut);

	/** Call this on a Mesh to compute normals according to the NormalsMode setting */
	virtual void RecomputeNormals(FDynamicMesh3& MeshOut);




	//
	// Support for AABBTree / Spatial Queries
	//
public:
	UPROPERTY(EditAnywhere, Category = SpatialQueryOptions)
	bool bEnableSpatialQueries = false;

	UPROPERTY(EditAnywhere, Category = SpatialQueryOptions)
	bool bEnableInsideQueries = false;

protected:
	// This AABBTree is updated each time SourceMesh is modified if bEnableSpatialQueries=true or bEnableInsideQueries=true
	FDynamicMeshAABBTree3 MeshAABBTree;
	// This FastWindingTree is updated each time SourceMesh is modified if bEnableInsideQueries=true
	TUniquePtr<TFastWindingTree<FDynamicMesh3>> FastWinding;


	//
	// Support for Runtime-Generated Collision
	//
public:
	UPROPERTY(EditAnywhere, Category = RuntimeCollisionOptions)
	EDynamicMeshActorCollisionMode CollisionMode = EDynamicMeshActorCollisionMode::NoCollision;


	//
	// ADynamicMeshBaseActor API that subclasses must implement.
	//
protected:
	/**
	 * Called when the SourceMesh has been modified. Subclasses override this function to
	 * update their respective Component with the new SourceMesh.
	 */
	virtual void OnMeshEditedInternal();




	//
	// Standard UE4 Actor Callbacks. If you need to override these functions,
	// make sure to call (eg) Super::Tick() or you will break the mesh updating functionality.
	//
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostLoad() override;
	virtual void PostActorCreated() override;

#if WITH_EDITOR
	// called when property is modified. This will call OnMeshGenerationSettingsModified() to update the mesh
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;




	//
	// Blueprint API
	//


public:
	/** 
	 * Update SourceMesh by reading external mesh file at Path. Optionally flip orientation and recompute normals. 
	 * Note: Path may be relative to Content folder, otherwise it must be an absolute path.
	 * @return false if mesh read failed
	 */
	UFUNCTION(BlueprintCallable)
	bool ImportMesh(FString Path, bool bFlipOrientation, bool bRecomputeNormals);

	/** Copy the SourceMesh of OtherMesh into our SourceMesh, and optionally recompute normals */
	UFUNCTION(BlueprintCallable)
	void CopyFromMesh(ADynamicMeshBaseActor* OtherMesh, bool bRecomputeNormals);



	//
	// Mesh Spatial Queries API
	//
public:
	/**
	 * Find NearestMeshWorldPoint on SourceMesh to WorldPoint, as well as NearestTriangle ID and barycentric coordinates of NearestMeshWorldPoint in triangle
	 * @return distance to point
	 */
	UFUNCTION(BlueprintCallable)
	float DistanceToPoint(FVector WorldPoint, FVector& NearestMeshWorldPoint, int& NearestTriangle, FVector& TriBaryCoords);

	/**
	 * @return nearest world-space point on SourceMesh to WorldPoint
	 */
	UFUNCTION(BlueprintCallable)
	FVector NearestPoint(FVector WorldPoint);

	/**
	 * @return true if mesh contains WorldPoint, which is defined as the mesh winding number being >= WindingThreshold
	 */
	UFUNCTION(BlueprintCallable)
	bool ContainsPoint(FVector WorldPoint, float WindingThreshold = 0.5);

	/**
	 * Calculate intersection of given 3D World-Space ray defined by (RayOrigin,RayDirection) with the SourceMesh.
	 * If hit, returns WorldHitPoint position, distance along ray in HitDistance, NearestTriangle ID, and barycentric coordinates of hit point in triangle
	 * Pass MaxDistance > 0 to limit the allowable ray-hit distance
	 * @return true if hit is found
	 */
	UFUNCTION(BlueprintCallable)
	bool IntersectRay(FVector RayOrigin, FVector RayDirection, FVector& WorldHitPoint, float& HitDistance, int& NearestTriangle, FVector& TriBaryCoords, float MaxDistance = 0);



	//
	// Mesh Modification API
	//
public:

	/** Compute the specified a Boolean operation with OtherMesh (transformed to world space) and store in our SourceMesh */
	UFUNCTION(BlueprintCallable)
	void BooleanWithMesh(ADynamicMeshBaseActor* OtherMesh, EDynamicMeshActorBooleanOperation Operation);

	/** Subtract OtherMesh from our SourceMesh */
	UFUNCTION(BlueprintCallable)
	void SubtractMesh(ADynamicMeshBaseActor* OtherMesh);

	/** Union OtherMesh with our SourceMesh */
	UFUNCTION(BlueprintCallable)
	void UnionWithMesh(ADynamicMeshBaseActor* OtherMesh);

	/** Intersect OtherMesh with our SourceMesh */
	UFUNCTION(BlueprintCallable)
	void IntersectWithMesh(ADynamicMeshBaseActor* OtherMesh);

	/** Create a "solid" verison of SourceMesh by voxelizing with the fast winding number at the given grid resolution */
	UFUNCTION(BlueprintCallable)
	void SolidifyMesh(int VoxelResolution = 64, float WindingThreshold = 0.5);

	/** Simplify current SourceMesh to the target triangle count */
	UFUNCTION(BlueprintCallable)
	void SimplifyMeshToTriCount(int32 TargetTriangleCount);

public:
	/** @return number of triangles in current SourceMesh */
	UFUNCTION(BlueprintCallable)
	int GetTriangleCount();
};
