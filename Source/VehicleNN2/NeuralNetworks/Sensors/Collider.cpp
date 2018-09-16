// Fill out your copyright notice in the Description page of Project Settings.

#include "Collider.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"


// Sets default values
ACollider::ACollider()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// SpringArm to detect distance from walls
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0,0,0));
	SpringArm->SetWorldRotation(FRotator(0, 0, 0));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritRoll = true;
	SpringArm->bInheritYaw = true;
	SpringArm->Mobility = EComponentMobility::Movable;
	RootComponent = SpringArm;

	// Sphere mesh to show collision location
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Game/StarterContent/Props/MaterialSphere.MaterialSphere"));
	static ConstructorHelpers::FObjectFinder<UMaterial> SphereMaterial(TEXT("/Game/StarterContent/Props/Materials/M_MaterialSphere_Plain.M_MaterialSphere_Plain"));
	SphereIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereIndicator"));
	SphereIndicator->SetStaticMesh(SphereMesh.Object);
	SphereIndicator->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	SphereIndicator->SetMobility(EComponentMobility::Movable);

	USceneComponent* SphereComponent = Cast<USceneComponent>(SphereIndicator);
	SphereComponent->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
}


float ACollider::GetActivationValue()
{
	float activation = (SphereIndicator->GetComponentLocation() - SpringArm->GetComponentLocation()).Size() / SpringArm->TargetArmLength;
	if (activation < 0.0f)
		return 0.0f;
	else if (activation > 1.0f)
		return 1.0f;
	else
		return activation;
}
