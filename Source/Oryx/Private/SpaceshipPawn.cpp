#pragma region Headers
#include "SpaceshipPawn.h"
#include "EnhancedInputComponent.h"				//Needed to bind Enhanced Input Actions.
#include "EnhancedInputSubsystems.h"			//Required to access the Input Subsystem on the local player.
#include "InputMappingContext.h"				//For the UInputMappingContext reference.
#include "InputAction.h"						//For Enhanced Input action definitions
#include "GameFramework/PlayerController.h"		//To access player controller for mouse, input mode, etc.
#include "Kismet/GameplayStatics.h"				//Utility for player and actor lookups.
#include "Engine/World.h"						//For world context.
#include "Components/StaticMeshComponent.h"		//To define and modify the ship's static mesh.
#include "Components/SceneComponent.h"			//For scene components (thruster attach points).
#include "LandingPad.h"							//For referencing landing pad.
#pragma endregion

//Constructor - Sets up component heirarchy, physics, and vfx
ASpaceshipPawn::ASpaceshipPawn()
{
	PrimaryActorTick.bCanEverTick = true; //Enable ticking every frame for physics and rotation updates

#pragma region ShipMesh and Ship Physics
	//Static mesh acts as the physical body for the ship
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh;

	//Enable physics simulation for movement via forces
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetEnableGravity(false);
	ShipMesh->SetLinearDamping(0.3f);
	ShipMesh->SetAngularDamping(0.4f);
#pragma endregion

#pragma region Thruster Scene Components
	//Empty transforms that define where forces and effects are applied
	MainThruster = CreateDefaultSubobject<USceneComponent>(TEXT("MainThruster"));
	MainThruster->SetupAttachment(ShipMesh);

	LeftThruster = CreateDefaultSubobject<USceneComponent>(TEXT("LeftThruster"));
	LeftThruster->SetupAttachment(ShipMesh);

	RightThruster = CreateDefaultSubobject<USceneComponent>(TEXT("RightThruster"));
	RightThruster->SetupAttachment(ShipMesh);

	ReverseLeftThruster = CreateDefaultSubobject<USceneComponent>(TEXT("ReverseLeftThruster"));
	ReverseLeftThruster->SetupAttachment(ShipMesh);

	ReverseRightThruster = CreateDefaultSubobject<USceneComponent>(TEXT("ReverseRightThruster"));
	ReverseRightThruster->SetupAttachment(ShipMesh);
#pragma endregion

#pragma region Niagara FX for thrusters
	//These hold the particle systems and can be activated/deactivated dynamically
	MainThrusterFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MainThrusterFX"));
	MainThrusterFX->SetupAttachment(MainThruster);
	MainThrusterFX->bAutoActivate = false;

	LeftThrusterFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftThrusterFX"));
	LeftThrusterFX->SetupAttachment(LeftThruster);
	LeftThrusterFX->bAutoActivate = false;

	RightThrusterFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightThrusterFX"));
	RightThrusterFX->SetupAttachment(RightThruster);
	RightThrusterFX->bAutoActivate = false;

	LeftBrakeThrusterFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftBrakeThrusterFX"));
	LeftBrakeThrusterFX->SetupAttachment(ReverseLeftThruster);
	LeftBrakeThrusterFX->bAutoActivate = false;

	RightBrakeThrusterFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightBrakeThrusterFX"));
	RightBrakeThrusterFX->SetupAttachment(ReverseRightThruster);
	RightBrakeThrusterFX->bAutoActivate = false;
#pragma endregion
}

//Called when the game starts or when spawned
//Handling setup of input context, cursor settings, and FX assets
void ASpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();
	
#pragma region Input mode and Cursor Configuration
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true; //Displays the cursore for the mouse-based direction input

		//Allow mouse to move freely on screen
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
#pragma endregion

#pragma region Input Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				//Register this pawn's specific input mapping
				if (ShipMappingContext)
				{
					Subsystem->AddMappingContext(ShipMappingContext, 0);
				}
			}
		}
	}
#pragma endregion

