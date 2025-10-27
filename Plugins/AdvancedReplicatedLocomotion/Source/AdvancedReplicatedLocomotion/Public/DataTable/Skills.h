#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "Data/ARL_Structs.h"
#include "Skills.generated.h"


UCLASS(BlueprintType)
class ADVANCEDREPLICATEDLOCOMOTION_API USkillsInputMap : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SKILLS |Input")
	TArray<FARLSkillsMapping> SkillsMappings;
};
