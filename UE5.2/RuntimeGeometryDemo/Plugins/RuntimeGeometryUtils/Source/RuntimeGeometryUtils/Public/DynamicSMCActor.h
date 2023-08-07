#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "DynamicMeshBaseActor.h"
#include "DynamicSMCActor.generated.h"

UCLASS()
class RUNTIMEGEOMETRYUTILS_API ADynamicSMCActor : public ADynamicMeshBaseActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADynamicSMCActor();

public:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY(Transient)
	UStaticMesh* StaticMesh = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostLoad() override;
	virtual void PostActorCreated() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	/**
	 * ADynamicBaseActor API
	 */
	virtual void OnMeshEditedInternal() override;

protected:
	virtual void UpdateSMCMesh();
};
