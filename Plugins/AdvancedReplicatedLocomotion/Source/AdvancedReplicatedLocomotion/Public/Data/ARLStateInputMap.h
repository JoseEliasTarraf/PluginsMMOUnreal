#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ARL_Structs.h"
#include "ARLStateInputMap.generated.h"


UCLASS(BlueprintType)
class ADVANCEDREPLICATEDLOCOMOTION_API UARLStateInputMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "ARL|Input")
	TArray<FARLStateKey> Keys;

	UPROPERTY(EditAnywhere, Category = "ARL|Input")
	bool bConsumeInput = true;
	
};
