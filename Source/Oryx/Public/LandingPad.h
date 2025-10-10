// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "LandingPad.generated.h"

class USphereComponent;
class ASpaceshipPawn;

UCLASS()
class ORYX_API ALandingPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALandingPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* Othercomp, int32 OtherBodyIndex, 
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing Pad")
	UStaticMeshComponent* PadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing Pad")
	USphereComponent* LandingTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ASpaceshipPawn* OverlappingShip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bCanLand = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInsideTrigger = false;
};
