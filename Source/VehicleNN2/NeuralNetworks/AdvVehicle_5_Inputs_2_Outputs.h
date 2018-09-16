// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleNN2Pawn.h"
#include "Sensors/Collider.h"
#include "AdvVehicle_5_Inputs_2_Outputs.generated.h"

/**
 * 
 */
UCLASS()
class VEHICLENN2_API AAdvVehicle_5_Inputs_2_Outputs : public AVehicleNN2Pawn
{
	GENERATED_BODY()

public:
	AAdvVehicle_5_Inputs_2_Outputs();
	
public:
	void Tick(float DeltaSeconds) override;
	void BeginPlay() override;
	
	
protected:
	ACollider* Collider_CF;
};
