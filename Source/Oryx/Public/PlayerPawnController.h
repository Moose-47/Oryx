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

    // Components
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* Capsule;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    // Input
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
    UPROPERTY(EditAnywhere, Category = "Inputs")
    UInputAction* GravityGrabAction;

    // Gravity Gun
    UPROPERTY(EditAnywhere, Category = "GravityGun")
    TSubclassOf<AGravityGun> GravityGunClass;

    UPROPERTY()
    AGravityGun* GravityGun;

    // Movement
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
    bool bIsGrounded = true;

    FVector2D MoveInput = FVector2D::ZeroVector;
    FVector2D CurrentHorizontalVelocity = FVector2D::ZeroVector;

    float CameraPitch = 0.f;

    // Functions
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

    void GrabObject();
    void ReleaseObject();

    void CheckGrounded();
};