#pragma region Assign Niagara FX if specified in editor
	if (MainThrusterEffect) MainThrusterFX->SetAsset(MainThrusterEffect);
	if (LeftThrusterEffect) LeftThrusterFX->SetAsset(LeftThrusterEffect);
	if (RightThrusterEffect) RightThrusterFX->SetAsset(RightThrusterEffect);
	if (LeftBrakeThrusterEffect) LeftBrakeThrusterFX->SetAsset(LeftBrakeThrusterEffect);
	if (RightBrakeThrusterEffect) RightBrakeThrusterFX->SetAsset(RightBrakeThrusterEffect);
#pragma endregion
}

// Called every frame
void ASpaceshipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RestrictMouseToCircle(); //Clamp mouse offset for steering

	//Landing sequence logic
	if (bIsLanding && TargetLandingPad)
	{
		LandingSequence(DeltaTime);
		return;
	}

	//Update input-based states
	UpdateRotation(DeltaTime); //Smoothly rotate towards mouse offset
	ApplyThrusters(DeltaTime); //Apply forces based on active thrusters
}

//Bind enhanced input actions to callback functions
void ASpaceshipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Forward thrust (W)
		EIC->BindAction(IA_ForwardThrust, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnForwardThrust);
		EIC->BindAction(IA_ForwardThrust, ETriggerEvent::Completed, this, &ASpaceshipPawn::OnForwardThrust);

		//Left thrust (D)
		EIC->BindAction(IA_LeftThrust, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnLeftThrust);
		EIC->BindAction(IA_LeftThrust, ETriggerEvent::Completed, this, &ASpaceshipPawn::OnLeftThrust);

		//Right thrust (A)
		EIC->BindAction(IA_RightThrust, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnRightThrust);
		EIC->BindAction(IA_RightThrust, ETriggerEvent::Completed, this, &ASpaceshipPawn::OnRightThrust);

		//All thrusters (Space)
		EIC->BindAction(IA_AllThrusters, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnAllThrusters);
		EIC->BindAction(IA_AllThrusters, ETriggerEvent::Completed, this, &ASpaceshipPawn::OnAllThrusters);

		//Brake/Reverse thrusters (LShift)
		EIC->BindAction(IA_Brake, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnBrake);
		EIC->BindAction(IA_Brake, ETriggerEvent::Completed, this, &ASpaceshipPawn::OnBrake);

		//Landing (E)
		EIC->BindAction(IA_Land, ETriggerEvent::Triggered, this, &ASpaceshipPawn::OnLand);
	}
}

