#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

namespace RTGUtils
{
	/**
	 * Read mesh in OBJ format from the given path into a FDynamicMesh3.
	 * @param bNormals should normals be imported into primary normal attribute overlay
	 * @param bTexCoords should texture coordinates be imported into primary UV attribute overlay
	 * @param bVertexColors should normals be imported into per-vertex colors
	 * @param bReverseOrientation if true, mesh orientation/normals are flipped. You probably want this for importing to UE4 from other apps.
	 * @param return false if read failed
	 */
	RUNTIMEGEOMETRYUTILS_API bool ReadOBJMesh(
		const FString& Path,
		UE::Geometry::FDynamicMesh3& MeshOut,
		bool bNormals,
		bool bTexCoords,
		bool bVertexColors,
		bool bReverseOrientation);
}

