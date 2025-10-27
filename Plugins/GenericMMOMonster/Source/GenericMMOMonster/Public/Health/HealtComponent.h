// HealthComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealtComponent.generated.h"

class UAnimMontage;
class UAnimSequenceBase;
class UAbilitySystemComponent;

UCLASS(ClassGroup = (ARL), meta = (BlueprintSpawnableComponent))
class GENERICMMOMONSTER_API UHealtComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UHealtComponent();
    
    UAbilitySystemComponent* ASC;

protected:
    virtual void BeginPlay() override;
   
    
};
