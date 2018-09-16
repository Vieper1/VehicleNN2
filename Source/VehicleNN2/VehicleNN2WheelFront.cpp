// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VehicleNN2WheelFront.h"
#include "TireConfig.h"
#include "UObject/ConstructorHelpers.h"

UVehicleNN2WheelFront::UVehicleNN2WheelFront()
{
	// Drift Setup - Tire Config Friction Scale = 0.5
	//ShapeRadius = 18.0f;
	//ShapeWidth = 15.0f;
	//bAffectedByHandbrake = false;
	//SteerAngle = 60.f;

	//// Setup suspension forces
	//SuspensionForceOffset = -4.0f;
	//SuspensionMaxRaise = 1.1f;
	//SuspensionMaxDrop = 1.1f;
	//SuspensionNaturalFrequency = 9.0f;
	//SuspensionDampingRatio = 1.05f;


	// Grip Setup
	ShapeRadius = 18.0f;
	ShapeWidth = 15.0f;
	bAffectedByHandbrake = false;
	SteerAngle = 60.f;

	// Setup suspension forces
	SuspensionForceOffset = -4.0f;
	SuspensionMaxRaise = 1.05f;
	SuspensionMaxDrop = 1.05f;
	SuspensionNaturalFrequency = 9.0f;
	SuspensionDampingRatio = 1.05f;

	// Find the tire object and set the data for it
	static ConstructorHelpers::FObjectFinder<UTireConfig> TireData(TEXT("/Game/VehicleAdv/Vehicle/WheelData/Vehicle_FrontTireConfig.Vehicle_FrontTireConfig"));
	TireConfig = TireData.Object;
}
