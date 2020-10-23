#include "DynamicPMCActor.h"
#include "MeshComponentRuntimeUtils.h"
#include "DynamicMesh3.h"


// Sets default values
ADynamicPMCActor::ADynamicPMCActor()
{
	MeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"), false);
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void ADynamicPMCActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADynamicPMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ADynamicPMCActor::OnMeshEditedInternal()
{
	UpdatePMCMesh();
	Super::OnMeshEditedInternal();
}

void ADynamicPMCActor::UpdatePMCMesh()
{
	if (MeshComponent)
	{
		bool bUseFaceNormals = (this->NormalsMode == EDynamicMeshActorNormalsMode::FaceNormals);
		bool bUseUV0 = true;
		bool bUseVertexColors = false;
		RTGUtils::UpdatePMCFromDynamicMesh_SplitTriangles(MeshComponent, &SourceMesh, bUseFaceNormals, bUseUV0, bUseVertexColors);

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}