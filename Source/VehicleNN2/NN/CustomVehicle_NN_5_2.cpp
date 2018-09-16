// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CustomVehicle_NN_5_2.h"
#include "VehicleNN2WheelFront.h"
#include "VehicleNN2WheelRear.h"
#include "VehicleNN2Hud.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "GameFramework/Controller.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

const FName ACustomVehicle_NN_5_2::LookUpBinding("LookUp");
const FName ACustomVehicle_NN_5_2::LookRightBinding("LookRight");
const FName ACustomVehicle_NN_5_2::EngineAudioRPM("RPM");

#define LOCTEXT_NAMESPACE "VehiclePawn"


////////////////////////////////////////////////////////////////////// Code that came with the VehicleAdv project
ACustomVehicle_NN_5_2::ACustomVehicle_NN_5_2()
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/VehicleAdv/Vehicle/Vehicle_SkelMesh.Vehicle_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/VehicleAdv/Vehicle/VehicleAnimationBlueprint"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Setup friction materials
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/Slippery.Slippery"));
	SlipperyMaterial = SlipperyMat.Object;

	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(TEXT("/Game/VehicleAdv/PhysicsMaterials/NonSlippery.NonSlippery"));
	NonSlipperyMaterial = NonSlipperyMat.Object;

	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	// Wheels/Tyres
	// Setup the wheels
	Vehicle4W->WheelSetups[0].WheelClass = UVehicleNN2WheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("PhysWheel_FL");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UVehicleNN2WheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("PhysWheel_FR");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UVehicleNN2WheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("PhysWheel_BL");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UVehicleNN2WheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("PhysWheel_BR");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	// Adjust the tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	// Engine 
	// Torque setup
	Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);

	// Adjust the steering 
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	// Transmission	
	// We want 4wd
	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	Vehicle4W->DifferentialSetup.RearLeftRightSplit = 0.5f;
	Vehicle4W->DifferentialSetup.RearBias = 1.0f;

	// Drive the front wheels a little more than the rear
	Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

	// Automatic gearbox
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
	Vehicle4W->TransmissionSetup.GearSwitchTime = 0.15f;
	Vehicle4W->TransmissionSetup.GearAutoBoxLatency = 1.0f;

	// Physics settings
	// Adjust the center of mass - the buggy is quite low
	UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
	if (UpdatedPrimitive)
	{
		UpdatedPrimitive->BodyInstance.COMNudge = FVector(8.0f, 0.0f, 0.0f);
	}

	// Set the inertia scale. This controls how the mass of the vehicle is distributed.
	Vehicle4W->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

	// Create a spring arm component for our chase camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
	SpringArm->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 125.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create the chase camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeLocation(FVector(-150.0, 0.0f, 25.0f));
	Camera->SetRelativeRotation(FRotator(10.0f, 0.0f, 0.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(-34.0f, -10.0f, 50.0f);
	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOrigin);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);

	// In car HUD
	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
	InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
	InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarGear->SetupAttachment(GetMesh());

	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("/Game/VehicleAdv/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
	EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
	EngineSoundComponent->SetSound(SoundCue.Object);
	EngineSoundComponent->SetupAttachment(GetMesh());

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bIsLowFriction = false;
	bInReverseGear = false;


	// Setup NN Collision Direction Indicator
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowMesh(TEXT("/Game/StarterContent/Blueprints/Assets/SM_Arrows.SM_Arrows"));
	CollisionDirectionIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionDirectionIndicator"));
	CollisionDirectionIndicator->SetStaticMesh(ArrowMesh.Object);
	CollisionDirectionIndicator->SetWorldScale3D(FVector(0.05f, 0.05f, 0.05f));
	CollisionDirectionIndicator->SetupAttachment(GetMesh(), TEXT("Sensor_Arrow"));

	bDo = true;
}

