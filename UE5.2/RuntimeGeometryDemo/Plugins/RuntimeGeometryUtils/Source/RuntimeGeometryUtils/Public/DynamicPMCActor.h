#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "DynamicMeshBaseActor.h"
#include "DynamicPMCActor.generated.h"



UCLASS()
class RUNTIMEGEOMETRYUTILS_API ADynamicPMCActor : public ADynamicMeshBaseActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynamicPMCActor();

public:
	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* MeshComponent = nullptr;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	/**
	 * ADynamicBaseActor API
	 */
	virtual void OnMeshEditedInternal() override;

protected:
	virtual void UpdatePMCMesh();

};
