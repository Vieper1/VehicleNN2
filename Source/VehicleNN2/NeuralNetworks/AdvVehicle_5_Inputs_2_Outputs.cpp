// Fill out your copyright notice in the Description page of Project Settings.

#include "AdvVehicle_5_Inputs_2_Outputs.h"
#include "Sensors/Collider.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"




AAdvVehicle_5_Inputs_2_Outputs::AAdvVehicle_5_Inputs_2_Outputs()
{
	/*Collider_CF = CreateDefaultSubobject<ACollider>(TEXT("Collider_CF"));
	Collider_CF->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("Sensor_CF")));
*/
}


void AAdvVehicle_5_Inputs_2_Outputs::BeginPlay()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ACollider* colliderCF = GetWorld()->SpawnActor<ACollider>(ACollider::StaticClass(), FVector(-10780.f, 8040.f, 60.f), FRotator(0, 0, 0), SpawnInfo);

	/*colliderCF->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName(TEXT("Sensor_CF")));*/
}

void AAdvVehicle_5_Inputs_2_Outputs::Tick(float DeltaSeconds)
{

}

