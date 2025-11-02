#include "PlayerPawnController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GravityGun.h"

APlayerPawnController::APlayerPawnController()
{
    PrimaryActorTick.bCanEverTick = true;

    //Setup capsule (physics)
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->InitCapsuleSize(42.f, 88.f);
    Capsule->SetSimulatePhysics(true); //Enabled physics simulation
    Capsule->SetEnableGravity(false); //Disabling gravity, handling it manually
    Capsule->SetLinearDamping(5.f); //Damping horizontal velocity for smooth movement
    Capsule->SetAngularDamping(1000000.f); //Prevent unwanted rotations
    Capsule->SetCollisionProfileName("PhysicsActor");
    RootComponent = Capsule;

    //Setup mesh
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Capsule);
    Mesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //Setup camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Capsule);
    Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
    Camera->bUsePawnControlRotation = false; //Manually handling camera rotation
}

void APlayerPawnController::BeginPlay()
{
    Super::BeginPlay();

    //Add input mapping context
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (MappingContext)
                Subsystem->AddMappingContext(MappingContext, 0);
        }
    }

    //Spawning gravity gun
    if (GravityGunClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        GravityGun = GetWorld()->SpawnActor<AGravityGun>(GravityGunClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (GravityGun)
            GravityGun->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
    }
}

void APlayerPawnController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CheckGrounded();

    //Apply gravity
    FVector Velocity = Capsule->GetPhysicsLinearVelocity();
    if (!bIsGrounded)
    {
        Velocity.Z += Gravity * DeltaTime;
        if (Velocity.Z < TerminalVelocity) Velocity.Z = TerminalVelocity;
    }
    else if (Velocity.Z < 0.f)
    {
        Velocity.Z = 0.f; //Stop downward velocity when grounded to prevent bouncing
    }

    //Compute movement direction relative to camera
    FVector Forward = Camera->GetForwardVector();
    FVector Right = Camera->GetRightVector();
    Forward.Z = 0.f; Forward.Normalize(); //Flatten movement on XY plane
    Right.Z = 0.f; Right.Normalize();

    FVector2D DesiredDir = MoveInput;
    if (DesiredDir.SizeSquared() > 1.f) DesiredDir.Normalize();

    //Smooth acceleration
    FVector2D TargetVel = DesiredDir * MoveSpeed;
    FVector2D DeltaVel = TargetVel - CurrentHorizontalVelocity;
    FVector2D Accel = DeltaVel.GetClampedToMaxSize(MoveAcceleration * DeltaTime);
    CurrentHorizontalVelocity += Accel;

    //Convert 2D velocity to world 3D velocity
    Velocity.X = Forward.X * CurrentHorizontalVelocity.X + Right.X * CurrentHorizontalVelocity.Y;
    Velocity.Y = Forward.Y * CurrentHorizontalVelocity.X + Right.Y * CurrentHorizontalVelocity.Y;
     
    //Apply velocity to capsule
    Capsule->SetPhysicsLinearVelocity(Velocity);
}

void APlayerPawnController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
#pragma region Player Movement Inputs
        //Binding Movement actions
        EIC->BindAction(ForwardAction, ETriggerEvent::Started, this, &APlayerPawnController::ForwardPressed);
        EIC->BindAction(ForwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::ForwardReleased);

        EIC->BindAction(BackwardAction, ETriggerEvent::Started, this, &APlayerPawnController::BackwardPressed);
        EIC->BindAction(BackwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::BackwardReleased);

        EIC->BindAction(LeftAction, ETriggerEvent::Started, this, &APlayerPawnController::LeftPressed);
        EIC->BindAction(LeftAction, ETriggerEvent::Completed, this, &APlayerPawnController::LeftReleased);

        EIC->BindAction(RightAction, ETriggerEvent::Started, this, &APlayerPawnController::RightPressed);
        EIC->BindAction(RightAction, ETriggerEvent::Completed, this, &APlayerPawnController::RightReleased);

        //Binding Look & Jump actions
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerPawnController::Look);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerPawnController::Jump);
#pragma endregion
        
#pragma region Gravity Gun Inputs
        //gravity gun grab/release action
        EIC->BindAction(GravityGrabAction, ETriggerEvent::Started, this, &APlayerPawnController::ToggleGrab);

        //Gravity gun rotation actions
        EIC->BindAction(RotateRightAction, ETriggerEvent::Started, this, &APlayerPawnController::StartRotateRight);
        EIC->BindAction(RotateRightAction, ETriggerEvent::Completed, this, &APlayerPawnController::StopRotateRight);

        EIC->BindAction(RotateLeftAction, ETriggerEvent::Started, this, &APlayerPawnController::StartRotateLeft);
        EIC->BindAction(RotateLeftAction, ETriggerEvent::Completed, this, &APlayerPawnController::StopRotateLeft);

        EIC->BindAction(RotateForwardAction, ETriggerEvent::Started, this, &APlayerPawnController::StartRotateForward);
        EIC->BindAction(RotateForwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::StopRotateForward);

        EIC->BindAction(RotateBackwardAction, ETriggerEvent::Started, this, &APlayerPawnController::StartRotateBackward);
        EIC->BindAction(RotateBackwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::StopRotateBackward);

        //Gravity gun Snap & spin actions
        EIC->BindAction(HorizontalSnapAction, ETriggerEvent::Started, this, &APlayerPawnController::SnapHorizontal);
        EIC->BindAction(VerticalSnapAction, ETriggerEvent::Started, this, &APlayerPawnController::SnapVertical);
        EIC->BindAction(ForwardSnapAction, ETriggerEvent::Started, this, &APlayerPawnController::SnapForward);

        EIC->BindAction(SpinAction, ETriggerEvent::Started, this, &APlayerPawnController::StartSpin);
        EIC->BindAction(SpinAction, ETriggerEvent::Completed, this, &APlayerPawnController::StopSpin);

        //gravity gun fire action
        EIC->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerPawnController::FireObject);
