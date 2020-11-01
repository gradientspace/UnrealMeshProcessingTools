#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "ProceduralMeshComponent.h"
#include "DynamicMesh3.h"


namespace RTGUtils
{


	/**
	 * Reinitialize the given StaticMesh with the input FDynamicMesh3.
	 * This calls StaticMesh->BuildFromMeshDescriptions(), which can be used at Runtime (vs StaticMesh->Build() which cannot)
	 */
	RUNTIMEGEOMETRYUTILS_API void UpdateStaticMeshFromDynamicMesh(
		UStaticMesh* StaticMesh,
		const FDynamicMesh3* Mesh);



	/**
	 * Initialize a ProceduralMeshComponent with a single section defined by the given FDynamicMesh3.
	 * @param bUseFaceNormals if true, each triangle is shaded with per-triangle normal instead of split-vertex normals from FDynamicMesh3 overlay
	 * @param bInitializeUV0 if true, UV0 is initialized, otherwise it is not (set to 0)
	 * @param bInitializePerVertexColors if true, per-vertex colors on the FDynamicMesh3 are used to initialize vertex colors of the PMC
	 */
	RUNTIMEGEOMETRYUTILS_API void UpdatePMCFromDynamicMesh_SplitTriangles(
		UProceduralMeshComponent* Component, 
		const FDynamicMesh3* Mesh,
		bool bUseFaceNormals,
		bool bInitializeUV0,
		bool bInitializePerVertexColors,
		bool bCreateCollision);

}