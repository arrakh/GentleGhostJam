// Fill out your copyright notice in the Description page of Project Settings.

#include "HoverPawnMovementComponent.h"
#include "GentleGhostJam.h"

UHoverPawnMovementComponent::UHoverPawnMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bApplyGravity = true;
	GravityScale = 1.f;
	MaxSpeed = 600.f;
	Acceleration = 2000.f;
	Deceleration = 4000.f;
	TurningBoost = 4.f;
}

void UHoverPawnMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check that everything is still valid and pawn is allowed to move
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	const AController* Controller = PawnOwner->GetController();
	if (Controller && Controller->IsLocalPlayerController())
	{
		// Apply input for local player controllers and add to velocity
		ApplyControlInputToVelocity(DeltaTime);

		Velocity = FVector(Velocity.X, Velocity.Y, 0.f);

		if (bApplyGravity)
		{
			// Apply gravity to the velocity
			FVector Gravity(0.f, 0.f, GetGravityZ());
			Velocity += Gravity * GravityScale * DeltaTime;
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(1, 2.f, FColor::Green, FString::Printf(TEXT("Gravity velocity: %s"), *Velocity.ToString()));
		}

		// Apply velocity to actor
		// @see APawn::AddMovementInput()
		FVector Delta = Velocity * DeltaTime;
		if (!Delta.IsNearlyZero(1e-6f))
		{
			const FVector OldLocation = UpdatedComponent->GetComponentLocation();
			const FQuat Rotation = UpdatedComponent->GetComponentQuat();
			
			FHitResult Hit(1.f);

			SafeMoveUpdatedComponent(Delta, Rotation, true, Hit);

			if (Hit.IsValidBlockingHit())
			{
				HandleImpact(Hit, DeltaTime, Delta);
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Green, FString::Printf(TEXT("Hit location: %s"), *Hit.Location.ToString()));
				// Try to slide the remaining distance along the surface.
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit);
			}

			// Update velocity
			const FVector NewLocation = UpdatedComponent->GetComponentLocation();
			Velocity = ((NewLocation - OldLocation) / DeltaTime);

		}
		// Finalize
		UpdateComponentVelocity();
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(2, 2.f, FColor::Green, FString::Printf(TEXT("Delta location: %s"), *Delta.ToString()));
	}

}

float UHoverPawnMovementComponent::GetGravityZ() const
{
	return Super::GetGravityZ() * GravityScale;
}

void UHoverPawnMovementComponent::ApplyControlInputToVelocity(float DeltaTime)
{
	const FVector ControlAcceleration = GetPendingInputVector().GetClampedToMaxSize(1.f);

	const float AnalogInputModifier = (ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f);
	const float MaxPawnSpeed = MaxSpeed * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed(MaxPawnSpeed);

	if (AnalogInputModifier > 0.f && !bExceedingMaxSpeed)
	{
		// Apply change in velocity direction
		if (Velocity.SizeSquared() > 0.f)
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude
			const float TimeScale = FMath::Clamp(DeltaTime * TurningBoost, 0.f, 1.f);
			Velocity = Velocity + (ControlAcceleration * Velocity.Size() - Velocity) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration
		if (Velocity.SizeSquared() > 0.f)
		{
			const FVector OldVelocity = Velocity;
			const float VelSize = FMath::Max(Velocity.Size() - FMath::Abs(Deceleration) * DeltaTime, 0.f);
			Velocity = Velocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it
			if (bExceedingMaxSpeed && Velocity.SizeSquared() < FMath::Square(MaxPawnSpeed))
			{
				Velocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude
	const float NewMaxSpeed = bExceedingMaxSpeed ? Velocity.Size() : MaxPawnSpeed;
	Velocity += ControlAcceleration * FMath::Abs(Acceleration) * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(NewMaxSpeed);

	ConsumeInputVector();
}
