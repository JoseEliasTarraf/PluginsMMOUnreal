#include "Health/HealtComponent.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "GenericMMOMonster/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

UHealtComponent::UHealtComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UHealtComponent::BeginPlay()
{
    Super::BeginPlay();

    ASC = Cast<UAbilitySystemComponent>(GetOwner()->GetComponentByClass(UAbilitySystemComponent::StaticClass()));
}

