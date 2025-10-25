#include "LocomotionAnim/ARLAnimInstance.h"

// ---------- Engine / Core ----------
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// ---------- Project ----------
#include "Character/ARLCharacter.h"
#include "Data/ARL_Enums.h"

/* =============================================================================
 * Update (States / Gates / Direction)
 * ============================================================================= */

void UARLAnimInstance::UpdateAnimationState(AARLCharacter* Character, EStates State)
{
	if (Character)
	{
		CurrentState = State;
	}
}

void UARLAnimInstance::UpdateGateStateAnim(EGatesStates LastGate, EGatesStates Gate)
{
	CurrenteGate = Gate;
	if (LastGate == EGatesStates::None)
	{
		return;
	}

	LastFrameGate = LastGate;
}

void UARLAnimInstance::UpdateGateVelocityDirection(EVelocityLocomotionDirection Direction)
{
	VelocityLocomotionDirection = Direction;
}

/* =============================================================================
 * Stop Cue
 * ============================================================================= */

void UARLAnimInstance::ApplyStopCue(uint8 InSeqIndex, int32 InNonce, float InStopDistance)
{
	StopSeqIndex = InSeqIndex;
	StopNonce = InNonce;
	NewStopDistance = InStopDistance;
	BP_OnApplyStopCue();
}

/* =============================================================================
 * Lifecycle
 * ============================================================================= */

void UARLAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UARLAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AARLCharacter* Char = Cast<AARLCharacter>(TryGetPawnOwner());
	if (!Char) return;

	if (GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	const UCharacterMovementComponent* Move = Char->GetCharacterMovement();
	if (!Move) return;

	float Target = 0.f;

	if (Move->IsFalling())
	{
		const FFindFloorResult& Floor = Move->CurrentFloor;
		if (Floor.bBlockingHit)
		{
			Target = Floor.FloorDist;
		}
		else
		{
			Target = MaxNoFloorDistance;
		}
	}
	else
	{
		Target = 0.f;
	}

	GroundDistance = Target;

	const bool   bOnGround = Move->IsMovingOnGround();
	const FVector Loc = Char->GetActorLocation();
	const FVector Vel2D = FVector(Move->Velocity.X, Move->Velocity.Y, 0.f);
	const float  Speed = Vel2D.Size();

	if (!bOnGround || Speed < KINDA_SMALL_NUMBER)
	{
		StopDistance2D = 0.f;
		DistForMatch = 0.f;
		PredictedStopLocation = Loc;
		bCanEnterStop = false;
		return;
	}

	float a = FMath::Max(Move->GetMaxBrakingDeceleration(), 1.f);
	if (Move->CurrentFloor.bBlockingHit)
	{
		const float slopeZ = Move->CurrentFloor.HitResult.ImpactNormal.Z;
		a *= FMath::Clamp(slopeZ, 0.7f, 1.3f);
	}

	const float s = (Speed * Speed) / (2.f * a);
	StopDistance2D = s;
	PredictedStopLocation = Loc + Vel2D.GetSafeNormal() * s;

	// ---------- Proxy handling: enter earlier + lead distance ----------
	const bool bProxy = (Char->GetLocalRole() == ROLE_SimulatedProxy) && !Char->IsLocallyControlled();

	const float EnterDist = bProxy ? EnterDist_Proxy : EnterDist_Owner;
	const float MinSpeed = bProxy ? MinSpeed_Proxy : MinSpeed_Owner;

	bool bTryEnterStop = Move->IsMovingOnGround() && (Speed > MinSpeed) && (StopDistance2D <= EnterDist);
	bool bCanEnterStopFinal = bTryEnterStop && !bInStopState && !bStopCooldown;

	// Lead: anticipate ~0.25s of path for distance match on proxies
	float Lead = 0.f;
	if (bProxy)
	{
		Lead = FMath::Clamp(Speed * 0.25f, 60.f, 250.f); // tune if needed
	}

	DistForMatch = FMath::Clamp(StopDistance2D + Lead, 0.f, CurveMaxDist);

	bCanEnterStop = (Speed > MinSpeed) && (StopDistance2D <= EnterDist) && bOnGround;
}
