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

#pragma region Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* GunMesh;

    UPROPERTY(VisibleAnywhere)
    UPhysicsHandleComponent* PhysicsHandle;

    UPROPERTY()
    UPrimitiveComponent* HeldComponent;
#pragma endregion

    FRotator HeldTargetRotation;

#pragma region Gravity Gun Transforms
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FVector GunOffset = FVector(35.f, 50.f, -40.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Transform")
    FRotator GunRotation = FRotator(90.f, 0.f, 90.f);
#pragma endregion

#pragma region Gravity Gun Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float HoldDistance = 200.f; //Distance object is held in front of camera

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float RotationSpeed = 90.f; //Speed of manual rotation (deg/sec)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float MaxSpinSpeed = 720.f; //Max Spin speed (deg/sec)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float SpinAcceleration = 5.f; //How quickly spin reaches max speed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float Range = 500.f; //Range to pick up objects

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
    float FireForce = 2000.f; //Impulse force when firing objects
#pragma endregion

    float CurrentSpinSpeed = 0.f;
    bool bSpinning = false;

    //Snap
    FRotator SnapTargetRotation;
    bool bSnapping = false;
    float SnapSpeed = 10.f; //Speed of interpolation when snapping

public:
    //Manual rotation flags 
    bool bRotateYawRight = false;
    bool bRotateYawLeft = false;
    bool bRotatePitchUp = false;
    bool bRotatePitchDown = false;

    //Actions
    void ToggleGrab(); //Grab or release object
    void Grab(); //grab object in front of player
    void Release(); //release currently held object

    void StartSpin(); //start spinning held object
    void StopSpin(); //stop spinning held object

    void SnapRotationToHorizontal(); //snap to horizontal plate
    void SnapRotationToVertical(); //Snap to vertical plane
    void SnapRotationForward(); //snap to Forward-facing rotation

    void FireObject(); //shoot object forward
};
