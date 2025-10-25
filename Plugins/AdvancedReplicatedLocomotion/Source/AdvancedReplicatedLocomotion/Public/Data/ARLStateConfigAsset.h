#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ARL_Enums.h"
#include "Data/ARL_Structs.h"
#include "ARLStateConfigAsset.generated.h"


UCLASS()
class ADVANCEDREPLICATEDLOCOMOTION_API UARLStateConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|State")
	TMap<EStates, FARLStateEntry> States;
	
};
