#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleDynamicMeshComponent.h"
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
	UPROPERTY(VisibleAnywhere)
	USimpleDynamicMeshComponent* MeshComponent = nullptr;

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
	virtual void UpdateSDMCMesh();
};
