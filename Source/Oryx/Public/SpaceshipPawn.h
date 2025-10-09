#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "SpaceshipPawn.generated.h"

class UInputAction;
class UInputMappingContext;
class UNiagaraComponent;

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

	//Resict mouse movement to circular radius and calculate offset
	void RestrictMouseToCircle();

	//Handles smooth rotation towards mouse offset
	void UpdateRotation(float DeltaTime);

	//Applied currently active thrusts
	void ApplyThrusters(float DeltaTime);

	//Input Action Callbacks
	void OnForwardThrust(const FInputActionValue& Value);
	void OnLeftThrust(const FInputActionValue& Value);
	void OnRightThrust(const FInputActionValue& Value);
	void OnAllThrusters(const FInputActionValue& Value);
	void OnBrake(const FInputActionValue& Value);

protected:
	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ship")
	UStaticMeshComponent* ShipMesh;

	//Input Actions
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

	// Thruster particles
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

	//State
	FVector2D MouseOffset;

	bool bForwardThrust = false;
	bool bLeftThrust = false;
	bool bRightThrust = false;
	bool bAllThrusters = false;
	bool bBrake = false;
};
