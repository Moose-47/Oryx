// Fill out your copyright notice in the Description page of Project Settings.


#include "LandingPad.h"
#include "SpaceshipPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ALandingPad::ALandingPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Mesh for the pad
	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	RootComponent = PadMesh;

	//Trigger Sphere
	LandingTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("LandingTrigger"));
	LandingTrigger->SetupAttachment(RootComponent);
	LandingTrigger->SetSphereRadius(500.f);
	LandingTrigger->SetCollisionProfileName(TEXT("Trigger"));

	//Bind overlap events
	LandingTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALandingPad::OnOverlapBegin);
	LandingTrigger->OnComponentEndOverlap.AddDynamic(this, &ALandingPad::OnOverlapEnd);
}

// Called when the game starts or when spawned
void ALandingPad::BeginPlay()
{
	Super::BeginPlay();	
}



void ALandingPad::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only check if a ship is inside the trigger
    if (OverlappingShip && bIsInsideTrigger)
    {
        const float ShipZ = OverlappingShip->GetActorLocation().Z;
        const float PadZ = GetActorLocation().Z;

        // Landing is allowed only if inside trigger AND above pad
        bool bWasCanLand = bCanLand;
        bCanLand = (ShipZ > PadZ);

        if (bWasCanLand != bCanLand)
        {
            if (bCanLand)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Press E to enter landing mode"));
                }
                OverlappingShip->OverlappingLandingPad = this;
            }
            else
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Too low to enter landing mode"));
                }
                OverlappingShip->OverlappingLandingPad = nullptr;
            }
        }
    }
}

void ALandingPad::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ASpaceshipPawn* Ship = Cast<ASpaceshipPawn>(OtherActor);
    if (Ship)
    {
        OverlappingShip = Ship;
        bIsInsideTrigger = true; // Ship is now inside trigger
    }
}

void ALandingPad::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == OverlappingShip)
    {
        OverlappingShip->OverlappingLandingPad = nullptr;
        OverlappingShip = nullptr;
        bCanLand = false;
        bIsInsideTrigger = false;
    }
}