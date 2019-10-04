// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "HoverPawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Category="Pawn Movement", ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class GENTLEGHOSTJAM_API UHoverPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
public:
	// Constructor and overrides
	UHoverPawnMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual float GetGravityZ() const override;

	// Max velocity magnitude allowed for the pawn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement", meta = (ClampMin = "0", UIMin = "0"))
	float MaxSpeed;

	/** Acceleration applied by input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement")
	float Acceleration;

	/** Deceleration applied when there is no input (rate of change of velocity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement")
	float Deceleration;

	/**
	 * Setting affecting extra force applied when changing direction, making turns have less drift and become more responsive.
	 * Velocity magnitude is not allowed to increase, that only happens due to normal acceleration. It may decrease with large direction changes.
	 * Larger values apply extra force to reach the target direction more quickly, while a zero value disables any extra turn force.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement", meta = (ClampMin = "0", UIMin = "0"))
	float TurningBoost;

protected:
	// Whether gravity should be applied to the movement component. Turn this off to disable gravity calculations in the MovementComponent.
	// (Gravity still needs to be turned off on actor's Root component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement")
	uint8 bApplyGravity:1;

	// Custom gravity scale, used to multiply with gravity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement", meta = (ClampMin = "0", UIMin = "0", ClampMax = "100.0", UIMax = "100.0"))
	float GravityScale;

	// Whether gravity should be applied to the movement component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn Movement|Debug")
	uint8 bShowDebugInfo : 1;

	/** Update Velocity based on input. Also applies gravity. */
	virtual void ApplyControlInputToVelocity(float DeltaTime);
};