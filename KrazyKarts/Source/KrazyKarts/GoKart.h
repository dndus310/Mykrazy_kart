// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;
};

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;
		
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGoKartMove LastMove;

};

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void SimulateMove(FGoKartMove Move);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FVector GetAirResistance();
	FVector GetRollingResistance();

	void ApplyRotation(FGoKartMove Move);

	void UpdateLocationFromVelocity(FGoKartMove Move);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	float Mass = 10000.f;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 20000.f;

	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10.f;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16.f;

	UPROPERTY(EditAnywhere)
	float RollingCoefficient = 0.015f;
	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void C2S_Move(FGoKartMove Move);

	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedState)
	FGoKartState ServerState;

	FVector Velocity;

	float Throttle;
	float SteeringThrow;

	UFUNCTION()
	virtual void OnRep_ReplicatedState();
};
