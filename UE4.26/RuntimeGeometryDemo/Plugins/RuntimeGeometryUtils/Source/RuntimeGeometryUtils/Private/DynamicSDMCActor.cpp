#include "DynamicSDMCActor.h"
#include "DynamicMesh3.h"


// Sets default values
ADynamicSDMCActor::ADynamicSDMCActor()
{
	MeshComponent = CreateDefaultSubobject<USimpleDynamicMeshComponent>(TEXT("MeshComponent"), false);
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void ADynamicSDMCActor::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void ADynamicSDMCActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ADynamicSDMCActor::OnMeshEditedInternal()
{
	UpdateSDMCMesh();
	Super::OnMeshEditedInternal();
}

void ADynamicSDMCActor::UpdateSDMCMesh()
{
	if (MeshComponent)
	{
		*(MeshComponent->GetMesh()) = SourceMesh;
		MeshComponent->NotifyMeshUpdated();

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		MeshComponent->SetMaterial(0, UseMaterial);
	}
}