#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/ARL_Enums.h"
#include "Data/ARL_Structs.h"
#include "ARLAnimInstance.generated.h"

class AARLCharacter;


UCLASS()
class ADVANCEDREPLICATEDLOCOMOTION_API UARLAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "State")
	void UpdateAnimationState(AARLCharacter* Character, EStates State);

	UFUNCTION(BlueprintCallable, Category = "GateState")
	void UpdateGateStateAnim(EGatesStates LastGate, EGatesStates Gate);

	UFUNCTION(BlueprintCallable, Category = "LocomotionDirection")
	void UpdateGateVelocityDirection(EVelocityLocomotionDirection Direction);

	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void ApplyStopCue(uint8 InSeqIndex, int32 InNonce, float InStopDistance);

	// (Opcional) Evento BP pra dar Set Sequence + Set Explicit Time = 0 na entrada
	UFUNCTION(BlueprintImplementableEvent, Category = "Locomotion")
	void BP_OnApplyStopCue();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	float GroundDistance = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float MaxNoFloorDistance = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float GroundDistInterpSpeed = 4.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	float StopDistance2D = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	float DistForMatch = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	bool bCanEnterStop = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	bool bUseStartAnim = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	bool bUseStopAnim = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	bool bUsePivot = true;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	bool bStopCooldown = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	bool bInStopState = false;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float EnterDist_Owner = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float EnterDist_Proxy = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float MinSpeed_Owner = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float MinSpeed_Proxy = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float CurveMaxDist = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float ExitSpeed = 140.f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Tuning")
	float CooldownSeconds = 0.25f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	FVector PredictedStopLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Jump")
	bool isJumping;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Jump")
	bool isFalling;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Jump")
	bool onAir;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Locomotion")
	bool bCanStop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GateState")
	EGatesStates CurrenteGate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GateState")
	EGatesStates LastFrameGate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EStates CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FStatesAnims StateAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FLayerBlend LayersBS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FLayerStopStartData StopStartData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FLayerTurnAnims TurnData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FLayerJumpAnims JumpData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FLayerPivotAnims PivotData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bIsCrouched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float crouchAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float LocationDelta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float RotationYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float Quadrant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float LastQuadrant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float LeanAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float PitchAimValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FVector CharacterVelocityV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FVector Acceleration2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	FVector PivotAcceleration2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EVelocityLocomotionDirection LastFrameDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EVelocityLocomotionDirection VelocityLocomotionDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	ERotationModes RotationModes;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	uint8 StopSeqIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	int32  StopNonce = 0;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	float NewStopDistance = 0.f;
};
