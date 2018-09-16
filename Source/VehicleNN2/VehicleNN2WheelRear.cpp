// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VehicleNN2WheelRear.h"
#include "TireConfig.h"
#include "UObject/ConstructorHelpers.h"

UVehicleNN2WheelRear::UVehicleNN2WheelRear()
{
	// Drift Setup - Tire Config Friction Scale = 0.7
	//ShapeRadius = 18.0f;
	//ShapeWidth = 15.0f;
	//bAffectedByHandbrake = true;
	//SteerAngle = 0.f;

	//// Setup suspension forces
	//SuspensionForceOffset = -4.0f;
	//SuspensionMaxRaise = 1.0f;
	//SuspensionMaxDrop = 1.0f;
	//SuspensionNaturalFrequency = 9.0f;
	//SuspensionDampingRatio = 1.05f;


	// Grip Setup
	ShapeRadius = 18.0f;
	ShapeWidth = 15.0f;
	bAffectedByHandbrake = true;
	SteerAngle = 0.f;

	// Setup suspension forces
	SuspensionForceOffset = -4.0f;
	SuspensionMaxRaise = 1.0f;
	SuspensionMaxDrop = 1.0f;
	SuspensionNaturalFrequency = 9.0f;
	SuspensionDampingRatio = 1.05f;


	// Find the tire object and set the data for it
	static ConstructorHelpers::FObjectFinder<UTireConfig> TireData(TEXT("/Game/VehicleAdv/Vehicle/WheelData/Vehicle_BackTireConfig.Vehicle_BackTireConfig"));
	TireConfig = TireData.Object;
}
