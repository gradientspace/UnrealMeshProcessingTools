#include "DynamicSDMCActor.h"
#include "DynamicMesh/DynamicMesh3.h"


// Sets default values
ADynamicSDMCActor::ADynamicSDMCActor()
{
	MeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("MeshComponent"), false);
	bool result = SetRootComponent(MeshComponent);

	if (!result) {
		UE_LOG(LogTemp, Error, TEXT("fadasd"));
	}
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


//void ADynamicSDMCActor::PostSpawnInitialize(FTransform const& UserSpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction, ESpawnActorScaleMethod TransformScaleMethod) {
//	UE_LOG(LogTemp, Warning, TEXT("ADynamicSDMCActor::PostSpawnInitialize MeshComponent: %p"), MeshComponent);
//	Super::PostSpawnInitialize(UserSpawnTransform, InOwner, InInstigator, bRemoteOwned, bNoFail, bDeferConstruction, TransformScaleMethod);
//	UE_LOG(LogTemp, Warning, TEXT("ADynamicSDMCActor::PostSpawnInitialize END MeshComponent: %p"), MeshComponent);
//}



void ADynamicSDMCActor::OnMeshEditedInternal()
{
	Super::OnMeshEditedInternal();
	UpdateSDMCMesh();
}

void ADynamicSDMCActor::PostActorCreated()
{
	UE_LOG(LogTemp, Warning, TEXT("ADynamicSDMCActor::PostActorCreated MeshComponent: %p"), MeshComponent);
	Super::PostActorCreated();
	UE_LOG(LogTemp, Warning, TEXT("ADynamicSDMCActor::PostActorCreated END MeshComponent: %p"), MeshComponent);
	//MeshComponent = NewObject<UDynamicMeshComponent>();
	//bool result = SetRootComponent(MeshComponent);

	//if (!result) {
	//	UE_LOG(LogTemp, Error, TEXT("fadasd"));
	//}
}

void ADynamicSDMCActor::UpdateSDMCMesh()
{
	auto root = Cast<UDynamicMeshComponent>(GetRootComponent());

	/*if (!MeshComponent) {
		MeshComponent = NewObject< UDynamicMeshComponent>();
		MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}*/

	if (root)
	{
		*(root->GetMesh()) = SourceMesh;

		// update material on new section
		UMaterialInterface* UseMaterial = (this->Material != nullptr) ? this->Material : UMaterial::GetDefaultMaterial(MD_Surface);
		root->SetMaterial(0, UseMaterial);
		root->NotifyMeshUpdated();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Mesh Component"));
	}
}