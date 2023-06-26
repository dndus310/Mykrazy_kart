// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	FVector GetAirResistance();

	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime);

	void UpdateLocationFromVelocity(float DeltaTime);

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
	void C2S_MoveForward(float Value);
	void C2S_MoveForward_Implementation(float Value);
	bool C2S_MoveForward_Validate(float Value);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void C2S_MoveRight(float Value);
	void C2S_MoveRight_Implementation(float Value);
	bool C2S_MoveRight_Validate(float Value);

	void MoveForward(float Value);
	void MoveRight(float Value);

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;
	
	UPROPERTY(Replicated)
	FVector Velocity;
	
	UFUNCTION()
	virtual void OnRep_ReplicatedTransform();

	UPROPERTY(Replicated)
	float Throttle;

	UPROPERTY(Replicated)
	float SteeringThrow;
};