void ACustomVehicle_NN_5_2::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACustomVehicle_NN_5_2::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACustomVehicle_NN_5_2::MoveRight);
	PlayerInputComponent->BindAxis(LookUpBinding);
	PlayerInputComponent->BindAxis(LookRightBinding);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &ACustomVehicle_NN_5_2::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &ACustomVehicle_NN_5_2::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ACustomVehicle_NN_5_2::OnToggleCamera);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACustomVehicle_NN_5_2::OnResetVR);

	PlayerInputComponent->BindAction("SelfDriveToggle", IE_Pressed, this, &ACustomVehicle_NN_5_2::ToggleSelfDrive);
	PlayerInputComponent->BindAction("TrainingModeToggle", IE_Pressed, this, &ACustomVehicle_NN_5_2::ToggleTrainingMode);
}

void ACustomVehicle_NN_5_2::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);

}

void ACustomVehicle_NN_5_2::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void ACustomVehicle_NN_5_2::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ACustomVehicle_NN_5_2::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void ACustomVehicle_NN_5_2::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void ACustomVehicle_NN_5_2::EnableIncarView(const bool bState)
{
	if (bState != bInCarCameraActive)
	{
		bInCarCameraActive = bState;

		if (bState == true)
		{
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
		}
		else
		{
			InternalCamera->Deactivate();
			Camera->Activate();
		}

		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}

void ACustomVehicle_NN_5_2::OnResetVR()
{
#if HMD_MODULE_INCLUDED
	if (GEngine->XRSystem.IsValid())
	{
		GEngine->XRSystem->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
#endif // HMD_MODULE_INCLUDED
}

void ACustomVehicle_NN_5_2::UpdateHUDStrings()
{
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	int32 Gear = GetVehicleMovement()->GetCurrentGear();

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));


	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}

}

void ACustomVehicle_NN_5_2::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr))
	{
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);

		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

void ACustomVehicle_NN_5_2::UpdatePhysicsMaterial()
{
	if (GetActorUpVector().Z < 0)
	{
		if (bIsLowFriction == true)
		{
			GetMesh()->SetPhysMaterialOverride(NonSlipperyMaterial);
			bIsLowFriction = false;
		}
		else
		{
			GetMesh()->SetPhysMaterialOverride(SlipperyMaterial);
			bIsLowFriction = true;
		}
	}
}

#undef LOCTEXT_NAMESPACE
////////////////////////////////////////////////////////////////////// Code that came with the VehicleAdv project



// Spawn Colliders For Sensor Data
void ACustomVehicle_NN_5_2::SetUpNNComponents()
{
	if (GetWorld() == NULL) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Collider_L = GetWorld()->SpawnActor<ACollider>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
	Collider_L->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("Sensor_L")));
	ColliderArray.Add(Collider_L);

	Collider_LW = GetWorld()->SpawnActor<ACollider>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
	Collider_LW->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("Sensor_LW")));
	ColliderArray.Add(Collider_LW);
	
	Collider_CF = GetWorld()->SpawnActor<ACollider>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
	Collider_CF->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("Sensor_CF")));
	ColliderArray.Add(Collider_CF);

	Collider_RW = GetWorld()->SpawnActor<ACollider>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
	Collider_RW->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("Sensor_RW")));
	ColliderArray.Add(Collider_RW);

	Collider_R = GetWorld()->SpawnActor<ACollider>(FVector(0, 0, 0), FRotator(0, 0, 0), SpawnParams);
	Collider_R->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("Sensor_R")));
	ColliderArray.Add(Collider_R);

	isInCollisionRadius = false;


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init Config Variables ///////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	WeightsUpdationThreshold = 0.3f;
	AlphaMultiplier = 0.5f;
	for (ACollider* _collider : ColliderArray)
	{
		SteeringWeights.Add(0.0f);
		ThrottleWeights.Add(0.0f);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Init Config Variables ///////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
}









// On Simulation Start
void ACustomVehicle_NN_5_2::BeginPlay()
{
	Super::BeginPlay();

	bool bWantInCar = false;
	// First disable both speed/gear displays 
	bInCarCameraActive = false;
	InCarSpeed->SetVisibility(bInCarCameraActive);
	InCarGear->SetVisibility(bInCarCameraActive);

	// Enable in car view if HMD is attached
#if HMD_MODULE_INCLUDED
	bWantInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED

	EnableIncarView(bWantInCar);
	// Start an engine sound playing
	EngineSoundComponent->Play();





	SetUpNNComponents();
}



