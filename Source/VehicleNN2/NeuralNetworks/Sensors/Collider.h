// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Collider.generated.h"

UCLASS()
class VEHICLENN2_API ACollider : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollider();
	

public:
	UPROPERTY(Category = Sensor, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	UPROPERTY(Category = Sensor, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SphereIndicator;

	UPROPERTY(Category = Sensor, EditAnywhere)
		float ArmLength;

public:
	float GetActivationValue();
};
