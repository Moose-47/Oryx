// Fill out your copyright notice in the Description page of Project Settings.


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

//Constructor - Sets up component heirarchy, physics, and vfx
ASpaceshipPawn::ASpaceshipPawn()
{
	PrimaryActorTick.bCanEverTick = true; //Enable ticking every frame for physics and rotation updates

	//Static mesh acts as the physical body for the ship
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh;

	//Enable physics simulation for movement via forces
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetEnableGravity(false);
	ShipMesh->SetLinearDamping(0.3f);
	ShipMesh->SetAngularDamping(0.4f);

	//Thruster Scene Components
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


	//Niagara FX Components
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
}

//Called when the game starts or when spawned
//Handling setup of input context, cursor settings, and FX assets
void ASpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();
	
	//Input mode and Cursor Configuration
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true; //Displays the cursore for the mouse-based direction input

		//Allow mouse to move freely on screen
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	//Add input mapping context
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

	//Assign Niagara effects (if specified in editor)
	if (MainThrusterEffect) MainThrusterFX->SetAsset(MainThrusterEffect);
	if (LeftThrusterEffect) LeftThrusterFX->SetAsset(LeftThrusterEffect);
	if (RightThrusterEffect) RightThrusterFX->SetAsset(RightThrusterEffect);
	if (LeftBrakeThrusterEffect) LeftBrakeThrusterFX->SetAsset(LeftBrakeThrusterEffect);
	if (RightBrakeThrusterEffect) RightBrakeThrusterFX->SetAsset(RightBrakeThrusterEffect);
}

// Called every frame
void ASpaceshipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Update input-based states
	RestrictMouseToCircle(); //Clamp mouse offset for steering
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
	}
}

//Input Callbacks - Updating internal bool states
void ASpaceshipPawn::OnForwardThrust(const FInputActionValue& Value) { bForwardThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnLeftThrust(const FInputActionValue& Value) { bLeftThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnRightThrust(const FInputActionValue& Value) { bRightThrust = Value.Get<bool>(); }
void ASpaceshipPawn::OnAllThrusters(const FInputActionValue& Value) { bAllThrusters = Value.Get<bool>(); }
void ASpaceshipPawn::OnBrake(const FInputActionValue& Value) { bBrake = Value.Get<bool>(); }

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