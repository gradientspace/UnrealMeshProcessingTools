#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMeshBaseActor.h"
#include "DynamicSDMCActor.generated.h"

class FDynamicMesh3;

UCLASS()
class RUNTIMEGEOMETRYUTILS_API ADynamicSDMCActor : public ADynamicMeshBaseActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADynamicSDMCActor();

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		UDynamicMeshComponent* MeshComponent;

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
	//virtual void PostSpawnInitialize(FTransform const& UserSpawnTransform, AActor* InOwner, APawn* InInstigator, bool bRemoteOwned, bool bNoFail, bool bDeferConstruction, ESpawnActorScaleMethod TransformScaleMethod) override;
	virtual void PostActorCreated() override;

protected:
	virtual void UpdateSDMCMesh();
};
