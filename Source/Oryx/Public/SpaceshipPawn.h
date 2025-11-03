#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SpaceshipPawn.generated.h"

//Forward class declarations tell the compiler that the class exists and will be defined elsewhere
//This allows me to declare pointers (*) or references (&) to objects of that class
//I cannot access any member variables or functions of the class until I include its full definition (header)
#pragma region Forward Class Declarations
class UInputAction;
class UInputMappingContext;
class UNiagaraComponent;
class ALandingPad;
#pragma endregion

UENUM(BlueprintType)
enum class ELandingStage : uint8 //uint8 to use 1 byte in memory
{
	None,
	RotateToPad,        // Rotate to face pad
	MoveToPad,          // Move forward towards pad
	ApplyBrakes,        // Activate brakes VFX
	AlignRotation,      // Match pad rotation
	Descend,            // Descend vertically
	Landed
};

UCLASS()
class ORYX_API ASpaceshipPawn : public APawn
{
	GENERATED_BODY()

public:
	ASpaceshipPawn();

protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

#pragma region Functions
	//Resict mouse movement to circular radius and calculate offset
	void RestrictMouseToCircle();

	//Handles smooth rotation towards mouse offset
	void UpdateRotation(float DeltaTime);

	//Applied currently active thrusts
	void ApplyThrusters(float DeltaTime);

	//Begins the landing process
	void StartLanding(ALandingPad* LandingPad);

	//Function handles the actual landing functionality
	void LandingSequence(float DeltaTime);

	void LockShipOnPad(bool bLock);
	void StartTakeoff();
	void OnExitShip();
#pragma endregion

#pragma region Input Action Callbacks
	void OnForwardThrust(const FInputActionValue& Value);
	void OnLeftThrust(const FInputActionValue& Value);
	void OnRightThrust(const FInputActionValue& Value);
	void OnAllThrusters(const FInputActionValue& Value);
	void OnBrake(const FInputActionValue& Value);
	void OnLand(const FInputActionValue& Value);
	
#pragma endregion
  
protected:
#pragma region Ship mesh and Thruster Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	UStaticMeshComponent* ShipMesh;

	//Empty scene components for position thrusters forces
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Thrusters")
	USceneComponent* MainThruster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Thrusters")
	USceneComponent* LeftThruster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Thrusters")
	USceneComponent* RightThruster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Thrusters")
	USceneComponent* ReverseLeftThruster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Thrusters")
	USceneComponent* ReverseRightThruster;
#pragma endregion

#pragma region Input Actions
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* ShipMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_ForwardThrust;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_LeftThrust;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_RightThrust;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_AllThrusters;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Boost;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Brake;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Land;
#pragma endregion

#pragma region Rotation Settings and Thruster forces
	//Rotation Settings
	UPROPERTY(EditAnywhere, Category = "Ship|Rotation")
	float MaxMouseRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "Ship|Rotation")
	float RotationInterpSpeed = 2.0f;

	//Force Values
	UPROPERTY(EditAnywhere, Category = "Ship|Forces")
	float ForwardThrusterForce = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Ship|Forces")
	float SideThrusterForce = 5000.f;
#pragma endregion

#pragma region Thruster particle effects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Effects")
	UNiagaraComponent* MainThrusterFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Effects")
	UNiagaraComponent* LeftThrusterFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Effects")
	UNiagaraComponent* RightThrusterFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Effects")
	UNiagaraComponent* LeftBrakeThrusterFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship|Effects")
	UNiagaraComponent* RightBrakeThrusterFX;

	UPROPERTY(EditAnywhere, Category = "Ship|Effects")
	UNiagaraSystem* MainThrusterEffect;

	UPROPERTY(EditAnywhere, Category = "Ship|Effects")
	UNiagaraSystem* LeftThrusterEffect;

	UPROPERTY(EditAnywhere, Category = "Ship|Effects")
	UNiagaraSystem* RightThrusterEffect;

	UPROPERTY(EditAnywhere, Category = "Ship|Effects")
	UNiagaraSystem* LeftBrakeThrusterEffect;

	UPROPERTY(EditAnywhere, Category = "Ship|Effects")
	UNiagaraSystem* RightBrakeThrusterEffect;
#pragma endregion

#pragma region Variables
	FVector2D MouseOffset;

	//State
	bool bForwardThrust = false;
	bool bLeftThrust = false;
	bool bRightThrust = false;
	bool bAllThrusters = false;
	bool bBrake = false;
	bool bIsLanding = false;
#pragma endregion

#pragma region Landing
	ALandingPad* TargetLandingPad = nullptr;

	UPROPERTY(EditAnywhere, Category = "Landing")
	float LandingMoveSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Landing")
	float LandingRotateSpeed = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Landing")
	float LandingDescendSpeed = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing")
	ELandingStage LandingStage = ELandingStage::None;

public:
	ALandingPad* OverlappingLandingPad = nullptr;
#pragma endregion

	UPROPERTY(EditAnywhere, Category = "Ship")
	TSubclassOf<APawn> PlayerPawnClass;

	UFUNCTION()
	void TryBoard(APlayerPawnController* PlayerPawn);
};