////////////////////////////////////////////////////////////////////// IMPORTANT STUFF DONE HERE
// On Every Frame
void ACustomVehicle_NN_5_2::Tick(float Delta)
{
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;

	// Update phsyics material
	UpdatePhysicsMaterial();

	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->XRSystem.IsValid() == true) && ((GEngine->XRSystem->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true)))
	{
		bHMDActive = true;
	}
#endif // HMD_MODULE_INCLUDED
	if (bHMDActive == false)
	{
		if ((InputComponent) && (bInCarCameraActive == true))
		{
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}

	// Pass the engine RPM to the sound component
	float RPMToAudioScale = 2500.0f / GetVehicleMovement()->GetEngineMaxRotationSpeed();
	EngineSoundComponent->SetFloatParameter(EngineAudioRPM, GetVehicleMovement()->GetEngineRotationSpeed()*RPMToAudioScale);




	// NN
	// Training on the go
	// Obtaining the training data from collisions
	RetCollisionInfo collisionInfo = GetCollisionDirection();
	UpdateCollisionDirectionIndicator();

	if (GEngine != NULL) {
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Direction = %f\nInvMagnitude = %f"), collisionInfo.CollisionZ, collisionInfo.CollisionInverseMagnitude));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Steering Weights")));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f => L"), SteeringWeights[0]));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f => LW"), SteeringWeights[1]));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f => CF"), SteeringWeights[2]));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f => RW"), SteeringWeights[3]));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f => R"), SteeringWeights[4]));
	}

	if (ShouldITrain)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("Training = ON"));
		if (collisionInfo.CollisionInverseMagnitude < WeightsUpdationThreshold)
		{
			NNDoOnce(collisionInfo);
		}
		else
		{
			NNResetDoOnce();
		}
	}
	else
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("Training = OFF"));
	

	if (SelfDrive)
	{
		// Steering Self Drive
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("SelfDrive = ON"));
		GetVehicleMovementComponent()->SetSteeringInput(CalculateInputFromWeights(SteeringWeights, false));
	}
	else
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, TEXT("SelfDrive = OFF"));
	
}


void ACustomVehicle_NN_5_2::NNDoOnce(RetCollisionInfo retDirectionInfo)
{
	if (bDo)
	{
		ResetVehiclePosition();
		UpdateWeights(retDirectionInfo);
		// isInCollisionRadius = false;
		bDo = false;
		return;
	}
	else
		return;
}

void ACustomVehicle_NN_5_2::NNResetDoOnce()
{
	bDo = true;
	return;
}
////////////////////////////////////////////////////////////////////// IMPORTANT STUFF DONE HERE












////////////////////////////////////////////////////////////////////// Neural Network Utilities
float ACustomVehicle_NN_5_2::GetSigmoid(float input)
{
	float x = FMath::Pow(2.718f, -1.0f * input);
	return (1.0f - x) / (1.0f + x);
}

