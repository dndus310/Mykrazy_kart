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

	DOREPLIFETIME(AGoKart, ReplicatedTransform);
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
	//if (HasAuthority())
	//{
	//	NetUpdateFrequency = 1.f;
	//}
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
	
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedTransform = GetActorTransform();
	}

	if (IsReplicatedTransform)
	{
		FTransform InterpTransform = UKismetMathLibrary::TInterpTo(GetActorTransform(), ReplicatedTransform, DeltaTime, 1.f);
		SetActorTransform(InterpTransform);

		if (InterpTransform.Equals(ReplicatedTransform))
			IsReplicatedTransform = false;
	}
	
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetNetRoleAsString(GetLocalRole()), this, FColor::White, DeltaTime);
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

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity.GetSafeNormal()) * Velocity.Size() * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta = FQuat(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

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

bool AGoKart::C2S_MoveForward_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.f;
}

bool AGoKart::C2S_MoveRight_Validate(float Value)
{
	return FMath::Abs(Value) <= 1.f;
}

void AGoKart::C2S_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

void AGoKart::C2S_MoveRight_Implementation(float Value)
{
	SteeringThrow = Value;
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
	C2S_MoveForward(Throttle);
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
	C2S_MoveRight(SteeringThrow);
}

void AGoKart::OnRep_ReplicatedTransform()
{

	IsReplicatedTransform = true;
}

