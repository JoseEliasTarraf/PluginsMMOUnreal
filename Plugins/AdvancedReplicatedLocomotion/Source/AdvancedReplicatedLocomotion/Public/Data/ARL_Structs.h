#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Animation/AnimSequence.h"
#include "ARL_Enums.h"
#include "ARL_Structs.generated.h"

// Forward declarations (keep headers light)
class UAnimInstance;
class UAnimMontage;
class UBlendSpace;
class UAnimSequence;
class UAnimSequenceBase;

/* =============================================================================
 * Gate settings
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FGateSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	float MaxWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	float MaxAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	float BrakingDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	float BrakingFrictionFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	float BrakingFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Gate")
	bool bUseSeparateBrakingFriction = false;
};

/* =============================================================================
 * Equip/Unequip anims
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FAnimSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Animations")
	UAnimMontage* EquipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Animations")
	UAnimMontage* UnequipAnimation;
};

/* =============================================================================
 * Jump anims
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FJumpAnims
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpStartAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpStartLoopAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpApexAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpFallLoopAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpFallLandAnim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TObjectPtr<UAnimSequence> JumpRecoveryAnim = nullptr;
};

/* =============================================================================
 * BlendSpaces per gate
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FLayerBlend
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Locomotion")
	UBlendSpace* WalkBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Locomotion")
	UBlendSpace* JogBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Locomotion")
	UBlendSpace* CrouchBS;
};

/* =============================================================================
 * Poses per state
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FLayerStateData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|State")
	TMap<EStates, UAnimSequence*> StatesPoses;
};

/* =============================================================================
 * Directional entries
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FAnimDirectionalEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Anim")
	float Direction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Anim")
	TObjectPtr<UAnimSequence> Anim = nullptr;
};

USTRUCT(BlueprintType)
struct FAnimSeqList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Anim")
	TArray<FAnimDirectionalEntry> Entries;
};

USTRUCT(BlueprintType)
struct FAnimsByDir
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Anim")
	TMap<EVelocityLocomotionDirection, TObjectPtr<UAnimSequence>> Entries;
};

/* =============================================================================
 * Stop/Start, Turn, Jump, Pivot layers
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FLayerStopStartData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|StopStart")
	TMap<EGatesStates, FAnimsByDir> StopAnims;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|StopStart")
	TMap<EGatesStates, FAnimsByDir> StartAnims;
};

USTRUCT(BlueprintType)
struct FLayerTurnAnims
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Turn")
	TMap<EGatesStates, FAnimSeqList> TurnAnims;
};

USTRUCT(BlueprintType)
struct FLayerJumpAnims
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Jump")
	TMap<EStates, FJumpAnims> JumpAnims;
};

USTRUCT(BlueprintType)
struct FLayerPivotAnims
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Pivot")
	TMap<EGatesStates, FAnimsByDir> PivotAnims;
};

/* =============================================================================
 * Overlay per state
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FStateOverlayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Overlay")
	TObjectPtr<UAnimSequenceBase> Anim = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Overlay")
	FName BlendMaskName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Overlay")
	int32 PoseIndex = 0;
};

USTRUCT(BlueprintType)
struct FStatesAnims
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|State")
	TMap<EStates, FStateOverlayConfig> States;
};

/* =============================================================================
 * State entries (linked anim layer)
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FARLStateEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|State")
	TSoftClassPtr<UAnimInstance> LinkedLayer;
};

/* =============================================================================
 * State keys (input)
 * ============================================================================= */

USTRUCT(BlueprintType)
struct FARLStateKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Input")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Input")
	EStates State = EStates::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Input")
	bool bFlipFlopToDefault = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Input")
	EStates OffState = EStates::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARL|Input")
	bool bOnReleased = false;
};