RetCollisionInfo ACustomVehicle_NN_5_2::GetCollisionDirection()
{
	if (ColliderArray.Num() < 1) { return{ 0.0f, 0.0f }; }

	float _zSum = 0.0f;
	float _activationSum = 0.0f;
	float _activationProduct = 1.0f;
	float _tempZ = 0.0f;
	RetCollisionInfo RetVal = { 0.0f, 0.0f };

	for (ACollider* _collider : ColliderArray)
	{
		// Get Z Rotation
		float _colliderYaw = 0.0f; /*_collider->GetActorRotation().Yaw*/; // Possible after I made this component public in ACollider
		// This is a stopgap. I couldn't find relative rotation in this context, like what I used in the Blueprint version
		if (_collider == Collider_L) _colliderYaw = 90.0f;
		if (_collider == Collider_LW) _colliderYaw = 140.0f;
		if (_collider == Collider_CF) _colliderYaw = 180.0f;
		if (_collider == Collider_RW) _colliderYaw = 220.0f;
		if (_collider == Collider_R) _colliderYaw = 270.0f;


		_tempZ = _colliderYaw;
		// This block's used with Relative rotation
		/*if (_colliderYaw >= 0.0f)
			_tempZ = _colliderYaw;
		else
			_tempZ = _colliderYaw + 360.0f;*/


		// Activation and aggregation
		_activationSum += (1.0f - _collider->GetActivationValue());
		_zSum += (1.0f - _collider->GetActivationValue()) * _tempZ;
		_activationProduct *= _collider->GetActivationValue();
	}

	if (_activationSum < 0.1f)
	{
		NetCollisionRotator = FRotator(0, 0, 0);
		NetCollisionScale = FVector(0.001f, 0.001f, 0.001f);
		RetVal = { 0.0f, 1.0f };
	}
	else
	{
		NetCollisionRotator = FRotator(0, _zSum / _activationSum, 0);
		float _temp = _activationSum * 0.02f * ColliderArray.Num();
		if (_temp < 0.001f)
			NetCollisionScale = FVector(0.001f, 0.05f, 0.05f);
		else
			NetCollisionScale = FVector(_temp, 0.05f, 0.05f);

		RetVal = { _zSum / _activationSum, FMath::Pow(_activationProduct, 1.0f / ColliderArray.Num()) };
	}

	/*if (GEngine != NULL)
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("%f"), Rotation));*/

	return RetVal;
}

void ACustomVehicle_NN_5_2::UpdateCollisionDirectionIndicator()
{
	CollisionDirectionIndicator->SetRelativeRotation(NetCollisionRotator);
	CollisionDirectionIndicator->SetWorldScale3D(NetCollisionScale);
	CollisionDirectionIndicator->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Sensor_Arrow"));
}


void ACustomVehicle_NN_5_2::UpdateWeights(RetCollisionInfo collisionInfo)
{
	float _sinD = 0.0f;

	float _temp = UKismetMathLibrary::DegSin(collisionInfo.CollisionZ);
	if (_temp > 0.0f) _sinD = 1.0f;
	else if (_temp < 0.0f) _sinD = -1.0f;

	for (int32 i = 0; i < SteeringWeights.Num(); i++)
	{
		float __temp = 1.0f - ColliderArray[i]->GetActivationValue();
		SteeringWeights[i] += (__temp * _sinD * AlphaMultiplier);
	}
}

float ACustomVehicle_NN_5_2::CalculateInputFromWeights(TArray<float> WeightsArray, bool NeedSigmoid)
{
	float _ySum = 0.0f;

	for (int32 i = 0; i < WeightsArray.Num(); i++)
	{
		_ySum += (WeightsArray[i] * ColliderArray[i]->GetActivationValue());
		if (NeedSigmoid)
			_ySum = GetSigmoid(_ySum);
	}

	return _ySum;
}

void ACustomVehicle_NN_5_2::ResetVehiclePosition()
{
	// Edit here if you want to change the Vehicle reset transform temporarily
	
	FVector _resetLocation = FVector(-5850.000000f, 10270.000000f, 112.000000f);
	FRotator _resetRotation = FRotator(0.000000f, 90.000114f, 0.000000f);
	if (TeleportTo(_resetLocation, _resetRotation, false, false))
	{
		GetMesh()->SetPhysicsLinearVelocity(FVector(0, 0, 0), false, TEXT("NONE"));
		GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector(0, 0, 0), false, TEXT("NONE"));
	}
}





void ACustomVehicle_NN_5_2::ToggleSelfDrive()
{
	if (SelfDrive)
		SelfDrive = false;
	else
		SelfDrive = true;
}

void ACustomVehicle_NN_5_2::ToggleTrainingMode()
{
	if (ShouldITrain)
		ShouldITrain = false;
	else
		ShouldITrain = true;
}
////////////////////////////////////////////////////////////////////// Neural Network Utilities