#pragma endregion
    }
}

//Movement input handlers
void APlayerPawnController::ForwardPressed(const FInputActionValue&) { MoveInput.X = 1.f; }
void APlayerPawnController::ForwardReleased(const FInputActionValue&) { if (MoveInput.X > 0.f) MoveInput.X = 0.f; }
void APlayerPawnController::BackwardPressed(const FInputActionValue&) { MoveInput.X = -1.f; }
void APlayerPawnController::BackwardReleased(const FInputActionValue&) { if (MoveInput.X < 0.f) MoveInput.X = 0.f; }
void APlayerPawnController::RightPressed(const FInputActionValue&) { MoveInput.Y = 1.f; }
void APlayerPawnController::RightReleased(const FInputActionValue&) { if (MoveInput.Y > 0.f) MoveInput.Y = 0.f; }
void APlayerPawnController::LeftPressed(const FInputActionValue&) { MoveInput.Y = -1.f; }
void APlayerPawnController::LeftReleased(const FInputActionValue&) { if (MoveInput.Y < 0.f) MoveInput.Y = 0.f; }

void APlayerPawnController::Look(const FInputActionValue& Value)
{
    FVector2D LookValue = Value.Get<FVector2D>();
    if (LookValue.IsNearlyZero()) return;
    float MouseSensitivity = 0.35f;

    //Yaw rotates the whole pawn
    AddActorLocalRotation(FRotator(0.f, LookValue.X * MouseSensitivity, 0.f));

    //Pitch rotates camera only
    CameraPitch = FMath::Clamp(CameraPitch - LookValue.Y * MouseSensitivity, -85.f, 85.f);
    Camera->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
}

void APlayerPawnController::Jump(const FInputActionValue&)
{
    if (bIsGrounded)
    {
        FVector Velocity = Capsule->GetPhysicsLinearVelocity();
        Velocity.Z = JumpVelocity; //Add upward velocity ensuring jump height is always the same
        Capsule->SetPhysicsLinearVelocity(Velocity);
        bIsGrounded = false;
    }
}

//Check if player is grounded using line trace
void APlayerPawnController::CheckGrounded()
{
    if (!GetWorld()) return;
    FVector Start = Capsule->GetComponentLocation();
    FVector End = Start - FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() + 5.f);
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    bIsGrounded = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}

#pragma region GravityGunMethods
void APlayerPawnController::ToggleGrab() { if (GravityGun) GravityGun->ToggleGrab(); }
void APlayerPawnController::SnapHorizontal() { if (GravityGun) GravityGun->SnapRotationToHorizontal(); }
void APlayerPawnController::SnapVertical() { if (GravityGun) GravityGun->SnapRotationToVertical(); }
void APlayerPawnController::SnapForward() { if (GravityGun) GravityGun->SnapRotationForward(); }
void APlayerPawnController::StartSpin() { if (GravityGun) GravityGun->StartSpin(); }
void APlayerPawnController::StopSpin() { if (GravityGun) GravityGun->StopSpin(); }
void APlayerPawnController::FireObject() { if (GravityGun) GravityGun->FireObject(); }

//Manual rotation 
void APlayerPawnController::StartRotateRight() { if (GravityGun) GravityGun->bRotateYawRight = true; }
void APlayerPawnController::StopRotateRight() { if (GravityGun) GravityGun->bRotateYawRight = false; }

void APlayerPawnController::StartRotateLeft() { if (GravityGun) GravityGun->bRotateYawLeft = true; }
void APlayerPawnController::StopRotateLeft() { if (GravityGun) GravityGun->bRotateYawLeft = false; }

void APlayerPawnController::StartRotateForward() { if (GravityGun) GravityGun->bRotatePitchUp = true; }
void APlayerPawnController::StopRotateForward() { if (GravityGun) GravityGun->bRotatePitchUp = false; }

void APlayerPawnController::StartRotateBackward() { if (GravityGun) GravityGun->bRotatePitchDown = true; }
void APlayerPawnController::StopRotateBackward() { if (GravityGun) GravityGun->bRotatePitchDown = false; }
#pragma endregion
