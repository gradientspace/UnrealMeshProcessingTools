#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

namespace RTGUtils
{
	/**
	 * Write mesh to the given output path in OBJ format. 
	 * @param bReverseOrientation if true, mesh orientation/normals are flipped. You probably want this for exporting from UE4 to other apps.
	 * @param return false if write failed
	 */
	RUNTIMEGEOMETRYUTILS_API bool WriteOBJMesh(
		const FString& OutputPath,
		const UE::Geometry::FDynamicMesh3& Mesh,
		bool bReverseOrientation);

	/**
	 * Write set of meshes to the given output path in OBJ format.
	 * @param bReverseOrientation if true, mesh orientation/normals are flipped. You probably want this for exporting from UE4 to other apps.
	 * @param return false if write failed
	 */
	RUNTIMEGEOMETRYUTILS_API bool WriteOBJMeshes(
		const FString& OutputPath,
		const TArray<UE::Geometry::FDynamicMesh3>& Meshes,
		bool bReverseOrientation);
}



