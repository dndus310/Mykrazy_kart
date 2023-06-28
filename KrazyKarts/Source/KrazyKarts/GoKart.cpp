// Fill out your copyright notice in the Description page of Project Settings.

#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/InputComponent.h"

#include "GoKart.h"


// Sets default values
AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		//NetUpdateFrequency = 1.f;
	}
}

void AGoKart::SimulateMove(FGoKartMove Move)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * Move.DeltaTime;
	
	ApplyRotation(Move);

	UpdateLocationFromVelocity(Move);
}

FString GetNetRoleAsString(ENetRole Role)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENetRole"), true);
	if (!EnumPtr) return FString("Invalid");

	return EnumPtr->GetNameStringByIndex((int32)Role);
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		FGoKartMove Move;
		Move.DeltaTime = DeltaTime;
		Move.SteeringThrow = SteeringThrow;
		Move.Throttle = Throttle;

		C2S_Move(Move);
	}
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.f; // g
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingCoefficient * NormalForce;
}

void AGoKart::ApplyRotation(FGoKartMove Move)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity.GetSafeNormal()) * Velocity.Size() * Move.DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * Move.SteeringThrow;
	FQuat RotationDelta = FQuat(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(FGoKartMove Move)
{
	FVector Translation = Velocity * 100 * Move.DeltaTime;

	FHitResult Result;

	AddActorWorldOffset(Translation, true, &Result);

	if (Result.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

bool AGoKart::C2S_Move_Validate(FGoKartMove Move)
{
	return true;
}

void AGoKart::C2S_Move_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void AGoKart::OnRep_ReplicatedState()
{
	SetActorTransform(ServerState.Transform);
}

