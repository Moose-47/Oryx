#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityGun.generated.h"

class UStaticMeshComponent;
class UPhysicsHandleComponent;

UCLASS()
class ORYX_API AGravityGun : public AActor
{
    GENERATED_BODY()

public:
    AGravityGun();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* GunMesh;

    UPROPERTY(VisibleAnywhere)
    UPhysicsHandleComponent* PhysicsHandle;

    // Held object
    UPROPERTY()
    UPrimitiveComponent* HeldComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FVector GunOffset = FVector(50.f, 20.f, -10.f); //Forward, Right, Down offsets

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FRotator GunRotation = FRotator(90.f, 0.f, 90.f); //Pitch, Yaw, Roll

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float HoldDistance = 200.f;


public:
    void Grab();
    void Release();
};
