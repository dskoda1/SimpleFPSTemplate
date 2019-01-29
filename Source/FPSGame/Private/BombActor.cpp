// Fill out your copyright notice in the Description page of Project Settings.

#include "BombActor.h"
#include <GameFramework/Actor.h>
#include <Kismet/GameplayStatics.h>
#include "Engine/Engine.h"
#include <Materials/MaterialInstanceDynamic.h>

// Sets default values
ABombActor::ABombActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RootComponent = MeshComp;
	ExplodeDelay = 2.0f;
}

// Called when the game starts or when spawned
void ABombActor::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle explode_timer_handle;
	GetWorldTimerManager().SetTimer(explode_timer_handle, this, &ABombActor::Explode, ExplodeDelay);

	MaterialInst = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
	CurrentColor = MaterialInst->K2_GetVectorParameterValue("Color");
	TargetColor = FLinearColor::MakeRandomColor();
}

void ABombActor::Explode()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionTemplate, GetActorLocation());

	// Create query for what objects can match collision
	FCollisionObjectQueryParams queryParams;
	queryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	queryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	// Dictate the shape of the collision
	FCollisionShape collShape;
	collShape.SetSphere(500.0f);

	// Create the array for overlaps and populate it
	TArray<FOverlapResult> outOverlaps;
	GetWorld()->OverlapMultiByObjectType(outOverlaps, GetActorLocation(), FQuat::Identity, queryParams, collShape);

	// Finally we can iterate over components weve overlapped with
	// and change their color
	for (auto result : outOverlaps) {
		UPrimitiveComponent* overlap = result.GetComponent();
		if (overlap && overlap->IsSimulatingPhysics()) {
			auto matInst = overlap->CreateAndSetMaterialInstanceDynamic(0);
			matInst->SetVectorParameterValue("Color", TargetColor);
		}
	}

	Destroy();
}
// Called every frame
void ABombActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float progress = (GetWorld()->TimeSeconds - CreationTime) / ExplodeDelay;
	FLinearColor newColor = FLinearColor::LerpUsingHSV(CurrentColor, TargetColor, progress);

	if (MaterialInst) {
		MaterialInst->SetVectorParameterValue("Color", newColor);
	}
}

