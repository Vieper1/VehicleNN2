// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VehicleNN2GameMode.h"
#include "VehicleNN2Pawn.h"
#include "VehicleNN2Hud.h"

AVehicleNN2GameMode::AVehicleNN2GameMode()
{
	DefaultPawnClass = AVehicleNN2Pawn::StaticClass();
	HUDClass = AVehicleNN2Hud::StaticClass();
}
