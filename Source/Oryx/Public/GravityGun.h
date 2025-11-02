#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityGun.generated.h"

class UStaticMeshComponent;
class UPhysicsHandleComponent;
class UCameraComponent;

UCLASS()
class ORYX_API AGravityGun : public AActor
{
    GENERATED_BODY()

public:
    AGravityGun();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* GunMesh;

    UPROPERTY(VisibleAnywhere)
    UPhysicsHandleComponent* PhysicsHandle;

    UPROPERTY()
    UPrimitiveComponent* HeldComponent;

    FRotator HeldTargetRotation;

    //Gun transform
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FVector GunOffset = FVector(35.f, 50.f, -40.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FRotator GunRotation = FRotator(90.f, 0.f, 90.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float HoldDistance = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float RotationSpeed = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float MaxSpinSpeed = 720.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float SpinAcceleration = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float Range = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float FireForce = 2000.f;

    float CurrentSpinSpeed = 0.f;
    bool bSpinning = false;

    //Snap
    FRotator SnapTargetRotation;
    bool bSnapping = false;
    float SnapSpeed = 10.f;

public:
    //Manual rotation flags (set directly from PlayerPawnController)
    bool bRotateYawRight = false;
    bool bRotateYawLeft = false;
    bool bRotatePitchUp = false;
    bool bRotatePitchDown = false;

    //Actions
    void ToggleGrab();
    void Grab();
    void Release();

    void StartSpin();
    void StopSpin();

    void SnapRotationToHorizontal();
    void SnapRotationToVertical();
    void SnapRotationForward();

    void FireObject();
};
