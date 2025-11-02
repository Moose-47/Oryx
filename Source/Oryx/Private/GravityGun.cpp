#include "GravityGun.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

AGravityGun::AGravityGun()
{
    PrimaryActorTick.bCanEverTick = true;

    GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
    RootComponent = GunMesh;

    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void AGravityGun::BeginPlay()
{
    Super::BeginPlay();

    GunMesh->SetRelativeLocation(GunOffset);
    GunMesh->SetRelativeRotation(GunRotation);;
}

void AGravityGun::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HeldComponent && PhysicsHandle)
    {
        FVector HoldLocation = GunMesh->GetComponentLocation() + GunMesh->GetForwardVector() * HoldDistance;
        PhysicsHandle->SetTargetLocationAndRotation(HoldLocation, HeldComponent->GetComponentRotation());
    }
}

void AGravityGun::Grab()
{
    if (!PhysicsHandle || HeldComponent) return;

    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 500.f;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_PhysicsBody, Params))
    {
        UPrimitiveComponent* HitComp = Hit.GetComponent();
        if (HitComp && HitComp->IsSimulatingPhysics())
        {
            HeldComponent = HitComp;
            PhysicsHandle->GrabComponentAtLocationWithRotation(HeldComponent, NAME_None, HitComp->GetComponentLocation(), HitComp->GetComponentRotation());
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
