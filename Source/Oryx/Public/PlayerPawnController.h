#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "PlayerPawnController.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class AGravityGun;

UCLASS()
class ORYX_API APlayerPawnController : public APawn
{
    GENERATED_BODY()

public:
    APlayerPawnController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

#pragma region Components
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* Capsule;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;
#pragma endregion

#pragma region Player Inputs
    UPROPERTY(EditAnywhere)
    UInputMappingContext* MappingContext;

    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* ForwardAction;
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* BackwardAction;
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* LeftAction;
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* RightAction;
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* LookAction;
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* JumpAction;
#pragma endregion

#pragma region Gravity Gun
    // Gravity Gun
    UPROPERTY(EditAnywhere, Category = "GravityGun")
    TSubclassOf<AGravityGun> GravityGunClass;
    UPROPERTY()
    AGravityGun* GravityGun;

    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* GravityGrabAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* RotateRightAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* RotateLeftAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* RotateForwardAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* RotateBackwardAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* HorizontalSnapAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* VerticalSnapAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* ForwardSnapAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* SpinAction;
    UPROPERTY(EditAnywhere, Category = "GravityGunInputs")
    UInputAction* FireAction;
#pragma endregion

#pragma region Movement Variables
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveAcceleration = 4000.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float JumpVelocity = 650.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float Gravity = -2000.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float TerminalVelocity = -1200.f;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
#pragma endregion

    bool bIsGrounded = true;

    FVector2D MoveInput = FVector2D::ZeroVector;
    FVector2D CurrentHorizontalVelocity = FVector2D::ZeroVector;

    float CameraPitch = 0.f;

#pragma region Player Functions
    void Look(const FInputActionValue& Value);
    void Jump(const FInputActionValue& Value);

    void ForwardPressed(const FInputActionValue&);
    void ForwardReleased(const FInputActionValue&);
    void BackwardPressed(const FInputActionValue&);
    void BackwardReleased(const FInputActionValue&);
    void RightPressed(const FInputActionValue&);
    void RightReleased(const FInputActionValue&);
    void LeftPressed(const FInputActionValue&);
    void LeftReleased(const FInputActionValue&);

    void CheckGrounded();
#pragma endregion


#pragma region Gravity Gun Functions
    //Gravity gun input
    void ToggleGrab();

    void SnapHorizontal();
    void SnapVertical();
    void SnapForward();

    void StartSpin();
    void StopSpin();

    void FireObject();

    void StartRotateRight();
    void StopRotateRight();

    void StartRotateLeft();
    void StopRotateLeft();

    void StartRotateForward();
    void StopRotateForward();

    void StartRotateBackward();
    void StopRotateBackward();
#pragma endregion
};
