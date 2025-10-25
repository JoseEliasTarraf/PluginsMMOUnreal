#pragma once
#include "CoreMinimal.h"
#include "ARL_Enums.generated.h"

UENUM(BlueprintType)
enum class EStates : uint8 {
	Default UMETA(DisplayName = "Default"),
	Lamp    UMETA(DisplayName = "Lamp"),
	Talisman UMETA(DisplayName = "Talisman"),
	MedKit  UMETA(DisplayName = "MedKit"),
};

UENUM(BlueprintType)
enum class EGatesStates : uint8 {
	None = 0,
	Walking UMETA(DisplayName = "Walking"),
	Jogging UMETA(DisplayName = "Jogging"),
	Crouch  UMETA(DisplayName = "Crouch"),
	Tired   UMETA(DisplayName = "Tired"),
};

UENUM(BlueprintType)
enum class EFootSide : uint8
{
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM(BlueprintType)
enum class ERotationModes : uint8 
{
	Idle UMETA(DisplayName = "Idle"),
	Movement UMETA(DisplayName = "Movement")
};

UENUM(BlueprintType)
enum class EVelocityLocomotionDirection : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Back UMETA(DisplayName = "Back"),
	Right UMETA(DisplayName = "Right"),
	Left UMETA(DisplayName = "Left")
};