#pragma region Input Callbacks
void ASpaceshipPawn::OnForwardThrust(const FInputActionValue& Value) { bForwardThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnLeftThrust(const FInputActionValue& Value) { bLeftThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnRightThrust(const FInputActionValue& Value) { bRightThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnAllThrusters(const FInputActionValue& Value) { bAllThrusters = Value.Get<bool>(); }
void ASpaceshipPawn::OnBrake(const FInputActionValue& Value) { bBrake = Value.Get<bool>(); }
#pragma endregion

//Limists mouse range for steering
void ASpaceshipPawn::RestrictMouseToCircle()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	int32 ScreenX, ScreenY;
	PC->GetViewportSize(ScreenX, ScreenY);

	FVector2D MousePos;
	if (!PC->GetMousePosition(MousePos.X, MousePos.Y))
		return;

	//Compute mouse offset from screen center
	FVector2D ScreenCenter(ScreenX / 2.f, ScreenY / 2.f);
	FVector2D Offset = MousePos - ScreenCenter;

	//Clamp offset within circular radius
	if (Offset.Size() > MaxMouseRadius)
	{
		Offset = Offset.GetSafeNormal() * MaxMouseRadius;
		FVector2D ClampedPos = ScreenCenter + Offset;
		PC->SetMouseLocation(ClampedPos.X, ClampedPos.Y);
	}

	MouseOffset = Offset / MaxMouseRadius; //normalized to [-1,1] range for easier directional math
}

//Smoothly turns ship towards mouse offset
void ASpaceshipPawn::UpdateRotation(float DeltaTime)
{
	if (!ShipMesh) return;

	//MouseOffset is normalized between [-1, 1] in both X and Y.
	//X controls left/right, Y controls up/down — calculated earlier in RestrictMouseToCircle().
	FVector2D Offset = MouseOffset;

	// If the mouse is very close to the center of the screen, skip rotation updates to prevent jitter.
	if (Offset.SizeSquared() < KINDA_SMALL_NUMBER)
		return;


	//Convert the 2D mouse offset into desired pitch, yaw, and roll angles.
	//These act like "rotation targets" that the ship will smoothly interpolate toward.

	//Pitch (X-axis rotation) controls nose up/down.
	//We invert the Y offset because in screen space:
	//- moving the mouse up gives a negative Y value, but we want the nose to go up (positive pitch).
	float TargetPitch = -Offset.Y * 45.f;  //Up to +/-45 degrees max pitch

	//Yaw (Z-axis rotation) controls turning left/right (like steering).
	//Positive X (mouse right) -> positive yaw -> turn right.
	float TargetYaw = Offset.X * 45.f;     //Up to +/-45 degrees max yaw

	//Roll (Y-axis rotation) gives the ship a banking effect when turning.
	//It uses only the X offset (horizontal movement) to roll into turns.
	float TargetRoll = Offset.X * 45.f;    //Up to +/-30 degrees max roll


	//Combine these into a target rotation relative to the current one.

	//Get current ship rotation so we can interpolate from it.
	FRotator CurrentRot = GetActorRotation();

	//Create a new target rotation based on the calculated pitch/yaw/roll deltas.
	//- Pitch and Yaw are added to the current values to steer the ship.
	//- Roll is treated more like a visual bank (not cumulative) so it just replaces the current roll.
	FRotator TargetRot = FRotator(
		CurrentRot.Pitch + TargetPitch, // nose up/down movement
		CurrentRot.Yaw + TargetYaw,     // turning left/right
		TargetRoll                      // rolling into the turn
	);


	//Smoothly interpolate toward the target rotation.
	//This makes the ship’s movement feel smooth and responsive rather than snappy.
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);

	//Finally, apply the new rotation to the ship.
	SetActorRotation(NewRot);
}

//Applies forces and activaes FX as needed
void ASpaceshipPawn::ApplyThrusters(float DeltaTime)
{
	if (!ShipMesh) return;

	//Lambda helper for applying forces at thruster locations
	auto ApplyForceAt = [&](USceneComponent* Thruster, float Force)
		{
			if (Thruster)
				ShipMesh->AddForceAtLocation(Thruster->GetForwardVector() * Force, Thruster->GetComponentLocation());
		};

	if (bAllThrusters)
	{
		//Apply all forces
		ApplyForceAt(MainThruster, ForwardThrusterForce);
		ApplyForceAt(LeftThruster, SideThrusterForce);
		ApplyForceAt(RightThruster, SideThrusterForce);

		//Activate FX
		if (MainThrusterFX && !MainThrusterFX->IsActive()) MainThrusterFX->Activate(true);
		if (LeftThrusterFX && !LeftThrusterFX->IsActive()) LeftThrusterFX->Activate(true);
		if (RightThrusterFX && !RightThrusterFX->IsActive()) RightThrusterFX->Activate(true);
	}
	else
	{
		//Main Thruster
		if (bForwardThrust)
		{
			ApplyForceAt(MainThruster, ForwardThrusterForce);
			if (MainThrusterFX && !MainThrusterFX->IsActive()) MainThrusterFX->Activate(true);
		}
		else if (MainThrusterFX && MainThrusterFX->IsActive())
		{
			MainThrusterFX->Deactivate();
		}

		//Left Thruster
		if (bLeftThrust)
		{
			ApplyForceAt(LeftThruster, SideThrusterForce);
			if (LeftThrusterFX && !LeftThrusterFX->IsActive()) LeftThrusterFX->Activate(true);
		}
		else if (LeftThrusterFX && LeftThrusterFX->IsActive())
		{
			LeftThrusterFX->Deactivate();
		}

		//Right Thruster
		if (bRightThrust)
		{
			ApplyForceAt(RightThruster, SideThrusterForce);
			if (RightThrusterFX && !RightThrusterFX->IsActive()) RightThrusterFX->Activate(true);
		}
		else if (RightThrusterFX && RightThrusterFX->IsActive())
		{
			RightThrusterFX->Deactivate();
		}
	}

	//Brake Thrusters (independent of others)
	if (bBrake)
	{
		ApplyForceAt(ReverseLeftThruster, -SideThrusterForce);
		ApplyForceAt(ReverseRightThruster, -SideThrusterForce);

		if (LeftBrakeThrusterFX && !LeftBrakeThrusterFX->IsActive()) LeftBrakeThrusterFX->Activate(true);
		if (RightBrakeThrusterFX && !RightBrakeThrusterFX->IsActive()) RightBrakeThrusterFX->Activate(true);
	}
	else
	{
		LeftBrakeThrusterFX->SetActive(false);
		RightBrakeThrusterFX->SetActive(false);
	}
}

//Input function to trigger landing
void ASpaceshipPawn::OnLand(const FInputActionValue& Value)
{
	if (bIsLanding) return;

	if (OverlappingLandingPad)
	{
		StartLanding(OverlappingLandingPad);
	}
}

//Function called from OnLand to set ELandingStage Enum and bIsLanding
void ASpaceshipPawn::StartLanding(ALandingPad* LandingPad)
{
	if (!LandingPad) return;

	TargetLandingPad = LandingPad;
	bIsLanding = true;

	bForwardThrust = false;
	bLeftThrust = false;
	bRightThrust = false;
	bAllThrusters = false;
	bBrake = false;

	LandingStage = ELandingStage::RotateToPad;
}

//Function that handles the entire landing sequence
void ASpaceshipPawn::LandingSequence(float DeltaTime)
{
	//Current ship and landing pad position
	FVector ShipLocation = GetActorLocation();
	FVector PadLocation = TargetLandingPad->GetActorLocation();

	//Define a target above the pad to approach before descending
	FVector TargetAbovePad = PadLocation + FVector(0.f, 0.f, 1000.f); //vertical offset above the pad
	FVector DirectionToPad = (TargetAbovePad - ShipLocation); //Vector from ship to pad
	float DistanceToTarget = DirectionToPad.Size();
	DirectionToPad.Normalize();

	//Get current rotation of the ship
	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = DirectionToPad.Rotation(); //Converts direction vector to rotator
	FRotator NewRot = CurrentRot; //New rotation to interpolate toward

	switch (LandingStage)
	{
	case ELandingStage::RotateToPad: //Face towards pad
	{
		//Ensuring all VFX are deactivate upon landing sequence starting
		if (MainThrusterFX->IsActive()) MainThrusterFX->Deactivate();
		if (LeftThrusterFX->IsActive()) LeftThrusterFX->Deactivate();
		if (RightThrusterFX->IsActive()) RightThrusterFX->Deactivate();
		if (RightBrakeThrusterFX->IsActive()) RightBrakeThrusterFX->Deactivate();
		if (LeftBrakeThrusterFX->IsActive()) LeftBrakeThrusterFX->Deactivate();

		//Interpolate current rotation toward target rotation
		//FMath::RInterpTo performs frame-independent rotation interpolation
		NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LandingRotateSpeed);
		SetActorRotation(NewRot);

		//Ensuring ship is rotated within +/- 2.5 degrees on all axis
		if (FMath::Abs(NewRot.Pitch - TargetRot.Pitch) < 2.5f &&
			FMath::Abs(NewRot.Yaw - TargetRot.Yaw) < 2.5f &&
			FMath::Abs(NewRot.Roll - TargetRot.Roll) < 2.5f)
		{
			LandingStage = ELandingStage::MoveToPad; //Go to moving phase
			if (!MainThrusterFX->IsActive()) MainThrusterFX->Activate(true);
		}
		break;
	}

	case ELandingStage::MoveToPad: //Move towards pad
	{
		//Move ship forward along normalized direction vector
		FVector NewLocation = ShipLocation + DirectionToPad * LandingMoveSpeed * DeltaTime;
		SetActorLocation(NewLocation);

		if (DistanceToTarget < 1000.f) //When ship within the specified range begin braking
		{
			LandingStage = ELandingStage::ApplyBrakes;
			if (MainThrusterFX->IsActive()) MainThrusterFX->Deactivate();
			if (!LeftBrakeThrusterFX->IsActive()) LeftBrakeThrusterFX->Activate(true);
			if (!RightBrakeThrusterFX->IsActive()) RightBrakeThrusterFX->Activate(true);
		}
		break;
	}

	case ELandingStage::ApplyBrakes: //Apply 'brakes' slowing approach
	{
		float SlowSpeed = LandingMoveSpeed * 0.4f; //Reduce speed to 40% of the previous speed

		//Continue moving toward target
		FVector NewLocation = ShipLocation + DirectionToPad * SlowSpeed * DeltaTime;
		SetActorLocation(NewLocation);

		if (DistanceToTarget < 300.f) //When ship within the specified range begin rotational alignment
		{
			LandingStage = ELandingStage::AlignRotation;
			LeftBrakeThrusterFX->Deactivate();
			RightBrakeThrusterFX->Deactivate();
		}
		break;
	}

	case ELandingStage::AlignRotation:
	{
		//Get pad yaw rotation
		float PadYaw = TargetLandingPad->GetActorRotation().Yaw;

		//Normalize to 0–360 range
		auto Normalize360 = [](float Angle)
			{
				Angle = FMath::Fmod(Angle, 360.f);
				if (Angle < 0.f)
					Angle += 360.f;
				return Angle;
			};

		//Normalize both the pads yaw and our ships yaw to a clean 0-360 range
		float ShipYaw = Normalize360(CurrentRot.Yaw);
		PadYaw = Normalize360(PadYaw);

		//The landing pad will only ever be flat in 1 of 4 angles(0, 90, 180, or 270 degrees)
		//They are then grouped into 2 families (X-Axis facing -> 0 or 180) (Y-Axis facing -> 90 or 270)
		//Group is decided based on the pads facing
		TArray<float> PossibleYaws;
		if (FMath::Abs(FRotator::NormalizeAxis(PadYaw - 0.f)) < 1.f ||
			FMath::Abs(FRotator::NormalizeAxis(PadYaw - 180.f)) < 1.f)
		{
			//Pad is facing along X-axis, so valid target yaws are 0 or 180
			PossibleYaws = { 0.f, 180.f };
		}
		else
		{
			//Pad is facing along Y-axis, so valid target yaws are 90 or 270
			PossibleYaws = { 90.f, 270.f };
		}

		//Find which of those two angles is closest to our current ship yaw
		float ClosestYaw = PossibleYaws[0]; //Start with the first option
		float SmallestDiff = 9999.f; //A large number so the first real difference will replace it
		for (float YawOption : PossibleYaws) //Loop through each possible yaw in the group
		{
			float Diff = FMath::Abs(FRotator::NormalizeAxis(YawOption - ShipYaw));
			if (Diff < SmallestDiff)
			{
				//Found the closer option, update our target
				SmallestDiff = Diff;
				ClosestYaw = YawOption;
			}
		}

		//Create smooth rotation
		TargetRot = FRotator(0.f, ClosestYaw, 0.f); //Only rotate around yaw, keep pitch/roll at 0

		//RinterpTo smoothly interpolates between the ships current rotation and its target rotation over time
		NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LandingRotateSpeed);
		SetActorRotation(NewRot); //Apply the new smoothed rotation to the ship

		//Small drift motion toward pad
		FVector DriftDirection = (PadLocation - ShipLocation).GetSafeNormal();
		FVector NewLocation = ShipLocation + DriftDirection * (LandingMoveSpeed * 0.2f) * DeltaTime;
		SetActorLocation(NewLocation);

		//Once aligned, move to next stage
		//NormalizeAxis ensures that 359 degrees and 0 degrees are treated as a 1 degree difference and not 359
		if (FMath::Abs(FRotator::NormalizeAxis(NewRot.Yaw - TargetRot.Yaw)) < 1.f)
		{
			LandingStage = ELandingStage::Descend;
		}

		break;
	}

	case ELandingStage::Descend: //Descend to pad
	{
		//Move ship downward by LandingDescendSpeed per second
		FVector DescendLoc = ShipLocation;
		DescendLoc.Z -= LandingDescendSpeed * DeltaTime;

		//'true' enableds collision chekcs during movement
		SetActorLocation(DescendLoc, true);

		//Stop descending once ships Z is close to the pads surface
		if (ShipLocation.Z <= PadLocation.Z + 300.f)
		{

			LandingStage = ELandingStage::Finished;
			bIsLanding = false; //Landing complete

			//Ensures all VFX used during landing sequence are deactivated
			MainThrusterFX->Deactivate();
			LeftBrakeThrusterFX->Deactivate();
			RightBrakeThrusterFX->Deactivate();

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Landing Complete"));
			}
		}
		break;
	}

	default:
		break;
	}

	return;
}