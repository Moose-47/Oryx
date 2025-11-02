#include "GravityGun.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

AGravityGun::AGravityGun()
{
    PrimaryActorTick.bCanEverTick = true;

    //Gun Mesh
    GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
    RootComponent = GunMesh;

    //Physics handle
    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
    //PhysicsHandle allows for smooth object movement using physics
}

void AGravityGun::BeginPlay()
{
    Super::BeginPlay();

    //Initalize the guns relative position and rotation
    GunMesh->SetRelativeLocation(GunOffset);
    GunMesh->SetRelativeRotation(GunRotation);
}

void AGravityGun::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HeldComponent || !PhysicsHandle) return;

    //Get player camera
    UCameraComponent* CameraComp = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    //Compute hold location in front of player
    FVector HoldLocation = CameraComp->GetComponentLocation() + CameraComp->GetForwardVector() * HoldDistance;

    //current rotation sotred as quaternion for smooth rotation
    FQuat CurrentQuat = HeldTargetRotation.Quaternion();

    //Camera-relative axes for rotation
    FVector Forward = CameraComp->GetForwardVector(); //For spinning
    FVector Right = CameraComp->GetRightVector();     //For pitch rotation
    FVector Up = CameraComp->GetUpVector();           //For yaw rotation

    //Spin
    if (bSpinning)
    {
        //Interpolate spin speed towards MaxSpinSpeed
        CurrentSpinSpeed = FMath::FInterpTo(CurrentSpinSpeed, MaxSpinSpeed, DeltaTime, SpinAcceleration);

        //Rotate around forward axis
        FQuat SpinDelta = FQuat(Forward, FMath::DegreesToRadians(CurrentSpinSpeed * DeltaTime));
        CurrentQuat = SpinDelta * CurrentQuat;
    }

    //Manual rotation
    if (bRotateYawRight || bRotateYawLeft)
    {
        float YawDir = bRotateYawRight ? 1.f : -1.f;
        FQuat YawDelta = FQuat(Up, FMath::DegreesToRadians(RotationSpeed * DeltaTime * YawDir));
        CurrentQuat = YawDelta * CurrentQuat;
        bSnapping = false;
    }

    if (bRotatePitchUp || bRotatePitchDown)
    {
        float PitchDir = bRotatePitchUp ? 1.f : -1.f;
        FQuat PitchDelta = FQuat(Right, FMath::DegreesToRadians(RotationSpeed * DeltaTime * PitchDir));
        CurrentQuat = PitchDelta * CurrentQuat;
        bSnapping = false;
    }

    //Snap rotation
    if (bSnapping)
    {
        FQuat TargetQuat = SnapTargetRotation.Quaternion();
        CurrentQuat = FQuat::Slerp(CurrentQuat, TargetQuat, DeltaTime * SnapSpeed);
        if (CurrentQuat.Equals(TargetQuat, 0.01f))
            bSnapping = false;
    }

    //Normalize rotation to avoid gimbal issues
    HeldTargetRotation = CurrentQuat.Rotator();
    HeldTargetRotation.Normalize();

    //Apply rotation and position to physics handle
    //PhysicsHandle moves the held object smoothly via physics
    PhysicsHandle->SetTargetLocationAndRotation(HoldLocation, HeldTargetRotation);
}

// Grab / Release
void AGravityGun::ToggleGrab() 
{ 
    if (!HeldComponent) Grab(); 
    else Release(); 
}

void AGravityGun::Grab()
{
    if (!PhysicsHandle || HeldComponent) return;

    UCameraComponent* CameraComp = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    FVector Start = CameraComp->GetComponentLocation();
    FVector End = Start + CameraComp->GetForwardVector() * Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    //Line trace to find object in front
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_PhysicsBody, Params))
    {
        UPrimitiveComponent* HitComp = Hit.GetComponent();
        if (HitComp && HitComp->IsSimulatingPhysics())
        {
            HeldComponent = HitComp;
            FVector ComponentCenter = HitComp->Bounds.Origin;

            //grab the object at its center with physics handle
            PhysicsHandle->GrabComponentAtLocationWithRotation(
                HeldComponent,
                NAME_None,
                ComponentCenter,
                HeldComponent->GetComponentRotation()
            );

            //reduce damping while holding for smooth movement
            HeldTargetRotation = HeldComponent->GetComponentRotation();
            HeldComponent->SetLinearDamping(1.f);
            HeldComponent->SetAngularDamping(1.f);
        }
    }
}

void AGravityGun::Release()
{
    if (PhysicsHandle && HeldComponent)
    {
        PhysicsHandle->ReleaseComponent();
        HeldComponent = nullptr;
    }
}

// Spin
void AGravityGun::StartSpin() { bSpinning = true; }
void AGravityGun::StopSpin() { bSpinning = false; CurrentSpinSpeed = 0.f; }

// Snap
void AGravityGun::SnapRotationToHorizontal()
{
    if (!HeldComponent) return;

    UCameraComponent* CameraComp = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    FRotator CameraRot = CameraComp->GetComponentRotation();
    SnapTargetRotation = FRotator(0.f, CameraRot.Yaw, 0.f); //Horizontal plane aligned with player
    bSnapping = true;
}

void AGravityGun::SnapRotationToVertical()
{
    if (!HeldComponent) return;

    UCameraComponent* CameraComp = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    FRotator CameraRot = CameraComp->GetComponentRotation();
    SnapTargetRotation = FRotator(90.f, CameraRot.Yaw, 0.f); //Vertical aligned with player's yaw
    bSnapping = true;
}

void AGravityGun::SnapRotationForward()
{
    if (!HeldComponent) return;

    UCameraComponent* CameraComp = GetOwner()->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    FRotator CameraRot = CameraComp->GetComponentRotation();
    SnapTargetRotation = FRotator(0.f, CameraRot.Yaw, 0.f); //Forward in front of player
    bSnapping = true;
}

// Fire
void AGravityGun::FireObject()
{
    if (!HeldComponent) return;
    UPrimitiveComponent* Prim = HeldComponent;
    FVector Forward = GetOwner()->FindComponentByClass<UCameraComponent>()->GetForwardVector();

    Release(); //Let go before applying physics impulse
    Prim->AddImpulse(Forward * FireForce, NAME_None, true); //Add impulse to simulate shooting
}
