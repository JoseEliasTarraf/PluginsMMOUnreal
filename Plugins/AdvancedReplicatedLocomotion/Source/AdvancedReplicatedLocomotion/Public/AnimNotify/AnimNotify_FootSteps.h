// AnimNotify_FootSteps.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NiagaraSystem.h"
#include "Data/ARL_Enums.h"
#include "AnimNotify_FootSteps.generated.h"

class UDataTable;
class USkeletalMeshComponent;
class UAnimSequenceBase;

UCLASS()
class ADVANCEDREPLICATEDLOCOMOTION_API UAnimNotify_FootSteps : public UAnimNotify
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Footstep")
    TObjectPtr<UNiagaraSystem> VFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "ARL|Footstep")
    UDataTable* FootstepFXTable = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ARL|Footstep")
    EFootSide FootSide = EFootSide::Left;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
