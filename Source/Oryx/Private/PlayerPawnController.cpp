#include "PlayerPawnController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GravityGun.h"

APlayerPawnController::APlayerPawnController()
{
    PrimaryActorTick.bCanEverTick = true;

    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->InitCapsuleSize(42.f, 88.f);
    Capsule->SetSimulatePhysics(true);
    Capsule->SetEnableGravity(false);
    Capsule->SetLinearDamping(5.f);
    Capsule->SetAngularDamping(1.f);
    Capsule->SetCollisionProfileName("PhysicsActor");
    RootComponent = Capsule;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Capsule);
    Mesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Capsule);
    Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
    Camera->bUsePawnControlRotation = false;
}

void APlayerPawnController::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (MappingContext)
                Subsystem->AddMappingContext(MappingContext, 0);
        }
    }

    //Spawn gravity gun
    if (GravityGunClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        GravityGun = GetWorld()->SpawnActor<AGravityGun>(GravityGunClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (GravityGun)
        {
            GravityGun->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}

void APlayerPawnController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckGrounded();

    FVector Velocity = Capsule->GetPhysicsLinearVelocity();

    //Gravity
    if (!bIsGrounded)
    {
        Velocity.Z += Gravity * DeltaTime;
        if (Velocity.Z < TerminalVelocity) Velocity.Z = TerminalVelocity;
    }
    else if (Velocity.Z < 0.f)
    {
        Velocity.Z = 0.f;
    }

    //Horizontal movement
    FVector Forward = Camera->GetForwardVector();
    FVector Right = Camera->GetRightVector();
    Forward.Z = 0.f; Forward.Normalize();
    Right.Z = 0.f; Right.Normalize();

    FVector2D DesiredDir(MoveInput.X, MoveInput.Y);
    if (DesiredDir.SizeSquared() > 1.f) DesiredDir.Normalize();

    FVector2D TargetVel = DesiredDir * MoveSpeed;
    FVector2D DeltaVel = TargetVel - CurrentHorizontalVelocity;
    FVector2D Accel = DeltaVel.GetClampedToMaxSize(MoveAcceleration * DeltaTime);
    CurrentHorizontalVelocity += Accel;

    Velocity.X = Forward.X * CurrentHorizontalVelocity.X + Right.X * CurrentHorizontalVelocity.Y;
    Velocity.Y = Forward.Y * CurrentHorizontalVelocity.X + Right.Y * CurrentHorizontalVelocity.Y;

    Capsule->SetPhysicsLinearVelocity(Velocity);
}

void APlayerPawnController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(ForwardAction, ETriggerEvent::Started, this, &APlayerPawnController::ForwardPressed);
        EIC->BindAction(ForwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::ForwardReleased);
        EIC->BindAction(BackwardAction, ETriggerEvent::Started, this, &APlayerPawnController::BackwardPressed);
        EIC->BindAction(BackwardAction, ETriggerEvent::Completed, this, &APlayerPawnController::BackwardReleased);
        EIC->BindAction(LeftAction, ETriggerEvent::Started, this, &APlayerPawnController::LeftPressed);
        EIC->BindAction(LeftAction, ETriggerEvent::Completed, this, &APlayerPawnController::LeftReleased);
        EIC->BindAction(RightAction, ETriggerEvent::Started, this, &APlayerPawnController::RightPressed);
        EIC->BindAction(RightAction, ETriggerEvent::Completed, this, &APlayerPawnController::RightReleased);

        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerPawnController::Look);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerPawnController::Jump);

        //Gravity gun
        EIC->BindAction(GravityGrabAction, ETriggerEvent::Started, this, &APlayerPawnController::GrabObject);
        EIC->BindAction(GravityGrabAction, ETriggerEvent::Completed, this, &APlayerPawnController::ReleaseObject);
    }
}

//Input methods
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
    AddActorLocalRotation(FRotator(0.f, LookValue.X * MouseSensitivity, 0.f));
    CameraPitch = FMath::Clamp(CameraPitch - LookValue.Y * MouseSensitivity, -85.f, 85.f);
    Camera->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
}

void APlayerPawnController::Jump(const FInputActionValue&)
{
    if (bIsGrounded)
    {
        FVector Velocity = Capsule->GetPhysicsLinearVelocity();
        Velocity.Z = JumpVelocity;
        Capsule->SetPhysicsLinearVelocity(Velocity);
        bIsGrounded = false;
    }
}

void APlayerPawnController::GrabObject() { if (GravityGun) GravityGun->Grab(); }
void APlayerPawnController::ReleaseObject() { if (GravityGun) GravityGun->Release(); }

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
